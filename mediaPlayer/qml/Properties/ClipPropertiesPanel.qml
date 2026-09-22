import QtQuick
import QtQuick.Controls

Rectangle
{
    id: root
    property string mediaName: ""
    property string mediaKind: "Unknown"
    property string mediaPath: ""
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
        text: root.enabled ? root.mediaName : "Select an asset"
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

    Column
    {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: divider.bottom
        anchors.margins: 10
        spacing: 10

        Label { text: "Asset information"; color: "#dce2eb"; font.pixelSize: 13; font.bold: true }
        Label { width: parent.width; text: root.enabled ? "Name: " + root.mediaName : "Import an asset to view information."; color: "#9aa6b5"; wrapMode: Text.Wrap }
        Label { width: parent.width; text: root.enabled ? "Type: " + root.mediaKind : ""; color: "#9aa6b5"; visible: root.enabled }
        Label { width: parent.width; text: root.enabled ? "Path: " + root.mediaPath : ""; color: "#7f8996"; elide: Text.ElideMiddle; visible: root.enabled }
        Label { width: parent.width; text: "Duration, dimensions, frame rate, and streams appear after MediaObj loads the asset."; color: "#7f8996"; wrapMode: Text.Wrap }
    }
}
