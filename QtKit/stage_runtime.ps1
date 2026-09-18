<#
    Copy the committed QtKit runtime bundle beside QmlColorPiker.exe.

    This script has no dependency on a PowerDirector checkout. The Visual
    Studio project invokes it after every successful build; unchanged files
    are skipped so incremental builds remain fast.
#>
[CmdletBinding()]
param(
    [string]$Destination = (Join-Path $PSScriptRoot '..\color_picker_demo\bin_x64'),
    [switch]$Clean,
    [switch]$Verify,
    [switch]$Quiet
)

$ErrorActionPreference = 'Stop'

$runtimeRoot = Join-Path $PSScriptRoot 'runtime'
if (-not (Test-Path -LiteralPath $runtimeRoot -PathType Container)) {
    throw "QtKit runtime bundle is missing: $runtimeRoot"
}

if (-not (Test-Path -LiteralPath $Destination)) {
    New-Item -ItemType Directory -Path $Destination -Force | Out-Null
}

$runtimeRoot = (Resolve-Path -LiteralPath $runtimeRoot).Path.TrimEnd('\')
$destinationRoot = (Resolve-Path -LiteralPath $Destination).Path.TrimEnd('\')
if ($runtimeRoot -eq $destinationRoot) {
    throw 'Destination must not be the bundled runtime source directory.'
}

$requiredFiles = @(
    'QtKit.dll',
    'Qt6Core.dll',
    'Qt6Gui.dll',
    'Qt6Qml.dll',
    'Qt6Quick.dll',
    'MSVCP140D.dll',
    'VCRUNTIME140D.dll',
    'VCRUNTIME140_1D.dll',
    'mfc140ud.dll',
    'platforms\qwindows.dll',
    'qml\QtQuick\qmldir',
    'qml\QtQuick\Controls\qmldir',
    'qml\QtQuick\Controls\Basic\qmldir'
)

foreach ($relativePath in $requiredFiles) {
    $sourcePath = Join-Path $runtimeRoot $relativePath
    if (-not (Test-Path -LiteralPath $sourcePath -PathType Leaf)) {
        throw "Required QtKit runtime file is missing: $relativePath"
    }
}
$sourceFiles = @(Get-ChildItem -LiteralPath $runtimeRoot -Recurse -File)
if ($sourceFiles.Count -eq 0) {
    throw "QtKit runtime bundle is empty: $runtimeRoot"
}

if ($Clean) {
    foreach ($sourceFile in $sourceFiles) {
        $relativePath = $sourceFile.FullName.Substring($runtimeRoot.Length + 1)
        $targetPath = Join-Path $destinationRoot $relativePath
        if (Test-Path -LiteralPath $targetPath -PathType Leaf) {
            Remove-Item -LiteralPath $targetPath -Force
        }
    }
}

$copied = 0
$skipped = 0
$totalBytes = 0L

foreach ($sourceFile in $sourceFiles) {
    $relativePath = $sourceFile.FullName.Substring($runtimeRoot.Length + 1)
    $targetPath = Join-Path $destinationRoot $relativePath
    $targetDirectory = Split-Path $targetPath -Parent
    if (-not (Test-Path -LiteralPath $targetDirectory)) {
        New-Item -ItemType Directory -Path $targetDirectory -Force | Out-Null
    }

    $targetFile = Get-Item -LiteralPath $targetPath -ErrorAction SilentlyContinue
    $isCurrent = $null -ne $targetFile -and
                 $targetFile.Length -eq $sourceFile.Length -and
                 $targetFile.LastWriteTimeUtc -eq $sourceFile.LastWriteTimeUtc

    if ($isCurrent) {
        ++$skipped
    }
    else {
        Copy-Item -LiteralPath $sourceFile.FullName -Destination $targetPath -Force
        ++$copied
    }
    $totalBytes += $sourceFile.Length
}

if ($Verify) {
    foreach ($sourceFile in $sourceFiles) {
        $relativePath = $sourceFile.FullName.Substring($runtimeRoot.Length + 1)
        $targetPath = Join-Path $destinationRoot $relativePath
        $targetFile = Get-Item -LiteralPath $targetPath -ErrorAction SilentlyContinue
        if ($null -eq $targetFile -or $targetFile.Length -ne $sourceFile.Length) {
            throw "Runtime verification failed: $relativePath"
        }
    }
}

if (-not $Quiet) {
    Write-Host "QtKit runtime staged to: $destinationRoot"
    Write-Host ("Files: {0} copied, {1} unchanged, {2} total ({3:N1} MB)" -f
        $copied, $skipped, $sourceFiles.Count, ($totalBytes / 1MB))
    if ($Verify) {
        Write-Host 'Verification succeeded.'
    }
}
