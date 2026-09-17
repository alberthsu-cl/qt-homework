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

void CDemoController::BeginColorEdit()
{
    m_pendingRgb = m_appliedRgb.load();
    m_isEditing = true;
    PublishState();
}

void CDemoController::SetPendingColorHex(const std::string& colorHex)
{
    m_pendingRgb = ParseColor(colorHex, m_pendingRgb.load());
}

void CDemoController::CommitAppliedColorHex(const std::string& colorHex)
{
    const unsigned rgb = ParseColor(colorHex, m_appliedRgb.load());
    m_pendingRgb = rgb;
    m_appliedRgb = rgb;
    m_isEditing = false;
}

void CDemoController::ApplyColorEdit()
{
    m_appliedRgb = m_pendingRgb.load();
    m_isEditing = false;
    PublishState();
}

void CDemoController::CancelColorEdit()
{
    m_pendingRgb = m_appliedRgb.load();
    m_isEditing = false;
    PublishState();
}

std::string CDemoController::AppliedColorHex() const
{
    return FormatColor(m_appliedRgb.load());
}

void CDemoController::AddCustomColor()
{
    const unsigned rgb = m_pendingRgb.load();
    m_customColors.erase(std::remove(m_customColors.begin(), m_customColors.end(), rgb), m_customColors.end());
    m_customColors.insert(m_customColors.begin(), rgb);
    if (m_customColors.size() > kMaxCustomColors) m_customColors.pop_back();
    SaveCustomColors();
    PublishState();
}

void CDemoController::ResetPendingColor()
{
    m_pendingRgb = m_initialRgb;
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

    std::ostringstream colors;
    for (size_t i = 0; i < m_customColors.size(); ++i) {
        if (i) colors << ",";
        colors << FormatColor(m_customColors[i]);
    }

    const std::string appliedColor = AppliedColorHex();
    const std::string pendingColor = FormatColor(m_pendingRgb.load());
    const std::string customColors = colors.str();
    const auto publish = [&](const char* name) {
        if (!pContext->isObjectBound(name))
            return;
        auto& prop = pContext->property(name);
        // colorHex is last because picker QML treats it as the completed state
        // update and refreshes its HSV/RGB controls from that value.
        prop.property("appliedColorHex", appliedColor.c_str());
        prop.property("pickerVisible", m_isEditing.load());
        prop.property("customColors", customColors.c_str());
        prop.property("colorHex", pendingColor.c_str());
    };

    QtKitHost::Log("PublishState: writing main and picker state");
    publish(DemoName.property);
    publish(DemoName.pickerProperty);
    QtKitHost::Log("PublishState: complete");
}
