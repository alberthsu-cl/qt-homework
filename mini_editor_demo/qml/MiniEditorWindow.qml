import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtKit
import "MediaLibrary" as MediaLibrary
import "Preview" as Preview
import "Timeline" as Timeline
import "Properties" as Properties

Window
{
    id: root
    visible: true
    flags: Qt.FramelessWindowHint
    color: "#171a20"
    title: "Mini Editor"

    property UIWindow binding
    Component.onCompleted: binding = qmlContext.bindWindow(this, MiniEditorName.window)
    Component.onDestruction: qmlContext.unbind(this, MiniEditorName.window)

    Image
    {
        id: previewBridge
        visible: false
        property UIImage binding
        source: binding ? binding.source : ""
        Component.onCompleted: binding = qmlContext.bindImage(this, MiniEditorName.previewImage)
        Component.onDestruction: qmlContext.unbind(this, MiniEditorName.previewImage)
    }

    Button
    {
        id: importBridge
        visible: false
        property UIButton binding
        Component.onCompleted: binding = qmlContext.bindButton(this, MiniEditorName.importButton)
        Component.onDestruction: qmlContext.unbind(this, MiniEditorName.importButton)
    }
    Button
    {
        id: addBridge
        visible: false
        property UIButton binding
        Component.onCompleted: binding = qmlContext.bindButton(this, MiniEditorName.addButton)
        Component.onDestruction: qmlContext.unbind(this, MiniEditorName.addButton)
    }
    Button
    {
        id: playBridge
        visible: false
        property UIButton binding
        Component.onCompleted: binding = qmlContext.bindButton(this, MiniEditorName.playButton)
        Component.onDestruction: qmlContext.unbind(this, MiniEditorName.playButton)
    }
    Button
    {
        id: stopBridge
        visible: false
        property UIButton binding
        Component.onCompleted: binding = qmlContext.bindButton(this, MiniEditorName.stopButton)
        Component.onDestruction: qmlContext.unbind(this, MiniEditorName.stopButton)
    }
    Button
    {
        id: splitBridge
        visible: false
        property UIButton binding
        Component.onCompleted: binding = qmlContext.bindButton(this, MiniEditorName.splitButton)
        Component.onDestruction: qmlContext.unbind(this, MiniEditorName.splitButton)
    }
    Button
    {
        id: resetBridge
        visible: false
        property UIButton binding
        Component.onCompleted: binding = qmlContext.bindButton(this, MiniEditorName.resetButton)
        Component.onDestruction: qmlContext.unbind(this, MiniEditorName.resetButton)
    }

    Rectangle
    {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 42
        color: "#20242c"

        Label
        {
            id: statusLabel
            anchors.left: parent.left
            anchors.leftMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            text: "Import media to begin the first milestone workflow"
            color: "#dce2eb"
            font.pixelSize: 13
            property UILabel binding
            Component.onCompleted: binding = qmlContext.bindLabel(this, MiniEditorName.statusLabel)
            Component.onDestruction: qmlContext.unbind(this, MiniEditorName.statusLabel)
        }

        Label
        {
            anchors.right: parent.right
            anchors.rightMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            text: "QtKit / QML prototype"
            color: "#7e8998"
            font.pixelSize: 12
        }
    }

    Timeline.TimelinePanel
    {
        id: timeline
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: implicitHeight
        hasClip: MiniEditorProperty.hasTimelineClip
        mediaName: MiniEditorProperty.mediaName
        playing: MiniEditorProperty.playing
        splitCount: MiniEditorProperty.splitCount
        onSplitRequested: if (splitBridge.binding) splitBridge.binding.clicked()
    }

    Item
    {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: timeline.top

        MediaLibrary.MediaLibraryPanel
        {
            id: library
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 300
            hasMedia: MiniEditorProperty.hasMedia
            mediaName: MiniEditorProperty.mediaName
            mediaSource: previewBridge.source
            onImportRequested: if (importBridge.binding) importBridge.binding.clicked()
            onAddRequested: if (addBridge.binding) addBridge.binding.clicked()
        }

        Properties.ClipPropertiesPanel
        {
            id: properties
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 290
            enabled: MiniEditorProperty.hasTimelineClip
            mediaName: MiniEditorProperty.mediaName
            onResetRequested: if (resetBridge.binding) resetBridge.binding.clicked()
        }

        Preview.PreviewPanel
        {
            anchors.left: library.right
            anchors.right: properties.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.leftMargin: 1
            anchors.rightMargin: 1
            hasMedia: MiniEditorProperty.hasMedia
            mediaName: MiniEditorProperty.mediaName
            mediaSource: previewBridge.source
            playing: MiniEditorProperty.playing
            onPlayRequested: if (playBridge.binding) playBridge.binding.clicked()
            onStopRequested: if (stopBridge.binding) stopBridge.binding.clicked()
        }
    }
}
