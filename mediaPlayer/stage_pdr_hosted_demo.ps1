[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$SourceExecutable,
    [string]$Destination,
    [switch]$Quiet
)

$ErrorActionPreference = 'Stop'

if ([string]::IsNullOrWhiteSpace($Destination))
{
    $Destination = Join-Path $PSScriptRoot '..\..\bin_x64\PowerDirector'
}

if (-not (Test-Path -LiteralPath $SourceExecutable -PathType Leaf))
{
    throw "Media Player executable not found: $SourceExecutable"
}

if (-not (Test-Path -LiteralPath $Destination -PathType Container))
{
    throw "PDR output directory not found: $Destination"
}

if (-not (Test-Path -LiteralPath (Join-Path $Destination 'PDR.exe') -PathType Leaf))
{
    throw "PDR.exe not found in deployment directory: $Destination"
}

$qmlSource = Join-Path $PSScriptRoot 'qml'
$qmlDestination = Join-Path $Destination 'MediaPlayerQml'

New-Item -ItemType Directory -Path $qmlDestination -Force | Out-Null
Copy-Item -LiteralPath $SourceExecutable -Destination (Join-Path $Destination 'MediaPlayer.exe') -Force
Copy-Item -Path (Join-Path $qmlSource '*') -Destination $qmlDestination -Recurse -Force

if (-not $Quiet)
{
    Write-Host "Deployed MediaPlayer.exe beside PDR.exe: $Destination"
}
