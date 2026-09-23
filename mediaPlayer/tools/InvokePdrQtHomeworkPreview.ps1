param(
    [Parameter(Mandatory = $true)]
    [ValidateScript({ Test-Path -LiteralPath $_ -PathType Leaf })]
    [string]$MediaPath,

    [int]$ProcessId = 0
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

if ($ProcessId -ne 0) {
    $pdrProcess = Get-Process -Id $ProcessId
}
else {
    $pdrProcess = Get-Process -Name PDR, PDR_debug -ErrorAction Stop |
        Where-Object { $_.MainWindowHandle -ne [IntPtr]::Zero } |
        Select-Object -First 1
}

if ($null -eq $pdrProcess -or $pdrProcess.MainWindowHandle -eq [IntPtr]::Zero) {
    throw 'No PowerDirector main window was found. Start PDR and open the editing workspace first.'
}

Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;

public static class QtHomeworkPreviewNative
{
    [StructLayout(LayoutKind.Sequential)]
    public struct CopyDataStruct
    {
        public IntPtr dwData;
        public int cbData;
        public IntPtr lpData;
    }

    [DllImport("user32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    public static extern IntPtr SendMessage(
        IntPtr hWnd,
        uint message,
        IntPtr wParam,
        ref CopyDataStruct lParam);
}
'@

$resolvedPath = (Resolve-Path -LiteralPath $MediaPath).Path
$payload = [Text.Encoding]::Unicode.GetBytes("$resolvedPath`0")
$payloadBuffer = [Runtime.InteropServices.Marshal]::AllocHGlobal($payload.Length)

try {
    [Runtime.InteropServices.Marshal]::Copy($payload, 0, $payloadBuffer, $payload.Length)
    $command = [QtHomeworkPreviewNative+CopyDataStruct]::new()
    $command.dwData = [IntPtr]::new(0x51485052) # QHPR
    $command.cbData = $payload.Length
    $command.lpData = $payloadBuffer

    $result = [QtHomeworkPreviewNative]::SendMessage(
        $pdrProcess.MainWindowHandle,
        0x004A,
        [IntPtr]::Zero,
        [ref]$command)
    if ($result -eq [IntPtr]::Zero) {
        throw 'PDR rejected the preview request. Confirm that the Qt DisplayPanel is enabled and the Debug build contains the bridge.'
    }
}
finally {
    [Runtime.InteropServices.Marshal]::FreeHGlobal($payloadBuffer)
}

Write-Host "Requested PDR DisplayMediaPlayer preview: $resolvedPath"
