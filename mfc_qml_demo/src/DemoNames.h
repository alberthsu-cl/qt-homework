#pragma once
//
// The C++ half of the shared name table.
//
// Binding across the QtKit boundary is BY STRING. Both sides must spell the
// same name, and nothing checks that they do - a typo compiles cleanly and
// fails at runtime. PDR's answer is to keep the strings in one constant table
// per feature, mirrored on both sides:
//
//   C++ : src/ui/common/TextToImage_Qt/View/DeductCreditName.h
//   QML : skinQt/qml/DeductCredit/DeductCreditName.qml   (pragma Singleton)
//
// This file is that pattern in miniature. Keep it in sync with
// qml/DemoName.qml - the two lists must match exactly.
//
struct DemoNames
{
    // bindWindow - the QML Window whose HWND we reparent into the MFC frame
    const char* window        = "demo.window";

    // bindProperty - the shared state block (qml/DemoProperty.qml).
    // C++ owns zoomFactor and rotationDeg and writes them through
    // context->property(DemoName.property).property("zoomFactor", 1.25f).
    const char* property      = "demo.property";

    // C++ -> QML
    const char* caption       = "demo.caption";   // bindLabel, we write .text()
    const char* photo         = "demo.photo";     // bindImage, we write the file URL

    // QML -> C++   (bindButton, QML calls binding.clicked())
    const char* zoomInButton  = "demo.zoomInButton";
    const char* zoomOutButton = "demo.zoomOutButton";
    const char* rotateButton  = "demo.rotateButton";
    const char* resetButton   = "demo.resetButton";
};

static const DemoNames DemoName;
