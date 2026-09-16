import QtQuick

// A copy of skinQt/qml/Effects/TintEffect.qml, pointed at this demo's own
// copy of the baked shaders.
//
// It recolours whatever texture it is given: the RGB comes from `tint`, the
// alpha comes from the source image. That is why the four icons in images/
// are plain white outlines - only their alpha channel survives, so their own
// colour is irrelevant. One file serves every state.
//
// Cost of this approach, measured rather than assumed:
//   ShaderEffect lives in Qt6Quick.dll, which the demo already stages, so the
//   tint adds NO new DLL. It adds exactly two files, shaders/*.qsb, 2.2 KB
//   together. The obvious Qt 6 alternative - MultiEffect's `colorization` -
//   would have pulled in Qt6QuickEffects.dll plus its whole qml module.
//
// The property NAMES here are load-bearing. `src` and `tint` are what
// tint.frag.qsb was compiled against; rename either and the shader silently
// renders nothing. .qsb is pre-compiled bytecode, not source Qt can adapt.
ShaderEffect {
    property variant src
    property color tint: Qt.rgba(1, 1, 1, 1.0)
    fragmentShader: Qt.resolvedUrl("shaders/tint.frag.qsb")
    vertexShader:   Qt.resolvedUrl("shaders/basic.vert.qsb")
}
