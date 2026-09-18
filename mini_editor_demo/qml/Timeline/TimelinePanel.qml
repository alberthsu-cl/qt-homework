import QtQuick
import QtQuick.Controls

Rectangle
{
    id: root
    implicitHeight: 225
    property bool hasClip: false
    property string mediaName: ""
    property bool playing: false
    property int splitCount: 0
    property real playheadPosition: 0.31
    signal splitRequested()
    color: "#171a20"
    clip: true

    Timer
    {
        interval: 80
        repeat: true
        running: root.playing
        onTriggered:
        {
            root.playheadPosition += 0.004
            if (root.playheadPosition > 0.9)
                root.playheadPosition = 0.1
        }
    }

    Item
    {
        id: toolbar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 12
        height: 38

        Label { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "Timeline"; color: "#eef2f7"; font.pixelSize: 15; font.bold: true }
        Button { anchors.left: parent.left; anchors.leftMargin: 78; anchors.verticalCenter: parent.verticalCenter; text: "Split"; enabled: root.hasClip; onClicked: root.splitRequested() }
        Label { id: zoomText; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; text: "100%"; color: "#dce1e8" }
        Slider { anchors.right: zoomText.left; anchors.rightMargin: 10; anchors.verticalCenter: parent.verticalCenter; width: 180; from: 25; to: 200; value: 100 }
    }

    Item
    {
        id: ruler
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: toolbar.bottom
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        height: 28
        Repeater
        {
            model: 9
            Label
            {
                x: 76 + index * (ruler.width - 96) / 8
                y: 5
                text: "00:" + (index * 2).toString().padStart(2, "0")
                color: "#9ea8b5"
                font.pixelSize: 11
            }
        }
    }

    Item
    {
        id: tracks
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: ruler.bottom
        anchors.bottom: parent.bottom
        anchors.margins: 12
        anchors.topMargin: 0

        Column
        {
            anchors.fill: parent
            spacing: 5
            TimelineTrack { width: parent.width; height: 72; trackName: "V1"; hasClip: root.hasClip; mediaName: root.mediaName; splitCount: root.splitCount }
            TimelineTrack { width: parent.width; height: 58; trackName: "A1"; hasClip: false; mediaName: ""; splitCount: 0 }
        }

        Rectangle
        {
            visible: root.hasClip
            x: 76 + root.playheadPosition * (parent.width - 76)
            y: -28
            width: 2
            height: parent.height + 28
            color: "#ff4b55"
            z: 10
            Rectangle { anchors.horizontalCenter: parent.horizontalCenter; anchors.top: parent.top; width: 10; height: 10; rotation: 45; color: "#ff4b55" }
        }
    }
}
