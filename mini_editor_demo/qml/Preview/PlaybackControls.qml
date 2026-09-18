import QtQuick
import QtQuick.Controls

Rectangle
{
    id: root
    property bool playing: false
    property bool canPlay: false
    signal playRequested()
    signal stopRequested()
    color: "#242932"
    height: 58

    Button
    {
        id: play
        anchors.left: parent.left
        anchors.leftMargin: 14
        anchors.verticalCenter: parent.verticalCenter
        width: 82
        text: root.playing ? "Pause" : "Play"
        enabled: root.canPlay
        onClicked: root.playRequested()
    }
    Button
    {
        id: stop
        anchors.left: play.right
        anchors.leftMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        width: 72
        text: "Stop"
        enabled: root.canPlay
        onClicked: root.stopRequested()
    }
    Label
    {
        id: timecode
        anchors.right: parent.right
        anchors.rightMargin: 14
        anchors.verticalCenter: parent.verticalCenter
        width: 92
        text: root.playing ? "00:00:05:12" : "00:00:00:00"
        color: "#e4e9ef"
        font.family: "Consolas"
    }
    Slider
    {
        anchors.left: stop.right
        anchors.right: timecode.left
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        from: 0
        to: 100
        value: 0
        enabled: root.canPlay
    }
}
