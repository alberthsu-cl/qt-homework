import QtQuick
import QtQuick.Controls
import QtKit

Rectangle
{
    id: root
    property bool hasMedia: false
    property string mediaName: ""
    property string mediaKind: "Unknown"
    property string mediaSource: ""
    property string previewHostBindingName: ""
    property string previewAreaBindingName: ""
    property bool playing: false
    property bool canPlay: false
    signal playRequested()
    signal stopRequested()

    color: "#111419"

    Rectangle
    {
        id: viewport
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: controls.top
        anchors.margins: 10
        anchors.bottomMargin: 6
        color: "#090b0e"
        border.color: "#2d333d"

        Item
        {
            id: previewArea
            anchors.fill: parent
            anchors.margins: 2
            visible: root.hasMedia && root.mediaKind === "Video"
            property UIItem binding
            Component.onCompleted: binding = qmlContext.bindItem(
                this, root.previewAreaBindingName)
            Component.onDestruction: qmlContext.unbind(
                this, root.previewAreaBindingName)

            WindowHost
            {
                anchors.fill: parent
                property WindowHost binding
                Component.onCompleted: binding = qmlContext.bindWindowHost(
                    this, root.previewHostBindingName)
                Component.onDestruction: qmlContext.unbind(
                    this, root.previewHostBindingName)
            }
        }

        Image
        {
            id: image
            anchors.fill: parent
            anchors.margins: 2
            source: root.mediaSource
            fillMode: Image.PreserveAspectFit
            visible: root.hasMedia && root.mediaKind === "Image" &&
                     status === Image.Ready
            asynchronous: false
        }

        Column
        {
            anchors.centerIn: parent
            spacing: 8
            visible: !image.visible && !previewArea.visible
            Label
            {
                anchors.horizontalCenter: parent.horizontalCenter
                text: root.hasMedia ? "SOURCE PLAYER" : "PREVIEW"
                color: "#708095"
                font.pixelSize: 22
                font.bold: true
            }
            Label
            {
                anchors.horizontalCenter: parent.horizontalCenter
                text: root.hasMedia ? root.mediaName : "Import media to preview one source"
                color: "#9aa6b5"
                font.pixelSize: 13
            }
            Label
            {
                anchors.horizontalCenter: parent.horizontalCenter
                visible: root.hasMedia && image.status !== Image.Ready
                text: "MediaObj source preview"
                color: "#586575"
                font.pixelSize: 11
            }
        }

        Rectangle
        {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.margins: 10
            width: 68
            height: 24
            radius: 3
            visible: root.playing
            color: "#d84848"
            Label { anchors.centerIn: parent; text: "PLAYING"; color: "white"; font.pixelSize: 10; font.bold: true }
        }
    }

    PlaybackControls
    {
        id: controls
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        canPlay: root.canPlay
        playing: root.playing
        onPlayRequested: root.playRequested()
        onStopRequested: root.stopRequested()
    }
}
