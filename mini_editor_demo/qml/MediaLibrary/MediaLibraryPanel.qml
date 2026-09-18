import QtQuick
import QtQuick.Controls

Rectangle
{
    id: root
    property bool hasMedia: false
    property string mediaName: ""
    property string mediaSource: ""
    signal importRequested()
    signal addRequested()
    color: "#20242c"

    Label
    {
        id: title
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 14
        text: "Media Library"
        color: "#f2f5f8"
        font.pixelSize: 16
        font.bold: true
    }

    Row
    {
        id: buttons
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: title.bottom
        anchors.margins: 14
        spacing: 8

        Button { width: (buttons.width - 8) / 2; text: "Import"; onClicked: root.importRequested() }
        Button { width: (buttons.width - 8) / 2; text: "Add to Timeline"; enabled: root.hasMedia; onClicked: root.addRequested() }
    }

    ScrollView
    {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: buttons.bottom
        anchors.bottom: parent.bottom
        anchors.margins: 14
        clip: true

        Column
        {
            width: parent.width
            spacing: 10
            MediaTile
            {
                width: parent.width
                visible: root.hasMedia
                title: root.mediaName
                source: root.mediaSource
                selected: true
            }
            Label
            {
                width: parent.width
                visible: !root.hasMedia
                topPadding: 60
                text: "No media imported\n\nUse Import to add a video,\naudio file, or image."
                color: "#7f8996"
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }
        }
    }
}
