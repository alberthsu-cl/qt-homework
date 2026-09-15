import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtKit

// The entry .qml. C++ loads this file by absolute path, then pulls this
// Window's HWND out via context->window("demo.window").window() and reparents
// it under the MFC frame.
//
// PDR's equivalent: skinQt/qml/DeductCredit/DeductCredit.qml
Window
{
    id: root

    // No width/height here on purpose. A literal size on the root Window wins
    // over IUIWindow::setPosition(), the scene stays at its declared logical
    // size, and anything anchored to the bottom renders below the visible
    // area. Let the host decide the size - it is the one embedded in an MFC
    // frame whose dimensions it does not control.
    visible: true          // the HWND must exist before C++ can adopt it

    // No native frame: this window is about to become a child of an MFC frame
    // that already has the title bar and menu. PDR gets the same effect from
    // its own Widgets/FramelessApplicationWindow.qml.
    flags: Qt.FramelessWindowHint
    color: "#14171c"
    title: "QML canvas"

    // zoom and rotation are QML-local view state. C++ is told when they change
    // but does not own them - see README.md, "What this demo does not show".
    property real zoomFactor: 1.0
    property int  rotationDeg: 0

    // --- the binding handshake ----------------------------------------------
    // Until this runs, context->window("demo.window") throws
    // QmlObjectNoBoundError on the C++ side, and isObjectBound() returns false.
    property var binding
    Component.onCompleted:   binding = qmlContext.bindWindow(this, DemoName.window)
    Component.onDestruction: qmlContext.unbind(this, DemoName.window)

    // The image URL arrives from C++ as TEXT on this invisible Label, and the
    // Image below binds its source to it.
    //
    // Why not just context->image("demo.photo").source(url)? Because QtKit's
    // bindImage / IUIImage targets QtKit's OWN image item types (FileImageItem,
    // StateImageItem, WebpImageItem - the ones PDR's skinQt widgets are built
    // from), not QtQuick's plain Image. Bind a QtQuick.Image and the source()
    // call is silently dropped: isObjectBound() says true, no error is logged,
    // and the picture never appears. A bound Label is the smallest channel that
    // works with stock QtQuick types.
    Label
    {
        id: imagePathHolder
        visible: false
        text: ""
        property var binding
        Component.onCompleted:   binding = qmlContext.bindLabel(this, DemoName.imagePath)
        Component.onDestruction: qmlContext.unbind(this, DemoName.imagePath)
    }

    // =========================================================================
    //  header - a Label whose text C++ writes
    // =========================================================================
    Rectangle
    {
        id: header
        anchors { left: parent.left; right: parent.right; top: parent.top }
        height: 44
        color: "#1d222a"

        // C++ -> QML, channel 1:  context->label("demo.caption").text("...")
        Label
        {
            id: caption
            anchors { left: parent.left; leftMargin: 16; verticalCenter: parent.verticalCenter }
            color: "#e5e9ef"
            font.pixelSize: 14
            text: "Use  File > Open Image...  in the native MFC menu above"

            property var binding
            Component.onCompleted:   binding = qmlContext.bindLabel(this, DemoName.caption)
            Component.onDestruction: qmlContext.unbind(this, DemoName.caption)
        }

        Label
        {
            anchors { right: parent.right; rightMargin: 16; verticalCenter: parent.verticalCenter }
            color: "#5fc189"
            font.pixelSize: 12
            text: "everything below the menu bar is QML"
        }

        Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: "#2b313a" }
    }

    // =========================================================================
    //  canvas - an Image whose source C++ writes
    // =========================================================================
    Item
    {
        id: canvasArea
        anchors { left: parent.left; right: parent.right; top: header.bottom; bottom: controls.top }
        clip: true

        // checkerboard, so transparency and image bounds are visible
        Canvas
        {
            anchors.fill: parent
            onPaint:
            {
                var ctx = getContext("2d");
                var s = 16;
                for (var y = 0; y < height; y += s)
                    for (var x = 0; x < width; x += s)
                    {
                        ctx.fillStyle = ((x / s + y / s) % 2 === 0) ? "#191d24" : "#1f242c";
                        ctx.fillRect(x, y, s, s);
                    }
            }
        }

        // C++ -> QML, channel 2:  context->image("demo.photo").source("file:///...")
        Image
        {
            id: photo
            anchors.centerIn: parent
            width: parent.width - 48
            height: parent.height - 48
            source: imagePathHolder.text
            fillMode: Image.PreserveAspectFit
            asynchronous: false
            visible: imagePathHolder.text.length > 0 && status === Image.Ready

            scale: root.zoomFactor
            rotation: root.rotationDeg
            Behavior on scale    { NumberAnimation { duration: 120 } }
            Behavior on rotation { NumberAnimation { duration: 160 } }

        }

        Label
        {
            anchors.centerIn: parent
            visible: !photo.visible
            color: "#727c89"
            font.pixelSize: 15
            horizontalAlignment: Text.AlignHCenter
            text: photo.status === Image.Error
                  ? "Qt could not decode that file"
                  : "no image loaded\n\nthe native File menu owns the file dialog;\nthis canvas only renders what C++ hands it"
        }
    }

    // =========================================================================
    //  controls - QML buttons that report to C++
    // =========================================================================
    Rectangle
    {
        id: controls
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
        height: 64
        color: "#1d222a"

        Rectangle { anchors.top: parent.top; width: parent.width; height: 1; color: "#2b313a" }

        Label
        {
            anchors { verticalCenter: parent.verticalCenter; left: parent.left; leftMargin: 16 }
            color: "#9ba4b1"; font.pixelSize: 12; font.family: "Consolas"
            text: "zoom " + root.zoomFactor.toFixed(2) + "x    rot " + root.rotationDeg + " deg"
        }

        Row
        {
            anchors { verticalCenter: parent.verticalCenter; right: parent.right; rightMargin: 16 }
            spacing: 10

            // QML -> C++, the EVENT CHANNEL.
            //
            // binding.clicked() invokes whatever C++ passed to setClickedAction().
            // The local view change and the notification are separate lines on
            // purpose: the first is QML's own business, the second is the part
            // that crosses the boundary and lands in the MFC status strip.
            Button
            {
                text: "Zoom out"
                property var binding
                Component.onCompleted:   binding = qmlContext.bindButton(this, DemoName.zoomOutButton)
                Component.onDestruction: qmlContext.unbind(this, DemoName.zoomOutButton)
                onClicked:
                {
                    root.zoomFactor = Math.max(0.1, root.zoomFactor - 0.25)
                    binding.clicked()
                }
            }

            Button
            {
                text: "Zoom in"
                property var binding
                Component.onCompleted:   binding = qmlContext.bindButton(this, DemoName.zoomInButton)
                Component.onDestruction: qmlContext.unbind(this, DemoName.zoomInButton)
                onClicked:
                {
                    root.zoomFactor = Math.min(4.0, root.zoomFactor + 0.25)
                    binding.clicked()
                }
            }

            Button
            {
                text: "Rotate 90"
                property var binding
                Component.onCompleted:   binding = qmlContext.bindButton(this, DemoName.rotateButton)
                Component.onDestruction: qmlContext.unbind(this, DemoName.rotateButton)
                onClicked:
                {
                    root.rotationDeg = (root.rotationDeg + 90) % 360
                    binding.clicked()
                }
            }

            Button
            {
                text: "Reset"
                property var binding
                Component.onCompleted:   binding = qmlContext.bindButton(this, DemoName.resetButton)
                Component.onDestruction: qmlContext.unbind(this, DemoName.resetButton)
                onClicked:
                {
                    root.zoomFactor = 1.0
                    root.rotationDeg = 0
                    binding.clicked()
                }
            }
        }
    }
}
