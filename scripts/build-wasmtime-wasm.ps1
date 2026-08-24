param(
  [string]$InputArtifact,
  [string]$OutputArtifact
)

$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
if (-not $InputArtifact) { $InputArtifact = Join-Path $repo 'wasm/occt-worker.wasm' }
if (-not $OutputArtifact) { $OutputArtifact = Join-Path $repo 'wasm/occt-worker.wasmtime.wasm' }
$wasmOpt = Join-Path $repo 'vendor/emsdk/upstream/bin/wasm-opt.exe'

if (-not (Test-Path $InputArtifact -PathType Leaf)) {
  throw "Main wasm artifact not found: $InputArtifact. Run npm run build:wasm first."
}
if (-not (Test-Path $wasmOpt -PathType Leaf)) {
  throw "wasm-opt not found: $wasmOpt. Run npm run bootstrap:sources first."
}

$outputDirectory = Split-Path -Parent $OutputArtifact
New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null
$temporaryArtifact = "$OutputArtifact.tmp"

try {
  # Emscripten 4.0.23 emits Phase 3 wasm-EH. Binaryen translates it to the
  # standardized try_table/exnref form accepted by current Wasmtime releases.
  & $wasmOpt $InputArtifact --translate-to-exnref --all-features -o $temporaryArtifact
  if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
  Move-Item -Force $temporaryArtifact $OutputArtifact
} finally {
  if (Test-Path $temporaryArtifact) { Remove-Item -Force $temporaryArtifact }
}

$artifact = Get-Item $OutputArtifact
$stream = [IO.File]::OpenRead($OutputArtifact)
try {
  $sha256 = [Security.Cryptography.SHA256]::Create()
  $hash = ([BitConverter]::ToString($sha256.ComputeHash($stream))).Replace('-', '').ToLowerInvariant()
} finally {
  if ($sha256) { $sha256.Dispose() }
  $stream.Dispose()
}
Write-Output "Built $($artifact.FullName) ($($artifact.Length) bytes, sha256 $hash)"
