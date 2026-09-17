import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtKit

Window {
    id: root
    visible: true
    flags: Qt.FramelessWindowHint
    color: "#12161D"
    property UIWindow binding
    property real hue: 215
    property real saturation: 0.91
    property real value: 0.96
    property bool syncing: false
    property var basicColors: ["#F44336", "#FF9800", "#FFEB3B", "#4CAF50", "#00BCD4", "#2196F3", "#3F51B5", "#9C27B0", "#E91E63", "#FFFFFF", "#808080", "#000000"]
    property color previewTint: DemoProperty.pickerVisible ? DemoProperty.colorHex : DemoProperty.appliedColorHex

    Component.onCompleted: { binding = qmlContext.bindWindow(this, DemoName.window); setFromHex(DemoProperty.colorHex) }
    Component.onDestruction: qmlContext.unbind(this, DemoName.window)
    function clamp(n, low, high) { return Math.max(low, Math.min(high, n)) }
    function rgbToHex(c) { function p(n) { var v = Math.round(n * 255).toString(16); return v.length === 1 ? "0" + v : v }; return "#" + p(c.r) + p(c.g) + p(c.b) }
    function hsvToHex() { return rgbToHex(Qt.hsva(hue / 360.0, saturation, value, 1.0)) }
    function setFromHex(hex) {
        if (!hex || !/^#[0-9a-fA-F]{6}$/.test(hex)) return
        var r = parseInt(hex.slice(1, 3), 16) / 255.0, g = parseInt(hex.slice(3, 5), 16) / 255.0, b = parseInt(hex.slice(5, 7), 16) / 255.0
        var max = Math.max(r, g, b), min = Math.min(r, g, b), d = max - min, h = 0
        if (d > 0) { if (max === r) h = 60 * (((g - b) / d + 6) % 6); else if (max === g) h = 60 * ((b - r) / d + 2); else h = 60 * ((r - g) / d + 4) }
        syncing = true; hue = h; saturation = max === 0 ? 0 : d / max; value = max; syncing = false; hueBoard.requestPaint()
    }
    function publish() { if (!syncing) DemoProperty.colorHex = hsvToHex(); hueBoard.requestPaint() }
    function openPicker() { DemoProperty.colorHex = DemoProperty.appliedColorHex; DemoProperty.pickerVisible = true; setFromHex(DemoProperty.colorHex); openEvent.binding.clicked() }
    function applyPicker() { DemoProperty.appliedColorHex = DemoProperty.colorHex; DemoProperty.pickerVisible = false; applyEvent.binding.clicked() }
    function cancelPicker() { DemoProperty.colorHex = DemoProperty.appliedColorHex; DemoProperty.pickerVisible = false; cancelEvent.binding.clicked() }
    Connections { target: DemoProperty; function onColorHexChanged() { root.setFromHex(DemoProperty.colorHex) } }

    Rectangle {
        anchors.fill: parent
        color: "#12161D"
        Canvas { anchors.fill: parent; onPaint: { var c = getContext("2d"), s = 18; for (var y = 0; y < height; y += s) for (var x = 0; x < width; x += s) { c.fillStyle = ((x / s + y / s) % 2 === 0) ? "#1A202A" : "#202833"; c.fillRect(x, y, s, s) } } }
        Image {
            id: photo
            anchors.fill: parent; anchors.margins: 20
            fillMode: Image.PreserveAspectFit; asynchronous: false
            property UIImage binding
            source: binding ? binding.source : ""; visible: status === Image.Ready
            Component.onCompleted: binding = qmlContext.bindImage(this, DemoName.photo)
            Component.onDestruction: qmlContext.unbind(this, DemoName.photo)
        }
        Rectangle { anchors.fill: photo; visible: photo.visible; color: root.previewTint; opacity: 0.32 }
        Label { anchors.centerIn: parent; visible: !photo.visible; text: "Open a preview image from File > Open Preview Image..."; color: "#B8C2D0"; font.pixelSize: 16 }

        Rectangle {
            anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom; height: 44; color: "#E812161D"
            Label { anchors.left: parent.left; anchors.leftMargin: 14; anchors.right: colorButton.left; anchors.rightMargin: 52; anchors.verticalCenter: parent.verticalCenter; elide: Text.ElideRight; color: "#EAF0F8"; font.pixelSize: 12; property UILabel binding; text: binding ? binding.text : "No image loaded"; Component.onCompleted: binding = qmlContext.bindLabel(this, DemoName.caption); Component.onDestruction: qmlContext.unbind(this, DemoName.caption) }
            Rectangle { anchors.right: colorButton.left; anchors.rightMargin: 8; anchors.verticalCenter: parent.verticalCenter; width: 22; height: 22; radius: 3; color: DemoProperty.appliedColorHex; border.color: "#FFFFFF" }
            Rectangle { id: colorButton; anchors.right: parent.right; anchors.rightMargin: 14; anchors.verticalCenter: parent.verticalCenter; width: 70; height: 28; radius: 3; color: "#313B4B"; border.color: "#5B6A80"; Label { anchors.centerIn: parent; text: "Color"; color: "#F2F5FA"; font.pixelSize: 12 } MouseArea { anchors.fill: parent; onClicked: root.openPicker() } }
        }

        Rectangle {
            anchors.fill: parent; visible: DemoProperty.pickerVisible; color: "#AA000000"
            Rectangle {
                id: picker
                anchors.centerIn: parent
                width: 560; height: 420; radius: 6
                color: "#242B37"; border.color: "#4D5A6D"
                Label { anchors.left: parent.left; anchors.leftMargin: 18; anchors.top: parent.top; anchors.topMargin: 15; text: "Color Picker"; color: "#F4F7FB"; font.pixelSize: 18; font.bold: true }
                Label { anchors.right: parent.right; anchors.rightMargin: 18; anchors.top: parent.top; anchors.topMargin: 19; text: "Live preview"; color: "#84D9A4"; font.pixelSize: 12 }
                Rectangle { id: swatch; anchors.left: parent.left; anchors.leftMargin: 18; anchors.top: parent.top; anchors.topMargin: 52; width: 44; height: 44; radius: 4; color: root.hsvToHex(); border.color: "#F4F7FB" }
                TextField { id: hexField; anchors.left: swatch.right; anchors.leftMargin: 12; anchors.right: parent.right; anchors.rightMargin: 18; anchors.verticalCenter: swatch.verticalCenter; height: 30; text: DemoProperty.colorHex; color: "#F4F7FB"; selectByMouse: true; background: Rectangle { radius: 3; color: "#151A22"; border.color: "#56657A" } onEditingFinished: { root.setFromHex(text); root.publish() } }

                Canvas {
                    id: hueBoard
                    anchors.left: parent.left; anchors.leftMargin: 18; anchors.top: swatch.bottom; anchors.topMargin: 18; width: 260; height: 170
                    onPaint: { var c = getContext("2d"); var h = c.createLinearGradient(0, 0, width, 0); h.addColorStop(0, "#FFFFFF"); h.addColorStop(1, Qt.hsva(root.hue / 360.0, 1, 1, 1)); c.fillStyle = h; c.fillRect(0, 0, width, height); var v = c.createLinearGradient(0, 0, 0, height); v.addColorStop(0, "rgba(0,0,0,0)"); v.addColorStop(1, "#000000"); c.fillStyle = v; c.fillRect(0, 0, width, height) }
                    MouseArea { anchors.fill: parent; function select(mouse) { root.saturation = root.clamp(mouse.x / parent.width, 0, 1); root.value = root.clamp(1 - mouse.y / parent.height, 0, 1); root.publish() } onPressed: select(mouse); onPositionChanged: if (pressed) select(mouse) }
                    Rectangle { x: root.saturation * hueBoard.width - 7; y: (1 - root.value) * hueBoard.height - 7; width: 14; height: 14; radius: 7; color: "transparent"; border.color: "white"; border.width: 2 }
                }
                Rectangle { id: hueTrack; anchors.left: hueBoard.right; anchors.leftMargin: 18; anchors.top: hueBoard.top; width: 10; height: hueBoard.height; radius: 5; color: "#151A22"; border.color: "#56657A"; Rectangle { y: (1 - root.hue / 360) * (hueTrack.height - height); width: 18; height: 18; x: -4; radius: 9; color: "#F4F7FB" } MouseArea { anchors.fill: parent; function select(mouse) { root.hue = root.clamp((1 - mouse.y / parent.height) * 360, 0, 360); root.publish() } onPressed: select(mouse); onPositionChanged: if (pressed) select(mouse) } }

                Column {
                    anchors.left: hueTrack.right; anchors.leftMargin: 28; anchors.right: parent.right; anchors.rightMargin: 18; anchors.top: hueBoard.top; spacing: 13
                    Repeater {
                        model: ["Hue", "Saturation", "Value"]
                        delegate: Row {
                            spacing: 8
                            Label { width: 72; anchors.verticalCenter: parent.verticalCenter; text: modelData; color: "#B5C0D0"; font.pixelSize: 12 }
                            Rectangle { width: 24; height: 24; color: "#151A22"; border.color: "#56657A"; Label { anchors.centerIn: parent; text: "-"; color: "#F4F7FB" } MouseArea { anchors.fill: parent; onClicked: { if (index === 0) root.hue = root.clamp(root.hue - 1, 0, 360); else if (index === 1) root.saturation = root.clamp(root.saturation - .01, 0, 1); else root.value = root.clamp(root.value - .01, 0, 1); root.publish() } } }
                            Label { width: 34; anchors.verticalCenter: parent.verticalCenter; horizontalAlignment: Text.AlignHCenter; text: index === 0 ? Math.round(root.hue) : (index === 1 ? Math.round(root.saturation * 100) : Math.round(root.value * 100)); color: "#F4F7FB"; font.pixelSize: 12 }
                            Rectangle { width: 24; height: 24; color: "#151A22"; border.color: "#56657A"; Label { anchors.centerIn: parent; text: "+"; color: "#F4F7FB" } MouseArea { anchors.fill: parent; onClicked: { if (index === 0) root.hue = root.clamp(root.hue + 1, 0, 360); else if (index === 1) root.saturation = root.clamp(root.saturation + .01, 0, 1); else root.value = root.clamp(root.value + .01, 0, 1); root.publish() } } }
                        }
                    }
                }

                Row { anchors.left: parent.left; anchors.leftMargin: 18; anchors.bottom: parent.bottom; anchors.bottomMargin: 18; spacing: 7; Repeater { model: root.basicColors; delegate: Rectangle { width: 18; height: 18; radius: 3; color: modelData; border.color: "#718098"; MouseArea { anchors.fill: parent; onClicked: { root.setFromHex(modelData); root.publish() } } } } }
                Rectangle { anchors.right: cancelButton.left; anchors.rightMargin: 8; anchors.bottom: parent.bottom; anchors.bottomMargin: 14; width: 86; height: 28; radius: 3; color: "#3D76CE"; Label { anchors.centerIn: parent; text: "Apply"; color: "white"; font.pixelSize: 12 } MouseArea { anchors.fill: parent; onClicked: root.applyPicker() } }
                Rectangle { id: cancelButton; anchors.right: parent.right; anchors.rightMargin: 18; anchors.bottom: parent.bottom; anchors.bottomMargin: 14; width: 86; height: 28; radius: 3; color: "#343E4D"; border.color: "#5B6A80"; Label { anchors.centerIn: parent; text: "Cancel"; color: "#F4F7FB"; font.pixelSize: 12 } MouseArea { anchors.fill: parent; onClicked: root.cancelPicker() } }
            }
        }

        Button { id: openEvent; visible: false; property UIButton binding; Component.onCompleted: binding = qmlContext.bindButton(this, DemoName.openPickerButton); Component.onDestruction: qmlContext.unbind(this, DemoName.openPickerButton) }
        Button { id: applyEvent; visible: false; property UIButton binding; Component.onCompleted: binding = qmlContext.bindButton(this, DemoName.applyButton); Component.onDestruction: qmlContext.unbind(this, DemoName.applyButton) }
        Button { id: cancelEvent; visible: false; property UIButton binding; Component.onCompleted: binding = qmlContext.bindButton(this, DemoName.cancelButton); Component.onDestruction: qmlContext.unbind(this, DemoName.cancelButton) }
    }
}
