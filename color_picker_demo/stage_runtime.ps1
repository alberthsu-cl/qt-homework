<#
    stage_runtime.ps1 - build a self-contained runtime folder for QmlColorPiker.

    Copies the MINIMUM set of files the demo needs out of the product output
    (bin_x64\PowerDirector) into arch_review\mfc_qml_demo\bin_x64, so you can
    see exactly what "hosting QML in an MFC app" costs at runtime.

    The manifest below is the point of this script. Every entry is here because
    the running process actually loaded it - the list was taken from
    Process.Modules of a live QmlColorPiker.exe, not from guesswork - and each
    group carries a note on what breaks without it.

        .\stage_runtime.ps1              stage runtime files
        .\stage_runtime.ps1 -Release     accepted for compatibility
        .\stage_runtime.ps1 -Clean       wipe staged files first (keeps the .exe)
        .\stage_runtime.ps1 -Verify      list the full staged tree at the end

    Run it once; re-run after `emma` updates QtKit or the Qt6 DLLs.
#>
[CmdletBinding()]
param(
    [switch]$Release,
    [switch]$Verify,
    [switch]$Clean
)

$ErrorActionPreference = 'Stop'

$src = Join-Path $PSScriptRoot '..\..\bin_x64\PowerDirector'
$dst = Join-Path $PSScriptRoot 'bin_x64'
$src = (Resolve-Path $src).Path

# Debug builds of PDR ship both variants side by side: Qt6Core.dll and
# Qt6Cored.dll, qwindows.dll and qwindowsd.dll. The demo always loads the
# RELEASE Qt, because QtKitWrapper's "#ifdef DEBUG" branch is dead in this repo
# (no .vcxproj defines bare DEBUG) and so is ours - we deliberately mirror it.
# QtKit's public interface returns std::string by value. Both application
# configurations therefore use the release CRT/MFC so allocation and
# destruction happen through one compatible STL ABI. The Debug configuration
# still emits symbols and disables optimization.

# =============================================================================
#  MANIFEST
# =============================================================================
$groups = [ordered]@{

    'the bridge itself' = @(
        # The only DLL this demo talks to directly, via LoadLibraryEx +
        # GetProcAddress("createQtKit"). Everything below is its dependency.
        'QtKit.dll'
    )

    'Qt core runtime' = @(
        # QtKit links these statically, so they must sit beside it.
        'Qt6Core.dll'
        'Qt6Gui.dll'
        'Qt6Qml.dll'
        'Qt6Quick.dll'
        'Qt6Network.dll'      # QtKit's NetworkAccessManagerFactory / disk cache
        'Qt6OpenGL.dll'       # scene-graph backend
        'Qt6Svg.dll'          # pulled in by the qsvg image plugin
        'Qt6QmlMeta.dll'
        'Qt6QmlModels.dll'
        'Qt6QmlWorkerScript.dll'
    )

    'Qt Quick Controls' = @(
        # Needed because DemoWindow.qml uses Button and Label from
        # QtQuick.Controls. A QML file that sticks to plain QtQuick items
        # (Rectangle, Text, Image, MouseArea) does not need this group at all.
        'Qt6QuickControls2.dll'
        'Qt6QuickControls2Basic.dll'
        'Qt6QuickControls2Impl.dll'
        'Qt6QuickTemplates2.dll'
    )

    'Qt platform plugin (MANDATORY)' = @(
        # Without this exact path Qt aborts at startup with
        #   "This application failed to start because no Qt platform plugin
        #    could be initialized."
        # It is the single most common packaging mistake with Qt.
        'platforms\qwindows.dll'
    )

    'image format plugins' = @(
        # One per format the demo can open. Drop any you do not need - PNG and
        # BMP are built into Qt6Gui and need no plugin at all.
        'imageformats\qjpeg.dll'
        'imageformats\qgif.dll'
        'imageformats\qico.dll'
        'imageformats\qwebp.dll'
        'imageformats\qsvg.dll'
    )

    'QML modules' = @(
        # Qt resolves "import QtQuick" by walking qml\ next to the .exe. Each
        # module needs its plugin DLL *and* its qmldir / .qmltypes companions;
        # copying only the DLLs leaves Qt unable to resolve the import.
        'qml\QtQuick\qmldir'
        'qml\QtQuick\plugins.qmltypes'
        # qtquick2plugin.dll is deliberately NOT here. Its qmldir declares it
        # "optional plugin", and QtKit links Qt6Quick statically, so the QtQuick
        # types are already registered by the time the import is resolved - the
        # plugin never loads. Verified by removing it and re-running.

        'qml\QtQuick\Window\qmldir'
        'qml\QtQuick\Window\quickwindow.qmltypes'
        'qml\QtQuick\Window\quickwindowplugin.dll'

        'qml\QtQuick\Templates\qmldir'
        'qml\QtQuick\Templates\plugins.qmltypes'
        'qml\QtQuick\Templates\qtquicktemplates2plugin.dll'

        'qml\QtQuick\Controls\qmldir'
        'qml\QtQuick\Controls\plugins.qmltypes'
        'qml\QtQuick\Controls\qtquickcontrols2plugin.dll'

        'qml\QtQuick\Controls\impl\qmldir'
        'qml\QtQuick\Controls\impl\plugins.qmltypes'
        'qml\QtQuick\Controls\impl\qtquickcontrols2implplugin.dll'
    )

    'C++ runtime' = @(
        'MSVCP140.dll'
        'MSVCP140_1.dll'
        'MSVCP140_2.dll'
        'VCRUNTIME140.dll'
        'VCRUNTIME140_1.dll'
    )
}

