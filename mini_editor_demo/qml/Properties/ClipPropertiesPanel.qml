import QtQuick
import QtQuick.Controls

Rectangle
{
    id: root
    property string mediaName: ""
    signal resetRequested()
    color: "#20242c"
    clip: true

    Label
    {
        id: title
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 10
        anchors.topMargin: 10
        text: "Properties"
        color: "#f2f5f8"
        font.pixelSize: 14
        font.bold: true
    }
    Label
    {
        id: media
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: title.bottom
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        anchors.topMargin: 8
        text: root.enabled ? root.mediaName : "Select a timeline clip"
        color: root.enabled ? "#8ec7ff" : "#7f8996"
        elide: Text.ElideMiddle
    }
    Rectangle
    {
        id: divider
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: media.bottom
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        anchors.topMargin: 8
        height: 1
        color: "#343a44"
    }

    Flickable
    {
        id: scroller
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: divider.bottom
        anchors.bottom: parent.bottom
        clip: true
        contentWidth: width
        contentHeight: controls.implicitHeight + 20
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar
        {
            policy: scroller.contentHeight > scroller.height
                    ? ScrollBar.AsNeeded : ScrollBar.AlwaysOff
        }

        EffectControls
        {
            id: controls
            x: 10
            y: 10
            width: Math.max(0, scroller.width - 26)
            enabled: root.enabled
            opacity: root.enabled ? 1.0 : 0.45
            onResetRequested: root.resetRequested()
        }
    }
}
