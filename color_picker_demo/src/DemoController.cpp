#include "pch.h"
#include "DemoController.h"
#include "QtKitHost.h"
#include "DemoNames.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace { const size_t kMaxCustomColors = 12; }

CDemoController::CDemoController()
{
    LoadCustomColors();
}
CDemoController::~CDemoController() = default;

unsigned CDemoController::ParseColor(const std::string& value, unsigned fallback)
{
    if (value.size() != 7 || value[0] != '#') return fallback;
    try { return static_cast<unsigned>(std::stoul(value.substr(1), nullptr, 16)) & 0xFFFFFF; }
    catch (...) { return fallback; }
}

std::string CDemoController::FormatColor(unsigned rgb)
{
    char buffer[8] = {};
    snprintf(buffer, sizeof(buffer), "#%06X", rgb & 0xFFFFFF);
    return buffer;
}

void CDemoController::SetColorHex(const std::string& colorHex)
{
    m_rgb = ParseColor(colorHex, m_rgb.load());
}

std::string CDemoController::ColorHex() const
{
    return FormatColor(m_rgb.load());
}

void CDemoController::AddCustomColor()
{
    const unsigned rgb = m_rgb.load();
    m_customColors.erase(std::remove(m_customColors.begin(), m_customColors.end(), rgb), m_customColors.end());
    m_customColors.insert(m_customColors.begin(), rgb);
    if (m_customColors.size() > kMaxCustomColors) m_customColors.pop_back();
    SaveCustomColors();
    PublishState();
}

void CDemoController::ResetToInitialColor()
{
    m_rgb = m_initialRgb;
    PublishState();
}

void CDemoController::LoadCustomColors()
{
    std::ifstream stream("custom_colors.txt");
    std::string color;
    while (std::getline(stream, color) && m_customColors.size() < kMaxCustomColors)
        m_customColors.push_back(ParseColor(color, 0));
}

void CDemoController::SaveCustomColors() const
{
    std::ofstream stream("custom_colors.txt", std::ios::trunc);
    for (unsigned color : m_customColors) stream << FormatColor(color) << "\n";
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

    QtKitHost::Log("PublishState: writing colorHex");
    auto& prop = pContext->property(DemoName.property);
    prop.property("colorHex", ColorHex().c_str());
    std::ostringstream colors;
    for (size_t i = 0; i < m_customColors.size(); ++i) {
        if (i) colors << ",";
        colors << FormatColor(m_customColors[i]);
    }
    QtKitHost::Log("PublishState: writing customColors");
    prop.property("customColors", colors.str().c_str());
    QtKitHost::Log("PublishState: complete");
}
