param(
  [ValidateSet('Release', 'Debug')]
  [string]$Configuration = 'Release',
  [string]$BuildRoot,
  [string]$OutputDir,
  [string[]]$Profiles = @(),
  [switch]$SkipManifestWrite,
  [long]$MaxMemory
)

$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$emsdk = Join-Path $repo 'vendor/emsdk'
$configurationBuildRoot = Join-Path $repo "build/$($Configuration.ToLowerInvariant())"
$occtInstall = Join-Path $configurationBuildRoot 'occt-install'
$profileBuild = if ($PSBoundParameters.ContainsKey('BuildRoot')) {
  $path = if ([IO.Path]::IsPathRooted($BuildRoot)) { $BuildRoot } else { Join-Path $repo $BuildRoot }
  [IO.Path]::GetFullPath($path)
} else {
  Join-Path $configurationBuildRoot 'profiles'
}
$artifactDir = if ($PSBoundParameters.ContainsKey('OutputDir')) {
  $path = if ([IO.Path]::IsPathRooted($OutputDir)) { $OutputDir } else { Join-Path $repo $OutputDir }
  [IO.Path]::GetFullPath($path)
} else {
  Join-Path $repo 'artifacts'
}
. (Join-Path $PSScriptRoot 'build-config.ps1')
$buildConfig = Get-OcctWasmBuildConfig

& npm run generate
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$occtChanges = & git -C (Join-Path $repo 'occt') status --porcelain
if ($occtChanges) {
  $occtChanges | Write-Error
  throw 'OCCT source tree must be clean'
}

if (-not (Test-Path -LiteralPath (Join-Path $occtInstall 'include/opencascade/Standard.hxx'))) {
  & (Join-Path $repo 'scripts/build-wasm.ps1') -Configuration $Configuration
  if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

$toolPaths = & (Join-Path $repo 'scripts/bootstrap-tools.ps1')
$env:PATH = (($toolPaths | Where-Object { $_ }) -join [IO.Path]::PathSeparator) + [IO.Path]::PathSeparator + $env:PATH
& (Join-Path $emsdk 'emsdk.ps1') activate $buildConfig.EmsdkVersion
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& (Join-Path $emsdk 'emsdk_env.ps1') | Out-Null

$cmakeArguments = @(
  "-DCMAKE_BUILD_TYPE=$Configuration"
  "-DOCCT_ROOT=$occtInstall"
  '-DKERNEL_BUILD_PROFILES=ON'
)
if ($PSBoundParameters.ContainsKey('MaxMemory')) {
  $cmakeArguments += "-DOCCT_WORKER_MAX_MEMORY=$MaxMemory"
}
& emcmake cmake -S $repo -B $profileBuild -G Ninja @cmakeArguments
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$topology = Get-Content -Raw (Join-Path $repo 'scripts/profile-topology.generated.json') | ConvertFrom-Json
$selectedProfiles = if ($Profiles.Count -eq 0) {
  @($topology.buildProfiles)
} else {
  $unknownProfiles = @($Profiles | Where-Object { $_ -notin $topology.buildProfiles.id })
  if ($unknownProfiles.Count -ne 0) {
    throw "Unknown build profile(s): $($unknownProfiles -join ', ')"
  }
  @($topology.buildProfiles | Where-Object { $_.id -in $Profiles })
}
$targets = @($selectedProfiles | ForEach-Object { $_.target })
& cmake --build $profileBuild --parallel --target @targets
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

if ('preview' -in $selectedProfiles.id) {
  $previewMap = Join-Path $profileBuild 'kernel/profile-preview.map'
  $previewSizeReport = Join-Path $profileBuild 'kernel/profile-preview.occt-sizes.md'
  & node (Join-Path $repo 'scripts/report-occt-link-map.mjs') $previewMap --output $previewSizeReport
  if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

New-Item -ItemType Directory -Force -Path $artifactDir | Out-Null
foreach ($profile in $selectedProfiles) {
  $source = Join-Path $profileBuild "kernel/$($profile.target).wasm"
  $target = Join-Path $artifactDir $profile.artifact
  if ($Configuration -eq 'Release') {
    & (Join-Path $emsdk 'upstream/bin/wasm-opt.exe') $source -O3 --all-features --converge -o $target
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
  } else {
    Copy-Item -Force $source $target
  }
  node -e "new WebAssembly.Module(require('fs').readFileSync(process.argv[1]))" $target
  if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}
if (-not $SkipManifestWrite) {
  & node (Join-Path $repo 'scripts/verify-artifacts.mjs') --write --write-family isolated
  if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}
