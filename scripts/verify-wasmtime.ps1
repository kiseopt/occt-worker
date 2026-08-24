param(
  [string]$WasmtimePath = 'wasmtime',
  [string]$ArtifactPath
)

$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
if (-not $ArtifactPath) { $ArtifactPath = Join-Path $repo 'wasm/occt-worker.wasmtime.wasm' }

if (-not (Test-Path $ArtifactPath -PathType Leaf)) {
  throw "Wasmtime wasm artifact not found: $ArtifactPath. Run npm run build:wasm:wasmtime first."
}
$wasmtime = Get-Command $WasmtimePath -ErrorAction SilentlyContinue
if (-not $wasmtime) {
  throw "Wasmtime CLI not found: $WasmtimePath. Install Wasmtime 47 or pass -WasmtimePath with its executable path."
}

$version = (& $wasmtime.Source --version).Trim()
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $wasmtime.Source run `
  --optimize 'memory-may-move=y,memory-reservation=16777216,memory-reservation-for-growth=0,memory-guard-size=65536' `
  --preload "env=$(Join-Path $repo 'tests/wasmtime/env-stubs.wat')" `
  --preload "occt=$ArtifactPath" `
  (Join-Path $repo 'tests/wasmtime/smoke.wat')
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$artifact = Get-Item $ArtifactPath
$stream = [IO.File]::OpenRead($ArtifactPath)
try {
  $sha256 = [Security.Cryptography.SHA256]::Create()
  $artifactHash = ([BitConverter]::ToString($sha256.ComputeHash($stream))).Replace('-', '').ToLowerInvariant()
} finally {
  if ($sha256) { $sha256.Dispose() }
  $stream.Dispose()
}
$report = [ordered]@{
  artifact = 'wasm/occt-worker.wasmtime.wasm'
  transform = 'Binaryen --translate-to-exnref'
  sizeBytes = $artifact.Length
  sha256 = $artifactHash
  testedWith = $version
  smoke = @('initialize', 'beginScope', 'makeBox', 'makeCylinder', 'booleanCut', 'bbox', 'tessellate', 'InvalidArgs', 'releaseAll', 'stats')
}
$reportJson = ($report | ConvertTo-Json -Depth 4) + [Environment]::NewLine
$utf8 = New-Object Text.UTF8Encoding($false)
[IO.File]::WriteAllText((Join-Path $repo 'docs/wasmtime-build.json'), $reportJson, $utf8)
Write-Output "Wasmtime smoke passed with $version"