# The Basic style ships its controls as .qml source, so this directory is
# copied wholesale rather than listed file by file (64 .qml + qmldir +
# qmltypes + plugin). Swap QtQuick.Controls for plain QtQuick items in your
# .qml and this ~510 KB disappears.
$wholeDirs = @(
    'qml\QtQuick\Controls\Basic'
)

$groups['MFC'] = @('mfc140u.dll')

# =============================================================================
#  COPY
# =============================================================================
Write-Host ""
Write-Host "  source : $src"
Write-Host "  target : $dst"
Write-Host "  runtime: Release (required by QtKit ABI)"
Write-Host ""

if ($Clean -and (Test-Path -LiteralPath $dst)) {
    # Keep our own build output; wipe everything staged.
    Get-ChildItem -LiteralPath $dst -Force |
        Where-Object { $_.Name -notlike 'QmlColorPiker.*' } |
        Remove-Item -Recurse -Force
    Write-Host "  cleaned staged files (build output kept)"
    Write-Host ""
}

$copied = 0; $bytes = 0; $missing = @()

foreach ($name in $groups.Keys) {
    $files = $groups[$name]
    $groupBytes = 0; $groupCount = 0
    foreach ($rel in $files) {
        $from = Join-Path $src $rel
        $to   = Join-Path $dst $rel
        if (-not (Test-Path -LiteralPath $from)) { $missing += $rel; continue }
        $dir = Split-Path $to -Parent
        if (-not (Test-Path -LiteralPath $dir)) { New-Item -ItemType Directory -Force -Path $dir | Out-Null }
        Copy-Item -LiteralPath $from -Destination $to -Force
        $len = (Get-Item -LiteralPath $to).Length
        $groupBytes += $len; $groupCount++
    }
    $copied += $groupCount; $bytes += $groupBytes
    Write-Host ("  {0,-34} {1,3} files  {2,7:N1} MB" -f $name, $groupCount, ($groupBytes / 1MB))
}

foreach ($rel in $wholeDirs) {
    $from = Join-Path $src $rel
    $to   = Join-Path $dst $rel
    if (-not (Test-Path -LiteralPath $from)) { $missing += $rel; continue }
    if (-not (Test-Path -LiteralPath $to)) { New-Item -ItemType Directory -Force -Path $to | Out-Null }
    # -Path, not -LiteralPath: the wildcard has to expand.
    Copy-Item -Path (Join-Path $from '*') -Destination $to -Recurse -Force

    # PDR ships release and debug plugins side by side (foo.dll + food.dll).
    # We always load the release Qt, so drop any debug twin that has a
    # release sibling - that alone is most of what a wholesale copy wastes.
    Get-ChildItem -LiteralPath $to -Recurse -File -Filter '*d.dll' | ForEach-Object {
        $twin = Join-Path $_.DirectoryName ($_.BaseName.Substring(0, $_.BaseName.Length - 1) + '.dll')
        if (Test-Path -LiteralPath $twin) { Remove-Item -LiteralPath $_.FullName -Force }
    }

    $dirFiles = Get-ChildItem -LiteralPath $to -Recurse -File
    $copied += $dirFiles.Count
    $dirBytes = ($dirFiles | Measure-Object Length -Sum).Sum
    $bytes += $dirBytes
    Write-Host ("  {0,-34} {1,3} files  {2,7:N1} MB" -f $rel, $dirFiles.Count, ($dirBytes / 1MB))
}

Write-Host ""
Write-Host ("  TOTAL  {0} files, {1:N1} MB" -f $copied, ($bytes / 1MB))

if ($missing.Count) {
    Write-Host ""
    Write-Host "  MISSING from the product folder:" -ForegroundColor Yellow
    $missing | ForEach-Object { Write-Host "    $_" -ForegroundColor Yellow }
    Write-Host "  Run emma in the repo root, then emma_move_files.py." -ForegroundColor Yellow
}

if ($Verify) {
    Write-Host ""
    Write-Host "  Staged tree:"
    Get-ChildItem -LiteralPath $dst -Recurse -File |
        ForEach-Object { $_.FullName.Substring($dst.Length + 1) } | Sort-Object |
        ForEach-Object { Write-Host "    $_" }
}

Write-Host ""
Write-Host "  Now build the project - it outputs QmlColorPiker.exe into this same folder."
Write-Host ""
