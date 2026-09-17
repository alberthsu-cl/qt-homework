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

    // QML events consumed by the controller
    property string openPickerButton: "colorPicker.openButton"
    property string addCustomButton:  "colorPicker.addCustomButton"
    property string resetButton:      "colorPicker.resetButton"
    property string applyButton:      "colorPicker.applyButton"
    property string cancelButton:     "colorPicker.cancelButton"
}
