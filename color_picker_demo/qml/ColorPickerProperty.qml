import QtQuick
import QtKit

// State channel owned by the independently loaded picker entry. Keeping this
// binding separate prevents unloading the picker from unbinding DemoWindow's
// long-lived DemoProperty object.
UIProperty {
    id: root
    property UIProperty binding

    property string colorHex: "#3B82F6"
    property string appliedColorHex: "#3B82F6"
    property bool pickerVisible: true
    property string customColors: ""

    Component.onCompleted: binding = qmlContext.bindProperty(this, DemoName.pickerProperty)
    Component.onDestruction: qmlContext.unbind(this, DemoName.pickerProperty)
}
