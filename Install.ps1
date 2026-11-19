[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$GamePath,
    [Parameter(Mandatory=$true)][string]$EnriquePackage,
    [string]$ModDirectory,
    [switch]$CheckOnly
)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.IO.Compression.FileSystem
$gameRoot = (Resolve-Path -LiteralPath $GamePath).Path
$exe = Join-Path $gameRoot 'bin\win_x64\eurotrucks2.exe'
$expectedExe = (Get-Content -LiteralPath (Join-Path $PSScriptRoot 'package\game.sha256') -Raw).Trim()
if ((Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash -ine $expectedExe) {
    throw 'Dieser ETS2-Build wird noch nicht unterstuetzt. Es wurde nichts installiert.'
}
$spec = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'package\model-patch.json') -Raw | ConvertFrom-Json
$packagePath = (Resolve-Path -LiteralPath $EnriquePackage).Path
$archive = [IO.Compression.ZipFile]::OpenRead($packagePath)
try {
    $entry = $archive.GetEntry($spec.path)
    if ($null -eq $entry) { throw 'Die passende Enrique-Modelldatei fehlt in diesem Paket.' }
    $inputStream = $entry.Open()
    $originalStream = New-Object IO.MemoryStream
    try { $inputStream.CopyTo($originalStream); $original = $originalStream.ToArray() }
    finally { $inputStream.Dispose(); $originalStream.Dispose() }
} finally { $archive.Dispose() }
function Get-BytesHash([byte[]]$Bytes) {
    $sha = [Security.Cryptography.SHA256]::Create()
    try { return ([BitConverter]::ToString($sha.ComputeHash($Bytes))).Replace('-','').ToLowerInvariant() }
    finally { $sha.Dispose() }
}
$inputHash = Get-BytesHash $original
if ($inputHash -eq $spec.target_sha256) { $patched = $original }
elseif ($inputHash -eq $spec.source_sha256) {
    $output = New-Object IO.MemoryStream
    try {
        foreach ($op in $spec.operations) {
            if ($op.kind -eq 'copy') {
                $offset = [int]$op.offset; $length = [int]$op.length
                if ($offset -lt 0 -or $length -lt 0 -or ($offset + $length) -gt $original.Length) {
                    throw 'Ungueltiger Patchbereich.'
                }
                $output.Write($original,$offset,$length)
            } elseif ($op.kind -eq 'data') {
                $literal = [Convert]::FromBase64String($op.base64)
                $output.Write($literal,0,$literal.Length)
            } else { throw 'Unbekannte Patchanweisung.' }
        }
        $patched = $output.ToArray()
    } finally { $output.Dispose() }
} else { throw 'Diese Version der Enrique-Modelldatei wird nicht unterstuetzt. Keine Aenderungen.' }
if ((Get-BytesHash $patched) -ne $spec.target_sha256) { throw 'Patch-Pruefsumme stimmt nicht.' }
$dll = Join-Path $PSScriptRoot 'BoHaCornerLights.dll'
if (-not (Test-Path -LiteralPath $dll -PathType Leaf)) { throw 'Die DLL fehlt. Bitte die fertige Release-ZIP herunterladen.' }
if ($CheckOnly) { Write-Output 'OK: Spielbuild, Quelldatei, Patch und DLL vorhanden; nichts installiert.'; return }
if (Get-Process eurotrucks2 -ErrorAction SilentlyContinue) { throw 'ETS2 bitte zuerst vollstaendig schliessen.' }
if (-not $ModDirectory) {
    $ModDirectory = Join-Path ([Environment]::GetFolderPath('MyDocuments')) 'Euro Truck Simulator 2\mod'
}
$modRoot = [IO.Path]::GetFullPath($ModDirectory)
$pluginRoot = Join-Path $gameRoot 'bin\win_x64\plugins'
[IO.Directory]::CreateDirectory($modRoot) | Out-Null
[IO.Directory]::CreateDirectory($pluginRoot) | Out-Null
$pluginTarget = Join-Path $pluginRoot 'BoHaCornerLights.dll'
$modTarget = Join-Path $modRoot 'BoHa - Corner Lights.scs'
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss-fff'
$tempMod = Join-Path $modRoot ('.boha-corner-' + [Guid]::NewGuid().ToString('N') + '.tmp')
function Add-ZipBytes($Zip,[string]$Name,[byte[]]$Bytes) {
    $item = $Zip.CreateEntry($Name)
    $stream = $item.Open()
    try { $stream.Write($Bytes,0,$Bytes.Length) } finally { $stream.Dispose() }
}
$newZip = [IO.Compression.ZipFile]::Open($tempMod,[IO.Compression.ZipArchiveMode]::Create)
try {
    Add-ZipBytes $newZip $spec.path $patched
    Add-ZipBytes $newZip 'unit/hookup/vehicle/flare/boha_corner_lights.sii' ([IO.File]::ReadAllBytes((Join-Path $PSScriptRoot 'package\boha_corner_lights.sii')))
    Add-ZipBytes $newZip 'manifest.sii' ([IO.File]::ReadAllBytes((Join-Path $PSScriptRoot 'package\manifest.sii')))
    Add-ZipBytes $newZip 'description.txt' ([IO.File]::ReadAllBytes((Join-Path $PSScriptRoot 'package\description.txt')))
} finally { $newZip.Dispose() }
$hadPlugin = Test-Path -LiteralPath $pluginTarget
$hadMod = Test-Path -LiteralPath $modTarget
$pluginBackup = $pluginTarget + '.' + $stamp + '.bak'
$modBackup = $modTarget + '.' + $stamp + '.bak'
if ($hadPlugin) { Copy-Item -LiteralPath $pluginTarget -Destination $pluginBackup }
if ($hadMod) { Copy-Item -LiteralPath $modTarget -Destination $modBackup }
try {
    Copy-Item -LiteralPath $dll -Destination $pluginTarget -Force
    Move-Item -LiteralPath $tempMod -Destination $modTarget -Force
    if ((Get-FileHash -LiteralPath $dll).Hash -ne (Get-FileHash -LiteralPath $pluginTarget).Hash) { throw 'DLL-Pruefung fehlgeschlagen.' }
} catch {
    if ($hadPlugin) { Copy-Item -LiteralPath $pluginBackup -Destination $pluginTarget -Force }
    elseif (Test-Path -LiteralPath $pluginTarget) { Remove-Item -LiteralPath $pluginTarget }
    if ($hadMod) { Copy-Item -LiteralPath $modBackup -Destination $modTarget -Force }
    elseif (Test-Path -LiteralPath $modTarget) { Remove-Item -LiteralPath $modTarget }
    throw
} finally {
    if (Test-Path -LiteralPath $tempMod) { Remove-Item -LiteralPath $tempMod }
}
Write-Output 'Installiert. BoHa - Corner Lights im Mod-Manager oberhalb von Enrique aktivieren.'
Write-Output 'Am Volvo 9700 DD muss die Enrique-Komponente Front Slots montiert sein.'
