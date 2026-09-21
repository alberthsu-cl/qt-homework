import QtQuick
import QtQuick.Controls

Column
{
    id: root
    property real scaleValue: 100
    property real opacityValue: 100
    signal resetRequested()
    spacing: 5

    Label
    {
        width: parent.width
        text: "Size / Position"
        color: "#e7ebf1"
        font.bold: true
    }
    Label
    {
        width: parent.width
        text: "Scale"
        color: "#aab3bf"
    }
    Row
    {
        width: parent.width
        spacing: 8
        Slider
        {
            width: parent.width - 54
            height: 30
            from: 25
            to: 200
            value: root.scaleValue
            onMoved: root.scaleValue = value
        }
        Label
        {
            width: 46
            anchors.verticalCenter: parent.verticalCenter
            text: Math.round(root.scaleValue) + "%"
            color: "#e4e8ee"
        }
    }

    Item { width: 1; height: 3 }
    Label
    {
        width: parent.width
        text: "Opacity / Fading"
        color: "#e7ebf1"
        font.bold: true
    }
    Label
    {
        width: parent.width
        text: "Opacity"
        color: "#aab3bf"
    }
    Row
    {
        width: parent.width
        spacing: 8
        Slider
        {
            width: parent.width - 54
            height: 30
            from: 0
            to: 100
            value: root.opacityValue
            onMoved: root.opacityValue = value
        }
        Label
        {
            width: 46
            anchors.verticalCenter: parent.verticalCenter
            text: Math.round(root.opacityValue) + "%"
            color: "#e4e8ee"
        }
    }

    Item { width: 1; height: 3 }
    Label
    {
        width: parent.width
        text: "Simple Effect"
        color: "#e7ebf1"
        font.bold: true
    }
    ComboBox
    {
        width: parent.width
        height: 34
        model: ["None", "Brightness", "Contrast", "Saturation", "Invert"]
    }
    Label
    {
        width: parent.width
        text: "Intensity"
        color: "#aab3bf"
    }
    Slider
    {
        width: parent.width
        height: 30
        from: 0
        to: 100
        value: 50
    }

    Item { width: 1; height: 3 }
    Button
    {
        width: parent.width
        height: 34
        text: "Reset Adjustments"
        onClicked:
        {
            root.scaleValue = 100
            root.opacityValue = 100
            root.resetRequested()
        }
    }
    Item { width: 1; height: 4 }
}
