pragma Singleton
import QtQuick
import QtKit

UIProperty
{
    property UIProperty binding
    property string mediaId: ""
    property string mediaPath: ""
    property string mediaName: ""
    property string mediaKind: "Unknown"
    property string mediaCatalogJson: "[]"
    property int requestedMediaIndex: -1
    property bool mediaObjLoaded: false
    property string mediaLoadStatus: ""
    property int mediaLoadError: 0
    property bool hasMedia: false
    property bool playing: false
    property int previewX: 0
    property int previewY: 0
    property int previewWidth: 0
    property int previewHeight: 0
    property int previewRevision: 0
    property bool previewLayoutReady: false
    property int positionMs: 0
    property int durationMs: 0

    Component.onCompleted: binding = qmlContext.bindProperty(this, MediaPlayerName.property)
    Component.onDestruction: qmlContext.unbind(this, MediaPlayerName.property)
}
