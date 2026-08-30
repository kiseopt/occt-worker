param(
  [ValidateSet('Release', 'Debug')]
  [string]$Configuration = 'Release'
)

$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$emsdk = Join-Path $repo 'vendor/emsdk'
$buildRoot = Join-Path $repo "build/$($Configuration.ToLowerInvariant())"
$occtBuild = Join-Path $buildRoot 'occt-build'
$occtInstall = Join-Path $buildRoot 'occt-install'
$kernelBuild = Join-Path $buildRoot 'kernel'
. (Join-Path $PSScriptRoot 'build-config.ps1')
$buildConfig = Get-OcctWasmBuildConfig
$occtCMakeArguments = $buildConfig.OcctCMakeArguments

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
$occtChanges = & git -C (Join-Path $repo 'occt') status --porcelain
if ($occtChanges) {
  $occtChanges | Write-Error
  throw 'OCCT source tree must be clean'
}
node (Join-Path $repo 'scripts/generate-protocol.mjs')

# -UOCC_CONVERT_SIGNALS: OCC_CATCH_SIGNALS must not expand to setjmp. This kernel never installs
# signal handlers, and Emscripten lowers setjmp through wasm-EH in a way that emits br_table
# instructions with inconsistent label arities (invalid wasm) throughout TKShHealing/TKDESTEP.
& emcmake cmake -S (Join-Path $repo 'occt') -B $occtBuild -G Ninja `
  "-DCMAKE_BUILD_TYPE=$Configuration" `
  "-DCMAKE_INSTALL_PREFIX=$occtInstall" `
  "-DCMAKE_C_FLAGS=$($buildConfig.OcctCFlags -join ' ')" `
  "-DCMAKE_CXX_FLAGS=$($buildConfig.OcctCxxFlags -join ' ')" `
  @occtCMakeArguments
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& cmake --build $occtBuild --target install --parallel
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& emcmake cmake -S $repo -B $kernelBuild -G Ninja `
  "-DCMAKE_BUILD_TYPE=$Configuration" `
  "-DOCCT_ROOT=$occtInstall"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& cmake --build $kernelBuild --parallel
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$builtArtifact = Join-Path $kernelBuild 'kernel/occt-worker.wasm'
if ($Configuration -eq 'Release') {
  $optimizedArtifact = Join-Path $kernelBuild 'kernel/occt-worker.optimized.wasm'
    & (Join-Path $emsdk 'upstream/bin/wasm-opt.exe') $builtArtifact -O3 --all-features --converge -o $optimizedArtifact
  if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
  $builtArtifact = $optimizedArtifact
}
$artifactPath = if ($Configuration -eq 'Release') {
  New-Item -ItemType Directory -Force -Path (Join-Path $repo 'wasm') | Out-Null
  Join-Path $repo 'wasm/occt-worker.wasm'
} else {
  Join-Path $buildRoot 'occt-worker.debug.wasm'
}
Copy-Item -Force $builtArtifact $artifactPath
if ($Configuration -eq 'Release') {
  node (Join-Path $repo 'scripts/verify-wasm.mjs')
} else {
  node -e "new WebAssembly.Module(require('fs').readFileSync(process.argv[1]))" $artifactPath
}
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
