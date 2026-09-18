# QtKit runtime bundle

This folder makes the homework repository independent of a PowerDirector
checkout for compilation and local execution.

```text
QtKit/
  include/QtKit/Interface.h  C++ bridge interface used at build time
  runtime/                   QtKit, Qt 6, QML plugins, and MSVC/MFC runtime
  stage_runtime.ps1          copies runtime/ beside the built executable
```

Both Visual Studio projects run `stage_runtime.ps1` automatically after a
successful build. To stage or verify the ColorPicker runtime manually:

```powershell
powershell -ExecutionPolicy Bypass -File .\QtKit\stage_runtime.ps1 -Verify
```

The default destination is `color_picker_demo/bin_x64`. A different executable
folder can be supplied with `-Destination`.

The bundle contains the release QtKit ABI used by both projects. The ColorPicker
Debug configuration emits symbols and disables optimization while retaining
the release MSVC ABI used by `QtKit.dll`. The reference MFC demo additionally
carries the Debug CRT/MFC DLLs required by its `/MDd` host executable.
