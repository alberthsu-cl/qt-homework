import QtQuick
import QtQuick.Controls

Rectangle
{
    id: root
    property bool hasMedia: false
    property string mediaName: ""
    property string mediaSource: ""
    signal importRequested()
    color: "#20242c"

    Label
    {
        id: title
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 10
        text: "Media Library"
        color: "#f2f5f8"
        font.pixelSize: 14
        font.bold: true
    }

    Row
    {
        id: buttons
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: title.bottom
        anchors.margins: 10
        spacing: 6

        Button { width: buttons.width; height: 34; text: "Import"; onClicked: root.importRequested() }
    }

    ScrollView
    {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: buttons.bottom
        anchors.bottom: parent.bottom
        anchors.margins: 10
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
