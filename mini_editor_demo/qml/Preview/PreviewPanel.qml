import QtQuick
import QtQuick.Controls

Rectangle
{
    id: root
    property bool hasMedia: false
    property string mediaName: ""
    property string mediaSource: ""
    property bool playing: false
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
        anchors.margins: 18
        anchors.bottomMargin: 10
        color: "#090b0e"
        border.color: "#2d333d"

        Image
        {
            id: image
            anchors.fill: parent
            anchors.margins: 2
            source: root.mediaSource
            fillMode: Image.PreserveAspectFit
            visible: root.hasMedia && status === Image.Ready
            asynchronous: false
        }

        Column
        {
            anchors.centerIn: parent
            spacing: 8
            visible: !image.visible
            Label
            {
                anchors.horizontalCenter: parent.horizontalCenter
                text: root.hasMedia ? "PLAYER SURFACE" : "PREVIEW"
                color: "#708095"
                font.pixelSize: 22
                font.bold: true
            }
            Label
            {
                anchors.horizontalCenter: parent.horizontalCenter
                text: root.hasMedia ? root.mediaName : "Import media, then add it to the timeline"
                color: "#9aa6b5"
                font.pixelSize: 13
            }
            Label
            {
                anchors.horizontalCenter: parent.horizontalCenter
                visible: root.hasMedia && image.status !== Image.Ready
                text: "SIMBA / MediaObj render target"
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
        canPlay: root.hasMedia
        playing: root.playing
        onPlayRequested: root.playRequested()
        onStopRequested: root.stopRequested()
    }
}
