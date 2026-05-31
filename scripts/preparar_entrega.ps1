$ErrorActionPreference = "Stop"

$repo = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $repo "build\Desktop_Qt_6_10_2_MinGW_64_bit-Debug"
$qtBin = "C:\Qt\6.10.2\mingw_64\bin"
$mingwBin = "C:\Qt\Tools\mingw1310_64\bin"
$exeBuild = Join-Path $buildDir "release\ClavadosCiudadAcademia.exe"
$entregaDir = Join-Path $repo "entrega"
$deployDir = Join-Path $entregaDir "paquete-windows"

$env:PATH = "$mingwBin;$qtBin;$env:PATH"

Push-Location $buildDir
try {
    & "$qtBin\qmake.exe" "..\..\ClavadosCiudadAcademia.pro"
    & "$mingwBin\mingw32-make.exe" "-j4"
}
finally {
    Pop-Location
}

New-Item -ItemType Directory -Force $entregaDir | Out-Null
Copy-Item -LiteralPath $exeBuild -Destination (Join-Path $entregaDir "ClavadosCiudadAcademia.exe") -Force

New-Item -ItemType Directory -Force $deployDir | Out-Null
Copy-Item -LiteralPath $exeBuild -Destination (Join-Path $deployDir "ClavadosCiudadAcademia.exe") -Force
& "$qtBin\windeployqt.exe" (Join-Path $deployDir "ClavadosCiudadAcademia.exe")

Write-Host "Entrega preparada en: $entregaDir"
