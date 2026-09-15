#include "pch.h"
#include "DemoController.h"
#include "QtKitHost.h"
#include "DemoNames.h"

#include <algorithm>

namespace {

// The rules live here, as named constants, instead of inline in four QML
// button handlers. That is the whole point of the layer.
const float kZoomMin  = 0.1f;
const float kZoomMax  = 4.0f;
const float kZoomStep = 0.25f;
const int   kRotStep  = 90;

} // namespace

CDemoController::CDemoController()  = default;
CDemoController::~CDemoController() = default;

void CDemoController::ZoomIn()
{
    m_fZoom = std::min(kZoomMax, m_fZoom.load() + kZoomStep);
    PublishState();
}

void CDemoController::ZoomOut()
{
    m_fZoom = std::max(kZoomMin, m_fZoom.load() - kZoomStep);
    PublishState();
}

void CDemoController::Rotate90()
{
    m_nRotation = (m_nRotation.load() + kRotStep) % 360;
    PublishState();
}

void CDemoController::ResetView()
{
    m_fZoom     = 1.0f;
    m_nRotation = 0;
    PublishState();
}

void CDemoController::PublishState()
{
    IQmlContext* pContext = QtKitHost::Inst().Context();
    if (!pContext)
        return;

    // Same guard as every other accessor in this demo: an unbound name does
    // not come back null, it logs "The id does not exist" and then faults.
    if (!pContext->isObjectBound(DemoName.property))
    {
        QtKitHost::Log("PublishState: %s not bound - skipped", DemoName.property);
        return;
    }

    const float fZoom = m_fZoom.load();
    const int   nRot  = m_nRotation.load();

    // One accessor, two typed writes. QML's declarative bindings do the rest -
    // nothing in DemoWindow.qml has to be told that the numbers changed.
    //
    // property() is overloaded per type and picks by the C++ argument, so the
    // float overload needs a float: passing a double here silently selects
    // nothing and will not compile.
    auto& prop = pContext->property(DemoName.property);
    prop.property("zoomFactor",  fZoom);
    prop.property("rotationDeg", nRot);

    QtKitHost::Log("PublishState: zoom=%.2f rot=%d (readback zoom=%.2f rot=%d)",
                   fZoom, nRot,
                   prop.propertyFloat("zoomFactor"),
                   prop.propertyInt("rotationDeg"));
}
