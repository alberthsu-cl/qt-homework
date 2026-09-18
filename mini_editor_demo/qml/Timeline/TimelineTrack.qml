import QtQuick
import QtQuick.Controls

Rectangle
{
    id: root
    property string trackName: "V1"
    property bool hasClip: false
    property string mediaName: ""
    property int splitCount: 0

    color: "#1d2128"
    border.color: "#292f38"

    Rectangle
    {
        id: header
        width: 76
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        color: "#252a33"
        Label
        {
            anchors.centerIn: parent
            text: root.trackName
            color: "#d9dfe7"
            font.bold: true
        }
    }

    TimelineClip
    {
        visible: root.hasClip
        x: header.width + 18
        y: 9
        width: Math.max(240, root.width * 0.58)
        height: root.height - 18
        title: root.mediaName
        splitCount: root.splitCount
    }
}
