pragma Singleton
import QtQuick

// The QML half of the shared name table. Must match src/DemoNames.h exactly.
// PDR's equivalent: skinQt/qml/DeductCredit/DeductCreditName.qml
QtObject
{
    property string window:         "colorPicker.window"

    // the shared state block - see DemoProperty.qml
    property string property:       "colorPicker.property"

    // C++ writes into these two
    property string caption:        "colorPicker.caption"
    property string photo:          "colorPicker.photo"

    // these four report back to C++
    property string zoomInButton:   "colorPicker.addCustomButton"
    property string zoomOutButton:  "colorPicker.resetButton"
    property string rotateButton:   "colorPicker.applyButton"
    property string resetButton:    "colorPicker.cancelButton"
    property string colorChangedButton: "colorPicker.colorChanged"
}
