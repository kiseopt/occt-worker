param(
  [ValidateSet('Release', 'Debug')]
  [string]$Configuration = 'Release'
)

$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$emsdk = Join-Path $repo 'vendor/emsdk'
$buildDir = Join-Path $repo "build/$($Configuration.ToLowerInvariant())/dylink-spike"
. (Join-Path $repo 'scripts/build-config.ps1')
$buildConfig = Get-OcctWasmBuildConfig

$toolPaths = & (Join-Path $repo 'scripts/bootstrap-tools.ps1')
$env:PATH = (($toolPaths | Where-Object { $_ }) -join [IO.Path]::PathSeparator) + [IO.Path]::PathSeparator + $env:PATH
$emsdkCommand = Join-Path $emsdk 'emsdk.ps1'
if (-not (Test-Path (Join-Path $emsdk 'upstream/emscripten/emcc.py'))) {
  & $emsdkCommand install $buildConfig.EmsdkVersion
  if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}
& $emsdkCommand activate $buildConfig.EmsdkVersion
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& (Join-Path $emsdk 'emsdk_env.ps1') | Out-Null

& emcmake cmake -S $PSScriptRoot -B $buildDir -G Ninja "-DCMAKE_BUILD_TYPE=$Configuration"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& cmake --build $buildDir
exit $LASTEXITCODE
