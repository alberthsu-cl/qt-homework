pragma Singleton
import QtQuick

// The QML half of the shared name table. Must match src/DemoNames.h exactly.
// PDR's equivalent: skinQt/qml/DeductCredit/DeductCreditName.qml
QtObject
{
    property string window:         "demo.window"

    // C++ writes into these two
    property string caption:        "demo.caption"
    property string imagePath:      "demo.imagePath"

    // these four report back to C++
    property string zoomInButton:   "demo.zoomInButton"
    property string zoomOutButton:  "demo.zoomOutButton"
    property string rotateButton:   "demo.rotateButton"
    property string resetButton:    "demo.resetButton"
}
