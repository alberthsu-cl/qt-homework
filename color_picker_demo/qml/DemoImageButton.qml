import QtQuick
import QtQuick.Controls

// A miniature of skinQt/qml/Widgets/TintImageButton.qml - PDR's image button.
//
// THE ROOT MUST STAY A Controls Button. qmlContext.bindButton() attaches to
// this object, and IUIButton::setClickedAction() is driven by the Button's
// own `clicked` signal. Rebuild this as an Item + MouseArea and the QML -> C++
// event channel disappears, with isObjectBound() still reporting true - the
// same silent-success failure as findings #7 and #8.
//
// ONE image, not four. The legacy pattern next door
// (skinQt/qml/Widgets/ImageButton.qml) swaps files per state, loading
// `image://Media/<name>_n|_h|_p|_d[_2x].png` - four PNGs per button, doubled
// again for high DPI, all served by a C++ IUIImageProvider this demo does not
// register. TintImageButton replaced that with a single monochrome source
// recoloured by a shader, which is why these buttons need no provider at all
// and why the icons ship as loose .svg files next to this .qml.
//
// SVG, not PNG: Qt6Svg.dll and imageformats/qsvg.dll are already in the staged
// runtime, and one vector file stays sharp at every DPI instead of needing an
// _2x twin.
Button {
    id: root

    required property string source
    property string tip: ""
    property int    margins: 9

    // The four state colours. TintImageButton takes these from skinQt's Style
    // singleton; the demo has no theme, so they are literals.
    property color colorNormal:   "#c3ccd8"
    property color colorHover:    "#ffffff"
    property color colorPressed:  "#5fc189"
    property color colorDisabled: "#59616c"

    implicitWidth: 38
    implicitHeight: 38

    // An icon-only button has no visible label, so give the name to the
    // accessibility tree and to a tooltip instead of dropping it entirely.
    Accessible.name: tip
    ToolTip.visible: hovered && tip.length > 0
    ToolTip.text: tip
    ToolTip.delay: 400

    // The Basic style's default contentItem is a Text for `text`. We never set
    // `text`, but an empty Text still participates in sizing - replace it.
    contentItem: Item {}

    background: Rectangle {
        radius: 6
        color: !root.enabled ? "transparent"
             : root.down     ? "#2b313a"
             : root.hovered  ? "#242a33"
                             : "transparent"
        border.width: 1
        border.color: (root.hovered && !root.down) ? "#39414c" : "transparent"
    }

    Image {
        id: icon
        anchors.fill: parent
        anchors.margins: root.margins
        source: root.source
        fillMode: Image.PreserveAspectFit

        // An SVG is rasterised ONCE at sourceSize and then scaled like a
        // bitmap. Leave this out and the icon is rendered at its 24x24 viewBox
        // and resampled, which looks soft on a 125% display.
        sourceSize: Qt.size(width * 2, height * 2)

        // layer.enabled turns this Image into a texture the shader can sample;
        // `src: icon` then hands that texture to the effect.
        layer.enabled: true
        layer.effect: DemoTintEffect {
            src: icon
            tint: {
                let c = !root.enabled ? root.colorDisabled
                      : root.down     ? root.colorPressed
                      : root.hovered  ? root.colorHover
                                      : root.colorNormal
                return Qt.rgba(c.r, c.g, c.b, 1.0)
            }
        }
    }
}
