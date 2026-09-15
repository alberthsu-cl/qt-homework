pragma Singleton
import QtQuick
import QtKit

// The shared state block: one object, bound under ONE name, carrying every
// value that crosses the C++/QML boundary in this demo.
//
// This is the binding type the other three (bindWindow, bindLabel, bindButton)
// cannot replace. They carry a window handle, a string and an event; this one
// carries typed STATE, in both directions, and QML binds to it declaratively.
// PDR's equivalent: skinQt/qml/VoiceClone/VCProperty.qml, and ~87 others.
//
// The root type MUST be UIProperty, not QtObject. Both bind without error and
// isObjectBound() returns true for either, but only a UIProperty leaves an
// IUIProperty behind for the C++ accessor - bind a QtObject and
// context->property(name) reports "The id does not exist" and then faults.
// Every *Property.qml in skinQt is a UIProperty; none is a QtObject.
UIProperty
{
    id: root
    property UIProperty binding

    // C++ owns both of these. QML reads them and must not assign to them -
    // the buttons below report intent and let the controller decide.
    property real zoomFactor:  1.0
    property int  rotationDeg: 0

    Component.onCompleted:   binding = qmlContext.bindProperty(this, DemoName.property)
    Component.onDestruction: qmlContext.unbind(this, DemoName.property)
}
