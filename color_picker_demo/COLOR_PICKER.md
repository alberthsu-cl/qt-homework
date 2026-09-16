# QtKit Color Picker Homework

This is a standalone MFC plus QtKit color picker shaped like a portable PDR
feature. It deliberately does not modify `src`, `skinQt`, or a PDR solution.

## What is implemented

- HSV hue and saturation canvas, with a value slider.
- Live color preview, hexadecimal value, HSV spin boxes, and RGB readout.
- Basic preset swatches.
- Add, load, and save up to twelve custom colors in `custom_colors.txt` next
  to the executable's working directory.
- A QtKit `UIProperty` named `colorPicker.property` for the selected color and
  serialized custom palette.

## Layer boundary

`DemoWindow.qml` contains presentation and input only. `CDemoController` owns
the selected RGB value and custom-color persistence. `CDemoViewController` is
the one QtKit adapter: it loads QML, binds the HWND, subscribes to user color
changes, and publishes controller state.

Do not use `setPropertyChangedAction` for high-frequency HSV edits in this
QtKit runtime. It mutates QtKit's listener list while dispatching a property
change and can crash. QML writes `colorHex` to `UIProperty` first, then signals
the dedicated `colorPicker.colorChanged` button event; the VC reads the value
there and updates the controller safely.

## Run

```bat
cd qt-homework\color_picker_demo
powershell -ExecutionPolicy Bypass -File .\stage_runtime.ps1 -Verify
msbuild MfcQmlDemo.vcxproj /p:Configuration=Debug /p:Platform=x64
bin_x64\ColorPickerDemo.exe
```

The project was built with `Debug|x64`, and the running process completed the
QtKit load, bound `colorPicker.window` and `colorPicker.property`, published
the initial color and palette, and resized the embedded QML child window.

## PDR extraction map

When this becomes a PDR feature, keep the controller and the QML state contract
but replace the homework host:

| Homework | PDR destination |
| --- | --- |
| `CDemoController` | feature controller/model |
| `CDemoViewController` | `CQtEntryViewController` or modal controller |
| `qml/DemoWindow.qml` | `skinQt/qml/<Feature>/<Feature>.qml` |
| `qml/DemoProperty.qml` | `<Feature>Property.qml` |
| `qml/DemoName.qml` and `src/DemoNames.h` | matching PDR name table |

An MFC button should only create/present the entry controller and consume the
selected `#RRGGBB` result. Applying the color to media is intentionally outside
this homework.
