param()

$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$version = '47.0.3'
$archiveName = "wasmtime-v$version-x86_64-windows.zip"
$archivePath = Join-Path $repo ".tools/$archiveName"
$installRoot = Join-Path $repo ".tools/wasmtime-v$version"
$executable = Join-Path $installRoot "wasmtime-v$version-x86_64-windows/wasmtime.exe"
$expectedHash = '80ddf037820b35a9a53c13519632f52947e848d6ba69a483840b7330110408f3'

if (-not (Test-Path $executable -PathType Leaf)) {
  New-Item -ItemType Directory -Force -Path (Split-Path -Parent $archivePath) | Out-Null
  if (-not (Test-Path $archivePath -PathType Leaf)) {
    Invoke-WebRequest `
      -Uri "https://github.com/bytecodealliance/wasmtime/releases/download/v$version/$archiveName" `
      -OutFile $archivePath
  }
  $stream = [IO.File]::OpenRead($archivePath)
  try {
    $sha256 = [Security.Cryptography.SHA256]::Create()
    $actualHash = ([BitConverter]::ToString($sha256.ComputeHash($stream))).Replace('-', '').ToLowerInvariant()
  } finally {
    if ($sha256) { $sha256.Dispose() }
    $stream.Dispose()
  }
  if ($actualHash -ne $expectedHash) {
    throw "Wasmtime archive hash mismatch: $actualHash"
  }
  New-Item -ItemType Directory -Force -Path $installRoot | Out-Null
  Expand-Archive -Path $archivePath -DestinationPath $installRoot -Force
}

if (-not (Test-Path $executable -PathType Leaf)) {
  throw "Wasmtime executable not found after extraction: $executable"
}

Write-Output $executable
