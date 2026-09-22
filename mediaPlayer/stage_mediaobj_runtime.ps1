[CmdletBinding()]
param(
    [string]$Destination = (Join-Path $PSScriptRoot 'bin_x64'),
    [string]$SourceRoot,
    [switch]$Quiet
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($SourceRoot))
{
    $SourceRoot = Join-Path $PSScriptRoot '..\..'
}
$sourceMediaCache = Join-Path $SourceRoot 'external_bin\runtime\mediacache'
$sourceDecoderPack = Join-Path $SourceRoot 'external_bin\runtime\decoderPack'
$destinationRuntime = Join-Path $Destination 'runtime'
$destinationMediaCache = Join-Path $destinationRuntime 'mediacache'
$destinationDecoderPack = Join-Path $destinationRuntime 'decoderPack'
$legacySimba = Join-Path $destinationRuntime 'simba'

if (-not (Test-Path -LiteralPath $sourceMediaCache -PathType Container))
{
    throw "MediaObj runtime source not found: $sourceMediaCache"
}

$requiredFiles = @(
    'MediaObj.dll',
    'MediaObjExt.dll',
    'MediaObj.ini',
    'CLMediaDetect.dll',
    'MediaInfo.dll',
    'CLDSPPlayer.dll',
    'CLDSPSource.ax',
    'CLCustomSourceReader.ax',
    'SrcIPCWrapper.ax',
    'UpDx11.ax'
)

$requiredDecoderFiles = @(
    'CLM4Splt.ax',
    'CLCVD\clcvd.ax',
    'CLCVD\264dsse2.dll',
    'CLCVD\pthreadVC2.dll',
    'CLCVD\vdshell.dll'
)

$missingFiles = $requiredFiles | Where-Object {
    -not (Test-Path -LiteralPath (Join-Path $sourceMediaCache $_) -PathType Leaf)
}
if ($missingFiles)
{
    throw "MediaObj runtime source is incomplete. Missing: $($missingFiles -join ', ')"
}

$missingDecoderFiles = $requiredDecoderFiles | Where-Object {
    -not (Test-Path -LiteralPath (Join-Path $sourceDecoderPack $_) -PathType Leaf)
}
if ($missingDecoderFiles)
{
    throw "MediaObj decoder source is incomplete. Missing: $($missingDecoderFiles -join ', ')"
}

New-Item -ItemType Directory -Path $destinationMediaCache -Force | Out-Null
foreach ($file in $requiredFiles)
{
    Copy-Item -LiteralPath (Join-Path $sourceMediaCache $file) `
        -Destination (Join-Path $destinationMediaCache $file) -Force
}

foreach ($file in $requiredDecoderFiles)
{
    $destinationFile = Join-Path $destinationDecoderPack $file
    New-Item -ItemType Directory -Path (Split-Path $destinationFile) -Force | Out-Null
    Copy-Item -LiteralPath (Join-Path $sourceDecoderPack $file) `
        -Destination $destinationFile -Force
}

if (Test-Path -LiteralPath $legacySimba -PathType Container)
{
    Remove-Item -LiteralPath $legacySimba -Recurse -Force
}

if (-not $Quiet)
{
    Write-Host "MediaObj source runtime staged in $destinationRuntime"
}
