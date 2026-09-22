[CmdletBinding()]
param(
    [string]$Destination = (Join-Path $PSScriptRoot 'bin_x64'),
    [switch]$Quiet
)

$ErrorActionPreference = 'Stop'
$copyScript = Join-Path $PSScriptRoot '..\..\tools\CopyPlaybackRuntime.ps1'

if (-not (Test-Path -LiteralPath $copyScript -PathType Leaf))
{
    throw "Playback runtime copier not found: $copyScript"
}

& $copyScript -OutputDirectory $Destination

if (-not $Quiet)
{
    Write-Host "Playback runtime staged in $Destination"
}
