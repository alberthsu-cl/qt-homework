pragma Singleton
import QtQuick
import QtKit

UIProperty
{
    property UIProperty binding
    property string mediaName: ""
    property bool hasMedia: false
    property bool hasTimelineClip: false
    property bool playing: false
    property int splitCount: 0

    Component.onCompleted: binding = qmlContext.bindProperty(this, MiniEditorName.property)
    Component.onDestruction: qmlContext.unbind(this, MiniEditorName.property)
}
