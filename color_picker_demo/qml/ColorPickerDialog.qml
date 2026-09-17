import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtKit

Window {
    id: root
    visible: true
    width: 560
    height: 420
    minimumWidth: 560
    maximumWidth: 560
    minimumHeight: 420
    maximumHeight: 420
    flags: Qt.Dialog | Qt.FramelessWindowHint
    modality: Qt.NonModal
    color: "transparent"
    title: "Color Picker"

    property UIWindow binding

    ColorPickerProperty {
        id: pickerProperty
        onAppliedColorHexChanged: root.initialColor = appliedColorHex
        onColorHexChanged: root.synchronizeFromProperty()
    }

    property string initialColor: "#3B82F6"
    property string previewColor: initialColor
    property string colorMode: "HSV"
    property bool synchronizingProperty: false
    property var customColors: pickerProperty.customColors === ""
                               ? [] : pickerProperty.customColors.split(",")

    property real hue: 215
    property real saturation: 0.91
    property real value: 0.96
    property bool syncing: false
    property var basicColors: [
        "#F44336", "#FF9800", "#FFEB3B", "#4CAF50",
        "#00BCD4", "#2196F3", "#3F51B5", "#9C27B0",
        "#E91E63", "#FFFFFF", "#808080", "#000000"
    ]

    function clamp(number, low, high) {
        return Math.max(low, Math.min(high, number))
    }

    function rgbToHex(colorValue) {
        function component(value) {
            var hex = Math.round(value * 255).toString(16)
            return hex.length === 1 ? "0" + hex : hex
        }
        return "#" + component(colorValue.r) + component(colorValue.g) + component(colorValue.b)
    }

    function hsvToHex() {
        return rgbToHex(Qt.hsva(hue / 360.0, saturation, value, 1.0))
    }

    function rgbChannel(index) {
        return parseInt(previewColor.slice(1 + index * 2, 3 + index * 2), 16)
    }

    function byteToHex(value) {
        var hex = clamp(Math.round(value), 0, 255).toString(16)
        return hex.length === 1 ? "0" + hex : hex
    }

    function setRgbChannel(index, channelValue) {
        var channels = [rgbChannel(0), rgbChannel(1), rgbChannel(2)]
        channels[index] = clamp(channelValue, 0, 255)
        var colorHex = "#" + byteToHex(channels[0]) + byteToHex(channels[1]) + byteToHex(channels[2])
        setFromHex(colorHex)
        previewColor = colorHex
    }

    function channelValue(index) {
        if (colorMode === "RGB")
            return rgbChannel(index)
        if (index === 0)
            return Math.round(hue)
        return index === 1 ? Math.round(saturation * 100) : Math.round(value * 100)
    }

    function channelMaximum(index) {
        if (colorMode === "RGB")
            return 255
        return index === 0 ? 360 : 100
    }

    function setChannelValue(index, channelValue) {
        if (colorMode === "RGB") {
            setRgbChannel(index, channelValue)
            return
        }

        if (index === 0)
            hue = clamp(channelValue, 0, 360)
        else if (index === 1)
            saturation = clamp(channelValue / 100.0, 0, 1)
        else
            value = clamp(channelValue / 100.0, 0, 1)
        publishPreview()
    }

    function setFromHex(colorHex) {
        if (!colorHex || !/^#[0-9a-fA-F]{6}$/.test(colorHex))
            return

        var red = parseInt(colorHex.slice(1, 3), 16) / 255.0
        var green = parseInt(colorHex.slice(3, 5), 16) / 255.0
        var blue = parseInt(colorHex.slice(5, 7), 16) / 255.0
        var maximum = Math.max(red, green, blue)
        var minimum = Math.min(red, green, blue)
        var delta = maximum - minimum
        var newHue = 0

        if (delta > 0) {
            if (maximum === red)
                newHue = 60 * (((green - blue) / delta + 6) % 6)
            else if (maximum === green)
                newHue = 60 * ((blue - red) / delta + 2)
            else
                newHue = 60 * ((red - green) / delta + 4)
        }

        syncing = true
        hue = newHue
        saturation = maximum === 0 ? 0 : delta / maximum
        value = maximum
        syncing = false
        hueBoard.requestPaint()
    }

    function publishPreview() {
        if (!syncing)
            previewColor = hsvToHex()
        hueBoard.requestPaint()
    }

    function accept() {
        pickerProperty.colorHex = previewColor
        applyEvent.binding.clicked()
    }

    function reject() {
        previewColor = initialColor
        cancelEvent.binding.clicked()
    }

    onPreviewColorChanged: {
        if (visible && !synchronizingProperty)
            pickerProperty.colorHex = previewColor
    }

    function synchronizeFromProperty() {
        if (pickerProperty.colorHex.toUpperCase() === previewColor.toUpperCase())
            return
        synchronizingProperty = true
        setFromHex(pickerProperty.colorHex)
        previewColor = hsvToHex()
        synchronizingProperty = false
    }

    onClosing: function(close) {
        close.accepted = false
        root.reject()
    }

    Component.onCompleted: {
        binding = qmlContext.bindWindow(this, DemoName.pickerWindow)
        initialColor = pickerProperty.appliedColorHex
        synchronizeFromProperty()
    }

    Component.onDestruction: qmlContext.unbind(this, DemoName.pickerWindow)

    Rectangle {
        anchors.fill: parent
        color: "transparent"

        Rectangle {
            id: picker
            anchors.fill: parent
            radius: 6
            color: "#242B37"
            border.color: "#4D5A6D"

            Rectangle {
                id: titleBar
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: 44
                radius: 6
                color: "#2D3543"

                MouseArea {
                    anchors.fill: parent
                    onPressed: root.startSystemMove()
                }

                Label {
                    anchors.left: parent.left
                    anchors.leftMargin: 18
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Color Picker"
                    color: "#F4F7FB"
                    font.pixelSize: 18
                    font.bold: true
                }

                Label {
                    anchors.right: closeButton.left
                    anchors.rightMargin: 14
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Live preview"
                    color: "#84D9A4"
                    font.pixelSize: 12
                }

                Rectangle {
                    id: closeButton
                    anchors.right: parent.right
                    anchors.rightMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    width: 26
                    height: 26
                    radius: 3
                    color: closeMouse.containsMouse ? "#465164" : "transparent"
                    Label { anchors.centerIn: parent; text: "X"; color: "#F4F7FB"; font.pixelSize: 12 }
                    MouseArea {
                        id: closeMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: root.reject()
                    }
                }
            }

            Rectangle {
                id: swatch
                anchors.left: parent.left
                anchors.leftMargin: 18
                anchors.top: parent.top
                anchors.topMargin: 52
                width: 44
                height: 44
                radius: 4
                color: root.previewColor
                border.color: "#F4F7FB"
            }

            TextField {
                id: hexField
                anchors.left: swatch.right
                anchors.leftMargin: 12
                anchors.right: parent.right
                anchors.rightMargin: 18
                anchors.verticalCenter: swatch.verticalCenter
                height: 30
                text: root.previewColor
                color: "#F4F7FB"
                selectByMouse: true
                background: Rectangle {
                    radius: 3
                    color: "#151A22"
                    border.color: "#56657A"
                }
                onEditingFinished: {
                    root.setFromHex(text)
                    root.publishPreview()
                }
            }

            Canvas {
                id: hueBoard
                anchors.left: parent.left
                anchors.leftMargin: 18
                anchors.top: swatch.bottom
                anchors.topMargin: 18
                width: 260
                height: 150

                onPaint: {
                    var context = getContext("2d")
                    var hueGradient = context.createLinearGradient(0, 0, width, 0)
                    hueGradient.addColorStop(0.0, "#FF0000")
                    hueGradient.addColorStop(1.0 / 6.0, "#FFFF00")
                    hueGradient.addColorStop(2.0 / 6.0, "#00FF00")
                    hueGradient.addColorStop(3.0 / 6.0, "#00FFFF")
                    hueGradient.addColorStop(4.0 / 6.0, "#0000FF")
                    hueGradient.addColorStop(5.0 / 6.0, "#FF00FF")
                    hueGradient.addColorStop(1.0, "#FF0000")
                    context.fillStyle = hueGradient
                    context.fillRect(0, 0, width, height)

                    var saturationGradient = context.createLinearGradient(0, 0, 0, height)
                    saturationGradient.addColorStop(0, "rgba(255,255,255,0)")
                    saturationGradient.addColorStop(1, "#FFFFFF")
                    context.fillStyle = saturationGradient
                    context.fillRect(0, 0, width, height)

                    context.fillStyle = "rgba(0,0,0," + (1 - root.value) + ")"
                    context.fillRect(0, 0, width, height)
                }

                MouseArea {
                    anchors.fill: parent

                    function select(mouse) {
                        root.hue = root.clamp(mouse.x / parent.width * 360, 0, 360)
                        root.saturation = root.clamp(1 - mouse.y / parent.height, 0, 1)
                        root.publishPreview()
                    }

                    onPressed: function(mouse) { select(mouse) }
                    onPositionChanged: function(mouse) {
                        if (pressed)
                            select(mouse)
                    }
                }

                Rectangle {
                    x: root.hue / 360 * hueBoard.width - 7
                    y: (1 - root.saturation) * hueBoard.height - 7
                    width: 14
                    height: 14
                    radius: 7
                    color: "transparent"
                    border.color: "white"
                    border.width: 2
                }
            }

            Rectangle {
                id: valueTrack
                anchors.left: hueBoard.right
                anchors.leftMargin: 18
                anchors.top: hueBoard.top
                width: 14
                height: hueBoard.height
                radius: 5
                border.color: "#56657A"
                gradient: Gradient {
                    orientation: Gradient.Vertical
                    GradientStop { position: 0.0; color: Qt.hsva(root.hue / 360.0, root.saturation, 1, 1) }
                    GradientStop { position: 1.0; color: "#000000" }
                }

                Rectangle {
                    y: (1 - root.value) * (valueTrack.height - height)
                    x: -4
                    width: 22
                    height: 18
                    radius: 9
                    color: "#F4F7FB"
                }

                MouseArea {
                    anchors.fill: parent

                    function select(mouse) {
                        root.value = root.clamp(1 - mouse.y / parent.height, 0, 1)
                        root.publishPreview()
                    }

                    onPressed: function(mouse) { select(mouse) }
                    onPositionChanged: function(mouse) {
                        if (pressed)
                            select(mouse)
                    }
                }
            }

            ComboBox {
                id: formatSelector
                anchors.left: valueTrack.right
                anchors.leftMargin: 28
                anchors.right: parent.right
                anchors.rightMargin: 18
                anchors.top: hueBoard.top
                height: 30
                model: ["HSV", "RGB"]
                currentIndex: root.colorMode === "HSV" ? 0 : 1
                onActivated: root.colorMode = currentText
            }

            Column {
                anchors.left: valueTrack.right
                anchors.leftMargin: 28
                anchors.right: parent.right
                anchors.rightMargin: 18
                anchors.top: formatSelector.bottom
                anchors.topMargin: 10
                spacing: 10

                Repeater {
                    model: root.colorMode === "HSV"
                           ? ["Hue", "Saturation", "Value"]
                           : ["Red", "Green", "Blue"]

                    delegate: Row {
                        spacing: 8

                        Label {
                            width: 72
                            anchors.verticalCenter: parent.verticalCenter
                            text: modelData
                            color: "#B5C0D0"
                            font.pixelSize: 12
                        }

                        SpinBox {
                            id: channelSpinBox
                            width: 110
                            height: 28
                            editable: true
                            from: 0
                            to: root.channelMaximum(index)
                            value: root.channelValue(index)
                            onValueModified: root.setChannelValue(index, value)

                            Connections {
                                target: root
                                function onPreviewColorChanged() {
                                    channelSpinBox.value = root.channelValue(index)
                                }
                            }
                        }
                    }
                }
            }

            Label {
                id: basicLabel
                anchors.left: parent.left
                anchors.leftMargin: 18
                anchors.top: hueBoard.bottom
                anchors.topMargin: 10
                text: "Basic colors"
                color: "#B5C0D0"
                font.pixelSize: 11
            }

            Row {
                id: basicColorRow
                anchors.left: parent.left
                anchors.leftMargin: 18
                anchors.top: basicLabel.bottom
                anchors.topMargin: 3
                spacing: 7

                Repeater {
                    model: root.basicColors
                    delegate: Rectangle {
                        width: 18
                        height: 18
                        radius: 3
                        color: modelData
                        border.color: "#718098"
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                root.setFromHex(modelData)
                                root.publishPreview()
                            }
                        }
                    }
                }
            }

            Label {
                id: customLabel
                anchors.left: parent.left
                anchors.leftMargin: 18
                anchors.top: basicColorRow.bottom
                anchors.topMargin: 6
                text: "Custom colors"
                color: "#B5C0D0"
                font.pixelSize: 11
            }

            Row {
                anchors.left: parent.left
                anchors.leftMargin: 18
                anchors.top: customLabel.bottom
                anchors.topMargin: 3
                spacing: 7

                Repeater {
                    model: root.customColors
                    delegate: Rectangle {
                        width: 18
                        height: 18
                        radius: 3
                        color: modelData
                        border.color: "#718098"
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                root.setFromHex(modelData)
                                root.publishPreview()
                            }
                        }
                    }
                }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.leftMargin: 18
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 14
                width: 86
                height: 28
                radius: 3
                color: "#343E4D"
                border.color: "#5B6A80"
                Label { anchors.centerIn: parent; text: "Add custom"; color: "#F4F7FB"; font.pixelSize: 11 }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        pickerProperty.colorHex = root.previewColor
                        addCustomEvent.binding.clicked()
                    }
                }
            }

            Rectangle {
                anchors.right: cancelButton.left
                anchors.rightMargin: 8
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 14
                width: 86
                height: 28
                radius: 3
                color: "#3D76CE"
                Label { anchors.centerIn: parent; text: "Apply"; color: "white"; font.pixelSize: 12 }
                MouseArea { anchors.fill: parent; onClicked: root.accept() }
            }

            Rectangle {
                id: cancelButton
                anchors.right: parent.right
                anchors.rightMargin: 18
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 14
                width: 86
                height: 28
                radius: 3
                color: "#343E4D"
                border.color: "#5B6A80"
                Label { anchors.centerIn: parent; text: "Cancel"; color: "#F4F7FB"; font.pixelSize: 12 }
                MouseArea { anchors.fill: parent; onClicked: root.reject() }
            }

            Button {
                id: addCustomEvent
                visible: false
                property UIButton binding
                Component.onCompleted: binding = qmlContext.bindButton(this, DemoName.addCustomButton)
                Component.onDestruction: qmlContext.unbind(this, DemoName.addCustomButton)
            }

            Button {
                id: applyEvent
                visible: false
                property UIButton binding
                Component.onCompleted: binding = qmlContext.bindButton(this, DemoName.applyButton)
                Component.onDestruction: qmlContext.unbind(this, DemoName.applyButton)
            }

            Button {
                id: cancelEvent
                visible: false
                property UIButton binding
                Component.onCompleted: binding = qmlContext.bindButton(this, DemoName.cancelButton)
                Component.onDestruction: qmlContext.unbind(this, DemoName.cancelButton)
            }
        }
    }
}
