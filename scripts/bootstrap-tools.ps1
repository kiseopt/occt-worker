$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$tools = Join-Path $repo '.tools'
$downloads = Join-Path $tools 'downloads'
$cmakeRoot = Join-Path $tools 'cmake-4.4.2-windows-x86_64'
$ninjaRoot = Join-Path $tools 'ninja-1.13.2'

New-Item -ItemType Directory -Force -Path $downloads | Out-Null

function Install-Archive {
  param(
    [string]$Url,
    [string]$Archive,
    [string]$Sha256,
    [string]$Destination,
    [string]$Expected
  )
  if (Test-Path -LiteralPath $Expected) { return }
  if (-not (Test-Path -LiteralPath $Archive)) {
    Invoke-WebRequest -Uri $Url -OutFile $Archive
  }
  $actual = (Get-FileHash -Algorithm SHA256 -LiteralPath $Archive).Hash.ToLowerInvariant()
  if ($actual -ne $Sha256) { throw "Checksum mismatch for $Archive" }
  New-Item -ItemType Directory -Force -Path $Destination | Out-Null
  Expand-Archive -LiteralPath $Archive -DestinationPath $Destination -Force
}

Install-Archive `
  -Url 'https://github.com/Kitware/CMake/releases/download/v4.4.2/cmake-4.4.2-windows-x86_64.zip' `
  -Archive (Join-Path $downloads 'cmake-4.4.2-windows-x86_64.zip') `
  -Sha256 'e8139d85b3813bc38833142ae1940472e9a587e9b5d2718ac1804c60f4e57a64' `
  -Destination $tools `
  -Expected (Join-Path $cmakeRoot 'bin/cmake.exe')

Install-Archive `
  -Url 'https://github.com/ninja-build/ninja/releases/download/v1.13.2/ninja-win.zip' `
  -Archive (Join-Path $downloads 'ninja-win.zip') `
  -Sha256 '07fc8261b42b20e71d1720b39068c2e14ffcee6396b76fb7a795fb460b78dc65' `
  -Destination $ninjaRoot `
  -Expected (Join-Path $ninjaRoot 'ninja.exe')

Write-Output (Join-Path $cmakeRoot 'bin')
Write-Output $ninjaRoot
