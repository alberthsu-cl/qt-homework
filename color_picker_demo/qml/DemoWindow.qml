import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtKit

Window {
    id: root
    visible: true
    flags: Qt.FramelessWindowHint
    color: "#171A21"
    title: "Color Picker"
    property UIWindow binding
    property real hue: 215
    property real saturation: 0.91
    property real value: 0.96
    property bool syncing: false
    property var basicColors: ["#F44336", "#FF9800", "#FFEB3B", "#4CAF50", "#00BCD4", "#2196F3", "#3F51B5", "#9C27B0", "#E91E63", "#FFFFFF", "#808080", "#000000"]

    Component.onCompleted: { binding = qmlContext.bindWindow(this, DemoName.window); setFromHex(DemoProperty.colorHex, false) }
    Component.onDestruction: qmlContext.unbind(this, DemoName.window)
    function clamp(n, low, high) { return Math.max(low, Math.min(high, n)) }
    function rgbToHex(c) { function part(n) { var v = Math.round(n * 255).toString(16); return v.length === 1 ? "0" + v : v }; return "#" + part(c.r) + part(c.g) + part(c.b) }
    function hsvToHex() { return rgbToHex(Qt.hsva(hue / 360.0, saturation, value, 1.0)) }
    function setFromHex(hex, notify) {
        if (!hex || !/^#[0-9a-fA-F]{6}$/.test(hex)) return
        var r = parseInt(hex.slice(1, 3), 16) / 255.0
        var g = parseInt(hex.slice(3, 5), 16) / 255.0
        var b = parseInt(hex.slice(5, 7), 16) / 255.0
        var max = Math.max(r, g, b), min = Math.min(r, g, b), delta = max - min, nextHue = 0
        if (delta > 0) { if (max === r) nextHue = 60 * (((g - b) / delta + 6) % 6); else if (max === g) nextHue = 60 * ((b - r) / delta + 2); else nextHue = 60 * ((r - g) / delta + 4) }
        syncing = true; hue = nextHue; saturation = max === 0 ? 0 : delta / max; value = max; syncing = false; hueBoard.requestPaint()
        if (notify) publishFromHsv()
    }
    function publishFromHsv() { if (!syncing) { DemoProperty.colorHex = hsvToHex(); if (colorChangedEvent.binding) colorChangedEvent.binding.clicked() }; hueBoard.requestPaint() }
    Connections { target: DemoProperty; function onColorHexChanged() { root.setFromHex(DemoProperty.colorHex, false) } }
    Rectangle {
        anchors.fill: parent; color: "#171A21"
        Label { id: heading; anchors.left: parent.left; anchors.leftMargin: 24; anchors.top: parent.top; anchors.topMargin: 18; text: "Color Picker"; color: "#F2F5FA"; font.pixelSize: 20; font.bold: true }
        Label { anchors.right: parent.right; anchors.rightMargin: 24; anchors.verticalCenter: heading.verticalCenter; text: "HSV color space"; color: "#AEB8C6"; font.pixelSize: 12 }

        Rectangle {
            id: pickerPanel
            anchors.left: parent.left; anchors.leftMargin: 24; anchors.top: heading.bottom; anchors.topMargin: 18
            width: Math.max(360, parent.width * 0.55); height: Math.max(280, parent.height - 142)
            radius: 6; color: "#222833"; border.color: "#333C49"
            Canvas {
                id: hueBoard
                anchors.left: parent.left; anchors.leftMargin: 16; anchors.top: parent.top; anchors.topMargin: 16
                width: Math.max(220, parent.width - 74); height: Math.max(210, parent.height - 32)
                onPaint: {
                    var c = getContext("2d"); c.clearRect(0, 0, width, height)
                    var h = c.createLinearGradient(0, 0, width, 0); h.addColorStop(0, "#ffffff"); h.addColorStop(1, Qt.hsva(root.hue / 360.0, 1, 1, 1)); c.fillStyle = h; c.fillRect(0, 0, width, height)
                    var v = c.createLinearGradient(0, 0, 0, height); v.addColorStop(0, "rgba(0,0,0,0)"); v.addColorStop(1, "#000000"); c.fillStyle = v; c.fillRect(0, 0, width, height)
                }
                MouseArea { anchors.fill: parent; function select(mouse) { root.saturation = root.clamp(mouse.x / parent.width, 0, 1); root.value = root.clamp(1 - mouse.y / parent.height, 0, 1); root.publishFromHsv() } onPressed: select(mouse); onPositionChanged: if (pressed) select(mouse) }
                Rectangle { x: root.saturation * hueBoard.width - width / 2; y: (1 - root.value) * hueBoard.height - height / 2; width: 14; height: 14; radius: 7; color: "transparent"; border.color: "white"; border.width: 2; Rectangle { anchors.centerIn: parent; width: 6; height: 6; radius: 3; color: "#151A20" } }
            }
            Slider { id: hueSlider; anchors.left: hueBoard.right; anchors.leftMargin: 18; anchors.verticalCenter: hueBoard.verticalCenter; width: hueBoard.height; rotation: -90; from: 0; to: 360; value: root.hue; onValueChanged: if (!root.syncing) { root.hue = value; root.publishFromHsv() } }
        }

        Rectangle {
            id: infoPanel
            anchors.left: pickerPanel.right; anchors.leftMargin: 18; anchors.right: parent.right; anchors.rightMargin: 24; anchors.top: pickerPanel.top
            height: pickerPanel.height; radius: 6; color: "#222833"; border.color: "#333C49"
            Rectangle { id: preview; anchors.left: parent.left; anchors.leftMargin: 16; anchors.top: parent.top; anchors.topMargin: 16; width: 58; height: 58; radius: 5; color: root.hsvToHex(); border.color: "#FFFFFF" }
            Label { anchors.left: preview.right; anchors.leftMargin: 12; anchors.verticalCenter: preview.verticalCenter; text: "Selected color\n" + root.hsvToHex(); color: "#F2F5FA"; font.pixelSize: 13; lineHeight: 1.35 }
            Label { id: hexLabel; anchors.left: parent.left; anchors.leftMargin: 16; anchors.top: preview.bottom; anchors.topMargin: 18; text: "Hex"; color: "#AEB8C6"; font.pixelSize: 12 }
            TextField { id: hexField; anchors.left: hexLabel.right; anchors.leftMargin: 12; anchors.right: parent.right; anchors.rightMargin: 16; anchors.verticalCenter: hexLabel.verticalCenter; text: DemoProperty.colorHex; color: "#F2F5FA"; selectByMouse: true; onEditingFinished: root.setFromHex(text, true) }
            Label { id: hueLabel; anchors.left: parent.left; anchors.leftMargin: 16; anchors.top: hexLabel.bottom; anchors.topMargin: 18; text: "Hue"; color: "#AEB8C6"; font.pixelSize: 12 }
            SpinBox { anchors.left: hueLabel.right; anchors.leftMargin: 12; anchors.verticalCenter: hueLabel.verticalCenter; from: 0; to: 360; value: Math.round(root.hue); onValueModified: { root.hue = value; root.publishFromHsv() } }
            Label { id: satLabel; anchors.left: parent.left; anchors.leftMargin: 16; anchors.top: hueLabel.bottom; anchors.topMargin: 12; text: "Saturation"; color: "#AEB8C6"; font.pixelSize: 12 }
            SpinBox { anchors.left: satLabel.right; anchors.leftMargin: 12; anchors.verticalCenter: satLabel.verticalCenter; from: 0; to: 100; value: Math.round(root.saturation * 100); onValueModified: { root.saturation = value / 100.0; root.publishFromHsv() } }
            Label { id: valLabel; anchors.left: parent.left; anchors.leftMargin: 16; anchors.top: satLabel.bottom; anchors.topMargin: 12; text: "Value"; color: "#AEB8C6"; font.pixelSize: 12 }
            SpinBox { anchors.left: valLabel.right; anchors.leftMargin: 12; anchors.verticalCenter: valLabel.verticalCenter; from: 0; to: 100; value: Math.round(root.value * 100); onValueModified: { root.value = value / 100.0; root.publishFromHsv() } }
            Label { anchors.left: parent.left; anchors.leftMargin: 16; anchors.top: valLabel.bottom; anchors.topMargin: 18; text: "RGB " + Math.round(Qt.hsva(root.hue / 360, root.saturation, root.value, 1).r * 255) + ", " + Math.round(Qt.hsva(root.hue / 360, root.saturation, root.value, 1).g * 255) + ", " + Math.round(Qt.hsva(root.hue / 360, root.saturation, root.value, 1).b * 255); color: "#AEB8C6"; font.pixelSize: 12 }
        }

        Rectangle {
            id: presets
            anchors.left: parent.left; anchors.leftMargin: 24; anchors.right: parent.right; anchors.rightMargin: 24; anchors.bottom: parent.bottom; anchors.bottomMargin: 12
            height: 94; radius: 6; color: "#222833"; border.color: "#333C49"
            Label { anchors.left: parent.left; anchors.leftMargin: 14; anchors.top: parent.top; anchors.topMargin: 10; text: "Basic colors"; color: "#AEB8C6"; font.pixelSize: 12 }
            Row { anchors.left: parent.left; anchors.leftMargin: 14; anchors.top: parent.top; anchors.topMargin: 31; spacing: 7; Repeater { model: root.basicColors; delegate: Rectangle { width: 22; height: 22; radius: 3; color: modelData; border.color: "#6E7988"; MouseArea { anchors.fill: parent; onClicked: root.setFromHex(modelData, true) } } } }
            Label { anchors.left: parent.left; anchors.leftMargin: 14; anchors.bottom: parent.bottom; anchors.bottomMargin: 10; text: "Custom colors"; color: "#AEB8C6"; font.pixelSize: 12 }
            Row { anchors.left: parent.left; anchors.leftMargin: 103; anchors.bottom: parent.bottom; anchors.bottomMargin: 8; spacing: 7; Repeater { model: DemoProperty.customColors === "" ? [] : DemoProperty.customColors.split(","); delegate: Rectangle { width: 22; height: 22; radius: 3; color: modelData; border.color: "#6E7988"; MouseArea { anchors.fill: parent; onClicked: root.setFromHex(modelData, true) } } } }
            Button { anchors.right: parent.right; anchors.rightMargin: 14; anchors.bottom: parent.bottom; anchors.bottomMargin: 7; text: "Add custom"; property UIButton binding; Component.onCompleted: binding = qmlContext.bindButton(this, DemoName.zoomInButton); Component.onDestruction: qmlContext.unbind(this, DemoName.zoomInButton); onClicked: binding.clicked() }
        }
        Button { anchors.right: presets.right; anchors.bottom: presets.top; anchors.bottomMargin: 10; text: "Reset"; property UIButton binding; Component.onCompleted: binding = qmlContext.bindButton(this, DemoName.zoomOutButton); Component.onDestruction: qmlContext.unbind(this, DemoName.zoomOutButton); onClicked: binding.clicked() }
        Button { id: colorChangedEvent; visible: false; property UIButton binding; Component.onCompleted: binding = qmlContext.bindButton(this, DemoName.colorChangedButton); Component.onDestruction: qmlContext.unbind(this, DemoName.colorChangedButton); onClicked: if (binding) binding.clicked() }
    }
}
