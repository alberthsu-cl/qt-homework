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
    property color previewTint: DemoProperty.pickerVisible
                                ? DemoProperty.colorHex
                                : DemoProperty.appliedColorHex

    Component.onCompleted: binding = qmlContext.bindWindow(this, DemoName.window)
    Component.onDestruction: qmlContext.unbind(this, DemoName.window)

    function openPicker() {
        openEvent.binding.clicked()
    }

    Rectangle {
        anchors.fill: parent
        color: "#12161D"

        Canvas {
            anchors.fill: parent
            onPaint: {
                var context = getContext("2d")
                var squareSize = 18
                for (var y = 0; y < height; y += squareSize) {
                    for (var x = 0; x < width; x += squareSize) {
                        context.fillStyle = ((x / squareSize + y / squareSize) % 2 === 0)
                                ? "#1A202A" : "#202833"
                        context.fillRect(x, y, squareSize, squareSize)
                    }
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

        Rectangle {
            anchors.fill: photo
            visible: photo.visible
            color: root.previewTint
            opacity: 0.32
        }

        Label {
            anchors.centerIn: parent
            visible: !photo.visible
            text: "Open a preview image from File > Open Preview Image..."
            color: "#B8C2D0"
            font.pixelSize: 16
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 44
            color: "#E812161D"

            Label {
                anchors.left: parent.left
                anchors.leftMargin: 14
                anchors.right: colorButton.left
                anchors.rightMargin: 52
                anchors.verticalCenter: parent.verticalCenter
                elide: Text.ElideRight
                color: "#EAF0F8"
                font.pixelSize: 12
                property UILabel binding
                text: binding ? binding.text : "No image loaded"
                Component.onCompleted: binding = qmlContext.bindLabel(this, DemoName.caption)
                Component.onDestruction: qmlContext.unbind(this, DemoName.caption)
            }

            Rectangle {
                anchors.right: colorButton.left
                anchors.rightMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                width: 22
                height: 22
                radius: 3
                color: DemoProperty.appliedColorHex
                border.color: "#FFFFFF"
            }

            Rectangle {
                id: colorButton
                anchors.right: parent.right
                anchors.rightMargin: 14
                anchors.verticalCenter: parent.verticalCenter
                width: 70
                height: 28
                radius: 3
                color: "#313B4B"
                border.color: "#5B6A80"
                Label { anchors.centerIn: parent; text: "Color"; color: "#F2F5FA"; font.pixelSize: 12 }
                MouseArea { anchors.fill: parent; onClicked: root.openPicker() }
            }
        }

        Button {
            id: openEvent
            visible: false
            property UIButton binding
            Component.onCompleted: binding = qmlContext.bindButton(this, DemoName.openPickerButton)
            Component.onDestruction: qmlContext.unbind(this, DemoName.openPickerButton)
        }

    }
}
