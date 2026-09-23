import QtQuick
import QtQuick.Controls

Rectangle
{
    id: root
    property bool playing: false
    property bool canPlay: false
    property int positionMs: 0
    property int durationMs: 0
    signal playRequested()
    signal stopRequested()
    color: "#242932"
    height: 46

    function formatTime(milliseconds)
    {
        var totalSeconds = Math.max(0, Math.floor(milliseconds / 1000))
        var minutes = Math.floor(totalSeconds / 60)
        var seconds = totalSeconds % 60
        return (minutes < 10 ? "0" : "") + minutes + ":" +
               (seconds < 10 ? "0" : "") + seconds
    }

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
        width: 112
        text: root.formatTime(root.positionMs) + " / " +
              root.formatTime(root.durationMs)
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
        to: Math.max(1, root.durationMs)
        value: root.positionMs
        enabled: false
    }
}
