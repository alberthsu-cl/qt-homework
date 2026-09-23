import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtKit
import "MediaLibrary" as MediaLibrary
import "Preview" as Preview
import "Properties" as Properties

Window
{
    id: root
    visible: true
    flags: Qt.FramelessWindowHint
    color: "#171a20"
    title: "Media Player"

    property UIWindow binding
    Component.onCompleted: binding = qmlContext.bindWindow(this, MediaPlayerName.window)
    Component.onDestruction: qmlContext.unbind(this, MediaPlayerName.window)

    Image
    {
        id: previewBridge
        visible: false
        property UIImage binding
        source: binding ? binding.source : ""
        Component.onCompleted: binding = qmlContext.bindImage(this, MediaPlayerName.previewImage)
        Component.onDestruction: qmlContext.unbind(this, MediaPlayerName.previewImage)
    }

    Button
    {
        id: importBridge
        visible: false
        property UIButton binding
        Component.onCompleted: binding = qmlContext.bindButton(this, MediaPlayerName.importButton)
        Component.onDestruction: qmlContext.unbind(this, MediaPlayerName.importButton)
    }
    Button
    {
        id: playBridge
        visible: false
        property UIButton binding
        Component.onCompleted: binding = qmlContext.bindButton(this, MediaPlayerName.playButton)
        Component.onDestruction: qmlContext.unbind(this, MediaPlayerName.playButton)
    }
    Button
    {
        id: stopBridge
        visible: false
        property UIButton binding
        Component.onCompleted: binding = qmlContext.bindButton(this, MediaPlayerName.stopButton)
        Component.onDestruction: qmlContext.unbind(this, MediaPlayerName.stopButton)
    }

    Rectangle
    {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 36
        color: "#20242c"

        Label
        {
            id: statusLabel
            anchors.left: parent.left
            anchors.leftMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            text: "Import a source asset to begin playback"
            color: "#dce2eb"
            font.pixelSize: 13
            property UILabel binding
            Component.onCompleted: binding = qmlContext.bindLabel(this, MediaPlayerName.statusLabel)
            Component.onDestruction: qmlContext.unbind(this, MediaPlayerName.statusLabel)
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

    Item
    {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: parent.bottom

        MediaLibrary.MediaLibraryPanel
        {
            id: library
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 260
            mediaCatalog: {
                try {
                    return JSON.parse(MediaPlayerProperty.mediaCatalogJson)
                } catch (error) {
                    return []
                }
            }
            selectedMediaId: MediaPlayerProperty.mediaId
            onImportRequested: if (importBridge.binding) importBridge.binding.clicked()
            onMediaSelected: MediaPlayerProperty.requestedMediaIndex = mediaIndex
        }

        Properties.ClipPropertiesPanel
        {
            id: properties
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 250
            enabled: MediaPlayerProperty.hasMedia
            mediaName: MediaPlayerProperty.mediaName
            mediaKind: MediaPlayerProperty.mediaKind
            mediaPath: MediaPlayerProperty.mediaPath
        }

        Preview.PreviewPanel
        {
            anchors.left: library.right
            anchors.right: properties.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.leftMargin: 1
            anchors.rightMargin: 1
            hasMedia: MediaPlayerProperty.hasMedia
            mediaName: MediaPlayerProperty.mediaName
            mediaKind: MediaPlayerProperty.mediaKind
            mediaSource: previewBridge.source
            layoutReady: MediaPlayerProperty.previewLayoutReady
            playing: MediaPlayerProperty.playing
            positionMs: MediaPlayerProperty.positionMs
            durationMs: MediaPlayerProperty.durationMs
            canPlay: MediaPlayerProperty.hasMedia &&
                     MediaPlayerProperty.mediaKind !== "Image" &&
                     MediaPlayerProperty.mediaObjLoaded
            onPlayRequested: if (playBridge.binding) playBridge.binding.clicked()
            onStopRequested: if (stopBridge.binding) stopBridge.binding.clicked()
            onPreviewRectReported: (x, y, width, height) => {
                MediaPlayerProperty.previewX = x
                MediaPlayerProperty.previewY = y
                MediaPlayerProperty.previewWidth = width
                MediaPlayerProperty.previewHeight = height
                MediaPlayerProperty.previewRevision += 1
            }
        }
    }
}
