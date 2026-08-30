param(
  [ValidateSet('Release', 'Debug')]
  [string]$Configuration = 'Release',
  [switch]$SidesOnly
)

# Shared Main + Side build (stage 4 of the wasm modular runbook).
#
# Layout decisions mandated by the runbook:
#   - shared uses its OWN PIC/relocatable OCCT build directory; the stable
#     non-PIC directory used by full is never reused or merged.
#   - shared-main: MAIN_MODULE=2 with ES6 modularized glue, no STANDALONE_WASM.
#   - shared-side-<group>: SIDE_MODULE=2 PIC objects, no STANDALONE_WASM.
#   - Main export manifest is generated from the union of Side undefined
#     imports plus the fixed host ABI (scripts/spike extract-exports flow).

$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$emsdk = Join-Path $repo 'vendor/emsdk'
$buildRoot = Join-Path $repo "build/$($Configuration.ToLowerInvariant())/shared"
$occtBuild = Join-Path $buildRoot 'occt-build-pic'
$occtInstall = Join-Path $buildRoot 'occt-install-pic'
$kernelBuild = Join-Path $buildRoot 'kernel'
. (Join-Path $PSScriptRoot 'build-config.ps1')
$buildConfig = Get-OcctWasmBuildConfig
$cFlags = $buildConfig.OcctCFlags -join ' '
$picFlags = (@('-fPIC') + $buildConfig.OcctCxxFlags) -join ' '
$occtCMakeArguments = $buildConfig.OcctCMakeArguments

& npm run generate
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$occtChanges = & git -C (Join-Path $repo 'occt') status --porcelain
if ($occtChanges) {
  $occtChanges | Write-Error
  throw 'OCCT source tree must be clean'
}

$toolPaths = & (Join-Path $repo 'scripts/bootstrap-tools.ps1')
$env:PATH = (($toolPaths | Where-Object { $_ }) -join [IO.Path]::PathSeparator) + [IO.Path]::PathSeparator + $env:PATH
& (Join-Path $emsdk 'emsdk.ps1') activate $buildConfig.EmsdkVersion
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& (Join-Path $emsdk 'emsdk_env.ps1') | Out-Null

if (-not $SidesOnly) {
  # 1. PIC/relocatable OCCT archives in a dedicated directory.
  & emcmake cmake -S (Join-Path $repo 'occt') -B $occtBuild -G Ninja `
    "-DCMAKE_BUILD_TYPE=$Configuration" `
    "-DCMAKE_INSTALL_PREFIX=$occtInstall" `
    "-DCMAKE_C_FLAGS=$cFlags" `
    "-DCMAKE_CXX_FLAGS=$picFlags" `
    @occtCMakeArguments
  if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
  & cmake --build $occtBuild --target install --parallel
  if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

# 2. Kernel configure for shared targets.
& emcmake cmake -S $repo -B $kernelBuild -G Ninja `
  "-DCMAKE_BUILD_TYPE=$Configuration" `
  "-DOCCT_ROOT=$occtInstall" `
  '-DKERNEL_BUILD_SHARED=ON'
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& cmake --build $kernelBuild --parallel
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$artifactDir = Join-Path $repo 'artifacts'
New-Item -ItemType Directory -Force -Path $artifactDir | Out-Null
Copy-Item -Force (Join-Path $kernelBuild 'kernel/shared-main.mjs') (Join-Path $artifactDir 'shared-main.mjs')
Copy-Item -Force (Join-Path $kernelBuild 'kernel/shared-main.wasm') (Join-Path $artifactDir 'shared-main.wasm')
$topology = Get-Content -Raw (Join-Path $repo 'scripts/profile-topology.generated.json') | ConvertFrom-Json
foreach ($side in $topology.sides) {
  Copy-Item -Force `
    (Join-Path $kernelBuild "kernel/$($side.target).wasm") `
    (Join-Path $artifactDir $side.artifact)
}
& node (Join-Path $repo 'scripts/verify-artifacts.mjs') --write --write-family shared
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
