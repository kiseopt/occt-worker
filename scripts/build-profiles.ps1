param(
  [ValidateSet('Release', 'Debug')]
  [string]$Configuration = 'Release'
)

$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$emsdk = Join-Path $repo 'vendor/emsdk'
$buildRoot = Join-Path $repo "build/$($Configuration.ToLowerInvariant())"
$occtInstall = Join-Path $buildRoot 'occt-install'
$profileBuild = Join-Path $buildRoot 'profiles'
$artifactDir = Join-Path $repo 'artifacts'
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

& emcmake cmake -S $repo -B $profileBuild -G Ninja `
  "-DCMAKE_BUILD_TYPE=$Configuration" `
  "-DOCCT_ROOT=$occtInstall" `
  '-DKERNEL_BUILD_PROFILES=ON'
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& cmake --build $profileBuild --parallel
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

New-Item -ItemType Directory -Force -Path $artifactDir | Out-Null
$topology = Get-Content -Raw (Join-Path $repo 'scripts/profile-topology.generated.json') | ConvertFrom-Json
foreach ($profile in $topology.buildProfiles) {
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
& node (Join-Path $repo 'scripts/verify-artifacts.mjs') --write --write-family isolated
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
