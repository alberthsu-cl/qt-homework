import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtKit

Window {
    id: root
    visible: true
    flags: Qt.FramelessWindowHint
    color: "#171A21"
    title: "QmlColorPiker"

    property UIWindow binding
    property real hue: 215
    property real saturation: 0.91
    property real value: 0.96
    property bool syncing: false
    property var basicColors: ["#F44336", "#FF9800", "#FFEB3B", "#4CAF50", "#00BCD4", "#2196F3", "#3F51B5", "#9C27B0", "#E91E63", "#FFFFFF", "#808080", "#000000"]
    property color previewTint: DemoProperty.pickerVisible ? DemoProperty.colorHex : DemoProperty.appliedColorHex

    Component.onCompleted: {
        binding = qmlContext.bindWindow(this, DemoName.window)
        setFromHex(DemoProperty.colorHex, false)
    }
    Component.onDestruction: qmlContext.unbind(this, DemoName.window)

    function clamp(n, low, high) { return Math.max(low, Math.min(high, n)) }
    function rgbToHex(c) {
        function part(n) { var v = Math.round(n * 255).toString(16); return v.length === 1 ? "0" + v : v }
        return "#" + part(c.r) + part(c.g) + part(c.b)
    }
    function hsvToHex() { return rgbToHex(Qt.hsva(hue / 360.0, saturation, value, 1.0)) }
    function setFromHex(hex, notify) {
        if (!hex || !/^#[0-9a-fA-F]{6}$/.test(hex)) return
        var r = parseInt(hex.slice(1, 3), 16) / 255.0
        var g = parseInt(hex.slice(3, 5), 16) / 255.0
        var b = parseInt(hex.slice(5, 7), 16) / 255.0
        var max = Math.max(r, g, b), min = Math.min(r, g, b), delta = max - min, nextHue = 0
        if (delta > 0) {
            if (max === r) nextHue = 60 * (((g - b) / delta + 6) % 6)
            else if (max === g) nextHue = 60 * ((b - r) / delta + 2)
            else nextHue = 60 * ((r - g) / delta + 4)
        }
        syncing = true
        hue = nextHue
        saturation = max === 0 ? 0 : delta / max
        value = max
        syncing = false
        hueBoard.requestPaint()
        if (notify) publishFromHsv()
    }
    function publishFromHsv() {
        if (!syncing) DemoProperty.colorHex = hsvToHex()
        hueBoard.requestPaint()
    }
    Connections {
        target: DemoProperty
        function onColorHexChanged() { root.setFromHex(DemoProperty.colorHex, false) }
    }

    Rectangle {
        anchors.fill: parent
        color: "#171A21"

        Canvas {
            anchors.fill: parent
            onPaint: {
                var c = getContext("2d")
                var s = 18
                for (var y = 0; y < height; y += s)
                    for (var x = 0; x < width; x += s) {
                        c.fillStyle = ((x / s + y / s) % 2 === 0) ? "#1A202A" : "#202833"
                        c.fillRect(x, y, s, s)
                    }
            }
        }

        Image {
            id: photo
            anchors.fill: parent
            anchors.margins: 20
            fillMode: Image.PreserveAspectFit
            asynchronous: false
            property UIImage binding
            source: binding ? binding.source : ""
            visible: status === Image.Ready
            Component.onCompleted: binding = qmlContext.bindImage(this, DemoName.photo)
            Component.onDestruction: qmlContext.unbind(this, DemoName.photo)
        }

        // The pending picker color drives this overlay immediately. Applying
        // commits it; cancelling returns previewTint to appliedColorHex.
        Rectangle {
            anchors.fill: photo
            visible: photo.visible
            color: root.previewTint
            opacity: 0.32
        }

        Label {
            anchors.centerIn: parent
            visible: !photo.visible
            width: parent.width - 80
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            color: "#B8C2D0"
            font.pixelSize: 16
            text: "Open a preview image from File > Open Preview Image..."
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 46
            color: "#D9171A21"
            Label {
                anchors.left: parent.left
                anchors.leftMargin: 14
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: colorButton.left
                anchors.rightMargin: 14
                elide: Text.ElideRight
                color: "#F2F5FA"
                font.pixelSize: 12
                property UILabel binding
                text: binding ? binding.text : "No image loaded"
                Component.onCompleted: binding = qmlContext.bindLabel(this, DemoName.caption)
                Component.onDestruction: qmlContext.unbind(this, DemoName.caption)
            }
            Button {
                id: colorButton
                anchors.right: parent.right
                anchors.rightMargin: 14
                anchors.verticalCenter: parent.verticalCenter
                text: "Color"
                property UIButton binding
                Component.onCompleted: binding = qmlContext.bindButton(this, DemoName.openPickerButton)
                Component.onDestruction: qmlContext.unbind(this, DemoName.openPickerButton)
                onClicked: binding.clicked()
            }
            Rectangle {
                anchors.right: colorButton.left
                anchors.rightMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                width: 24; height: 24; radius: 3
                color: DemoProperty.appliedColorHex
                border.color: "#FFFFFF"
            }
        }

        Rectangle {
            id: scrim
            anchors.fill: parent
            visible: DemoProperty.pickerVisible
            color: "#B8000000"

            Rectangle {
                id: picker
                anchors.centerIn: parent
                width: Math.min(510, parent.width - 40)
                height: Math.min(510, parent.height - 36)
                radius: 7
                color: "#222833"
                border.color: "#465162"

                Label {
                    anchors.left: parent.left
                    anchors.leftMargin: 18
                    anchors.top: parent.top
                    anchors.topMargin: 14
                    text: "Color Picker"
                    color: "#F2F5FA"
                    font.pixelSize: 18
                    font.bold: true
                }
                Label {
                    anchors.right: parent.right
                    anchors.rightMargin: 18
                    anchors.top: parent.top
                    anchors.topMargin: 18
                    text: "Live preview"
                    color: "#8FE3AD"
                    font.pixelSize: 12
                }

                Rectangle {
                    id: preview
                    anchors.left: parent.left
                    anchors.leftMargin: 18
                    anchors.top: parent.top
                    anchors.topMargin: 52
                    width: 48; height: 48; radius: 4
                    color: root.hsvToHex()
                    border.color: "#FFFFFF"
                }
                TextField {
                    id: hexField
                    anchors.left: preview.right
                    anchors.leftMargin: 12
                    anchors.right: parent.right
                    anchors.rightMargin: 18
                    anchors.verticalCenter: preview.verticalCenter
                    text: DemoProperty.colorHex
                    color: "#F2F5FA"
                    selectByMouse: true
                    onEditingFinished: root.setFromHex(text, true)
                }

                Canvas {
                    id: hueBoard
                    anchors.left: parent.left
                    anchors.leftMargin: 18
                    anchors.right: hueSlider.left
                    anchors.rightMargin: 18
                    anchors.top: preview.bottom
                    anchors.topMargin: 16
                    height: 165
                    onPaint: {
                        var c = getContext("2d")
                        var h = c.createLinearGradient(0, 0, width, 0)
                        h.addColorStop(0, "#FFFFFF")
                        h.addColorStop(1, Qt.hsva(root.hue / 360.0, 1, 1, 1))
                        c.fillStyle = h; c.fillRect(0, 0, width, height)
                        var v = c.createLinearGradient(0, 0, 0, height)
                        v.addColorStop(0, "rgba(0,0,0,0)")
                        v.addColorStop(1, "#000000")
                        c.fillStyle = v; c.fillRect(0, 0, width, height)
                    }
                    MouseArea {
                        anchors.fill: parent
                        function select(mouse) {
                            root.saturation = root.clamp(mouse.x / parent.width, 0, 1)
                            root.value = root.clamp(1 - mouse.y / parent.height, 0, 1)
                            root.publishFromHsv()
                        }
                        onPressed: select(mouse)
                        onPositionChanged: if (pressed) select(mouse)
                    }
                    Rectangle {
                        x: root.saturation * hueBoard.width - width / 2
                        y: (1 - root.value) * hueBoard.height - height / 2
                        width: 14; height: 14; radius: 7
                        color: "transparent"; border.color: "white"; border.width: 2
                    }
                }
                Slider {
                    id: hueSlider
                    anchors.right: parent.right
                    anchors.rightMargin: 9
                    anchors.verticalCenter: hueBoard.verticalCenter
                    width: hueBoard.height
                    rotation: -90
                    from: 0; to: 360; value: root.hue
                    onValueChanged: if (!root.syncing) { root.hue = value; root.publishFromHsv() }
                }

                Label { id: hueLabel; anchors.left: parent.left; anchors.leftMargin: 18; anchors.top: hueBoard.bottom; anchors.topMargin: 13; text: "Hue"; color: "#AEB8C6"; font.pixelSize: 12 }
                SpinBox { anchors.left: hueLabel.right; anchors.leftMargin: 10; anchors.verticalCenter: hueLabel.verticalCenter; from: 0; to: 360; value: Math.round(root.hue); onValueModified: { root.hue = value; root.publishFromHsv() } }
                Label { id: satLabel; anchors.left: parent.left; anchors.leftMargin: 18; anchors.top: hueLabel.bottom; anchors.topMargin: 7; text: "Saturation"; color: "#AEB8C6"; font.pixelSize: 12 }
                SpinBox { anchors.left: satLabel.right; anchors.leftMargin: 10; anchors.verticalCenter: satLabel.verticalCenter; from: 0; to: 100; value: Math.round(root.saturation * 100); onValueModified: { root.saturation = value / 100.0; root.publishFromHsv() } }
                Label { id: valueLabel; anchors.left: parent.left; anchors.leftMargin: 18; anchors.top: satLabel.bottom; anchors.topMargin: 7; text: "Value"; color: "#AEB8C6"; font.pixelSize: 12 }
                SpinBox { anchors.left: valueLabel.right; anchors.leftMargin: 10; anchors.verticalCenter: valueLabel.verticalCenter; from: 0; to: 100; value: Math.round(root.value * 100); onValueModified: { root.value = value / 100.0; root.publishFromHsv() } }

                Row {
                    anchors.left: parent.left
                    anchors.leftMargin: 18
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 16
                    spacing: 8
                    Repeater { model: root.basicColors; delegate: Rectangle { width: 19; height: 19; radius: 3; color: modelData; border.color: "#6E7988"; MouseArea { anchors.fill: parent; onClicked: root.setFromHex(modelData, true) } } }
                }
                Button { anchors.right: cancelButton.left; anchors.rightMargin: 8; anchors.bottom: parent.bottom; anchors.bottomMargin: 12; text: "Apply"; property UIButton binding; Component.onCompleted: binding = qmlContext.bindButton(this, DemoName.applyButton); Component.onDestruction: qmlContext.unbind(this, DemoName.applyButton); onClicked: binding.clicked() }
                Button { id: cancelButton; anchors.right: parent.right; anchors.rightMargin: 18; anchors.bottom: parent.bottom; anchors.bottomMargin: 12; text: "Cancel"; property UIButton binding; Component.onCompleted: binding = qmlContext.bindButton(this, DemoName.cancelButton); Component.onDestruction: qmlContext.unbind(this, DemoName.cancelButton); onClicked: binding.clicked() }
                Button { anchors.right: cancelButton.left; anchors.rightMargin: 124; anchors.bottom: parent.bottom; anchors.bottomMargin: 12; text: "Add"; property UIButton binding; Component.onCompleted: binding = qmlContext.bindButton(this, DemoName.addCustomButton); Component.onDestruction: qmlContext.unbind(this, DemoName.addCustomButton); onClicked: binding.clicked() }
            }
        }

    }
}
