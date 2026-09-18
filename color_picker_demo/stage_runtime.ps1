<#
    Compatibility entry point for existing build instructions.

    The committed, PDR-independent runtime bundle and implementation now live
    in ..\QtKit. This wrapper preserves the original command line.
#>
[CmdletBinding()]
param(
    [switch]$Release,
    [switch]$Verify,
    [switch]$Clean
)

$ErrorActionPreference = 'Stop'
$stageScript = Join-Path $PSScriptRoot '..\QtKit\stage_runtime.ps1'
$destination = Join-Path $PSScriptRoot 'bin_x64'

& $stageScript -Destination $destination -Clean:$Clean -Verify:$Verify
