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
$destinationRuntime = Join-Path $Destination 'runtime'
$destinationMediaCache = Join-Path $destinationRuntime 'mediacache'
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

$missingFiles = $requiredFiles | Where-Object {
    -not (Test-Path -LiteralPath (Join-Path $sourceMediaCache $_) -PathType Leaf)
}
if ($missingFiles)
{
    throw "MediaObj runtime source is incomplete. Missing: $($missingFiles -join ', ')"
}

New-Item -ItemType Directory -Path $destinationMediaCache -Force | Out-Null
foreach ($file in $requiredFiles)
{
    Copy-Item -LiteralPath (Join-Path $sourceMediaCache $file) `
        -Destination (Join-Path $destinationMediaCache $file) -Force
}

if (Test-Path -LiteralPath $legacySimba -PathType Container)
{
    Remove-Item -LiteralPath $legacySimba -Recurse -Force
}

if (-not $Quiet)
{
    Write-Host "MO-only runtime staged in $destinationMediaCache"
}
