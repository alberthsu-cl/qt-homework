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
    height: 46

    Button
    {
        id: play
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        width: 70
        height: 32
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
        width: 62
        height: 32
        text: "Stop"
        enabled: root.canPlay
        onClicked: root.stopRequested()
    }
    Label
    {
        id: timecode
        anchors.right: parent.right
        anchors.rightMargin: 10
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
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        height: 30
        anchors.verticalCenter: parent.verticalCenter
        from: 0
        to: 100
        value: 0
        enabled: root.canPlay
    }
}
