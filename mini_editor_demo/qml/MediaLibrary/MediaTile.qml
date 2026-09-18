import QtQuick
import QtQuick.Controls

Rectangle
{
    id: root
    property string title: ""
    property string source: ""
    property bool selected: false
    signal activated()

    width: 250
    height: 150
    radius: 4
    color: selected ? "#313945" : "#292e37"
    border.color: selected ? "#4aa3ff" : "#3a414c"
    border.width: selected ? 2 : 1

    Image
    {
        id: thumbnail
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 7
        height: 104
        source: root.source
        fillMode: Image.PreserveAspectCrop
        asynchronous: false
        clip: true
    }

    Rectangle
    {
        anchors.fill: thumbnail
        visible: thumbnail.status !== Image.Ready
        color: "#171a20"
        Label
        {
            anchors.centerIn: parent
            text: "MEDIA"
            color: "#788391"
            font.bold: true
        }
    }

    Label
    {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: 9
        anchors.rightMargin: 9
        anchors.bottomMargin: 9
        text: root.title
        color: "#eef2f7"
        elide: Text.ElideMiddle
        font.pixelSize: 12
    }

    MouseArea
    {
        anchors.fill: parent
        onClicked: root.activated()
    }
}
