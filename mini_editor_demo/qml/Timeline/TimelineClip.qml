import QtQuick
import QtQuick.Controls

Rectangle
{
    id: root
    property string title: ""
    property int splitCount: 0
    property bool selected: true

    radius: 3
    color: "#396b8d"
    border.color: selected ? "#ffb347" : "#5e8dab"
    border.width: selected ? 2 : 1

    Repeater
    {
        model: root.splitCount
        Rectangle
        {
            x: (index + 1) * root.width / (root.splitCount + 1)
            width: 2
            height: root.height
            color: "#f1f5f8"
            opacity: 0.8
        }
    }

    Label
    {
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        text: root.title
        color: "white"
        elide: Text.ElideRight
        width: parent.width - 20
    }
}
