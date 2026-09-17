import QtQuick
import QtQuick.Controls

Item {
    id: root
    visible: false

    property string initialColor: "#3B82F6"
    property string previewColor: initialColor

    signal accepted(string colorHex)
    signal rejected()

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

    function open(colorHex) {
        initialColor = colorHex
        setFromHex(colorHex)
        previewColor = hsvToHex()
        visible = true
    }

    function accept() {
        visible = false
        accepted(previewColor)
    }

    function reject() {
        previewColor = initialColor
        visible = false
        rejected()
    }

    Rectangle {
        anchors.fill: parent
        color: "#AA000000"

        MouseArea { anchors.fill: parent }

        Rectangle {
            id: picker
            anchors.centerIn: parent
            width: 560
            height: 420
            radius: 6
            color: "#242B37"
            border.color: "#4D5A6D"

            Label {
                anchors.left: parent.left
                anchors.leftMargin: 18
                anchors.top: parent.top
                anchors.topMargin: 15
                text: "Color Picker"
                color: "#F4F7FB"
                font.pixelSize: 18
                font.bold: true
            }

            Label {
                anchors.right: parent.right
                anchors.rightMargin: 18
                anchors.top: parent.top
                anchors.topMargin: 19
                text: "Live preview"
                color: "#84D9A4"
                font.pixelSize: 12
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
                height: 170

                onPaint: {
                    var context = getContext("2d")
                    var hueGradient = context.createLinearGradient(0, 0, width, 0)
                    hueGradient.addColorStop(0, "#FFFFFF")
                    hueGradient.addColorStop(1, Qt.hsva(root.hue / 360.0, 1, 1, 1))
                    context.fillStyle = hueGradient
                    context.fillRect(0, 0, width, height)

                    var valueGradient = context.createLinearGradient(0, 0, 0, height)
                    valueGradient.addColorStop(0, "rgba(0,0,0,0)")
                    valueGradient.addColorStop(1, "#000000")
                    context.fillStyle = valueGradient
                    context.fillRect(0, 0, width, height)
                }

                MouseArea {
                    anchors.fill: parent

                    function select(mouse) {
                        root.saturation = root.clamp(mouse.x / parent.width, 0, 1)
                        root.value = root.clamp(1 - mouse.y / parent.height, 0, 1)
                        root.publishPreview()
                    }

                    onPressed: function(mouse) { select(mouse) }
                    onPositionChanged: function(mouse) {
                        if (pressed)
                            select(mouse)
                    }
                }

                Rectangle {
                    x: root.saturation * hueBoard.width - 7
                    y: (1 - root.value) * hueBoard.height - 7
                    width: 14
                    height: 14
                    radius: 7
                    color: "transparent"
                    border.color: "white"
                    border.width: 2
                }
            }

            Rectangle {
                id: hueTrack
                anchors.left: hueBoard.right
                anchors.leftMargin: 18
                anchors.top: hueBoard.top
                width: 10
                height: hueBoard.height
                radius: 5
                color: "#151A22"
                border.color: "#56657A"

                Rectangle {
                    y: (1 - root.hue / 360) * (hueTrack.height - height)
                    x: -4
                    width: 18
                    height: 18
                    radius: 9
                    color: "#F4F7FB"
                }

                MouseArea {
                    anchors.fill: parent

                    function select(mouse) {
                        root.hue = root.clamp((1 - mouse.y / parent.height) * 360, 0, 360)
                        root.publishPreview()
                    }

                    onPressed: function(mouse) { select(mouse) }
                    onPositionChanged: function(mouse) {
                        if (pressed)
                            select(mouse)
                    }
                }
            }

            Column {
                anchors.left: hueTrack.right
                anchors.leftMargin: 28
                anchors.right: parent.right
                anchors.rightMargin: 18
                anchors.top: hueBoard.top
                spacing: 13

                Repeater {
                    model: ["Hue", "Saturation", "Value"]

                    delegate: Row {
                        spacing: 8

                        Label {
                            width: 72
                            anchors.verticalCenter: parent.verticalCenter
                            text: modelData
                            color: "#B5C0D0"
                            font.pixelSize: 12
                        }

                        Rectangle {
                            width: 24
                            height: 24
                            color: "#151A22"
                            border.color: "#56657A"
                            Label { anchors.centerIn: parent; text: "-"; color: "#F4F7FB" }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (index === 0)
                                        root.hue = root.clamp(root.hue - 1, 0, 360)
                                    else if (index === 1)
                                        root.saturation = root.clamp(root.saturation - 0.01, 0, 1)
                                    else
                                        root.value = root.clamp(root.value - 0.01, 0, 1)
                                    root.publishPreview()
                                }
                            }
                        }

                        Label {
                            width: 34
                            anchors.verticalCenter: parent.verticalCenter
                            horizontalAlignment: Text.AlignHCenter
                            text: index === 0 ? Math.round(root.hue)
                                              : (index === 1 ? Math.round(root.saturation * 100)
                                                             : Math.round(root.value * 100))
                            color: "#F4F7FB"
                            font.pixelSize: 12
                        }

                        Rectangle {
                            width: 24
                            height: 24
                            color: "#151A22"
                            border.color: "#56657A"
                            Label { anchors.centerIn: parent; text: "+"; color: "#F4F7FB" }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (index === 0)
                                        root.hue = root.clamp(root.hue + 1, 0, 360)
                                    else if (index === 1)
                                        root.saturation = root.clamp(root.saturation + 0.01, 0, 1)
                                    else
                                        root.value = root.clamp(root.value + 0.01, 0, 1)
                                    root.publishPreview()
                                }
                            }
                        }
                    }
                }
            }

            Row {
                anchors.left: parent.left
                anchors.leftMargin: 18
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 18
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
        }
    }
}
