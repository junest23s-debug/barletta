[⚠️ Suspicious Content] int opusFrameSizeIdx = 3;
int opusFrameSize = 960;
int channels = 2;

#include "overlay.hpp"
#include "other/configs/globals.h"
#include "libraries/opus/include/opus.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include "sys_info_utils.hpp" 
#include <algorithm> 
#include <cmath>     
#include "skCrypt.hpp"
#include <fstream>
#include <WinUser.h>
#include <cstring> 
#include <TlHelp32.h>
#include <psapi.h>   
#include <chrono>
#include <dwmapi.h>
#include <map> 

std::string GetProcessName();

void RunDBChecker() {

    return;
}

#define MY_PI 3.14159265358979323846f

template <typename T>
T Min(T a, T b) { return (a < b) ? a : b; }

template <typename T>
T Max(T a, T b) { return (a > b) ? a : b; }

static float time_since_start = 0.0f;
ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
ImVec4 main_color = ImVec4(1.0f, 0.0f, 0.0f, 1.00f);
int TOGGLE_HOTKEY = VK_F5;

HWND hwnd = nullptr;
bool show_imgui_window = true;

bool dbCheckerActive = false;
std::string dbCheckStatus = "Ready";

int opusEncodingMethod = 0;
int opusSignalType = 0;
const char* opusEncodingMethods[] = { "VBR (Variable)", "CBR (Constant)", "CVBR (Constrained VBR)" };
const char* opusSignalTypes[] = { "Auto", "Voice Optimized", "Music Optimized" };

float bassEQ = 0.0f;

float midEQ = 0.0f;
float highEQ = 0.0f;
float panningValue = 0.0f;
bool inHeadLeft = false;
bool inHeadRight = false;
bool energyEnabled = true;
float energyValue = 510000.0f;
float bitrateValue = 510000.0f;
int audioChannelMode = 1;
const char* channelNames[] = { "Mono", "Stereo" };
float reverbMix = 0.5f;
float reverbSize = 0.7f;
float reverbDamping = 0.5f;
float reverbWidth = 1.0f;
bool reverbEnabled = false;
bool rgbModeEnabled = false;
float rgbCycleSpeed = 0.5f;
bool bassBoostEnabled = false;

void StyleTabBar();
void DrawNestedFrame(const char* title, bool rgbMode);
void EndNestedFrame(bool rgbMode);
void DrawAlignedSeparator(const char* label, bool rgbMode, ImVec4 customColor = ImVec4(0, 0, 0, 0));

float AnimateSin(float speed, float min, float max, float phase);
float AnimateExpo(float speed, float min, float max);
float AnimateElastic(float speed, float min, float max);
void AnimateBorderCorners(const ImVec2& topLeft, const ImVec2& bottomRight, const ImVec4& color, float thickness);
bool AnimatedButton(const char* label, const ImVec2& size);
void AnimatedSlider(const char* label, float* value, float min, float max, const char* tooltip);
bool AnimatedToggleButton(const char* label, bool* value);
bool AnimatedPanningSlider(const char* label, float* value, float min, float max);
void DrawAdvancedSeparator(bool rgbMode, float widthFactor, float yFactor);
void DrawEnhancedPanel(const char* label, float alpha);
void EndEnhancedPanel(bool rgbMode);
bool PremiumButton(const char* label, const ImVec2& size);
void DrawInnerContentBorder();

void EndNestedFrame(bool rgbMode) {

    ImGui::Spacing();

    ImVec4 separatorColor;
    if (rgbMode) {

        float timeOffset = time_since_start * rgbCycleSpeed;
        float r = 0.5f + 0.5f * sinf(timeOffset);
        float g = 0.5f + 0.5f * sinf(timeOffset + 2.0f * MY_PI / 3.0f);
        float b = 0.5f + 0.5f * sinf(timeOffset + 4.0f * MY_PI / 3.0f);
        separatorColor = ImVec4(r, g, b, 0.7f);
    }
    else {

        float pulse = AnimateSin(1.5f, 0.7f, 1.0f, 0.0f);
        separatorColor = ImVec4(main_color.x * pulse, main_color.y * pulse, main_color.z * pulse, 0.7f);
    }

    ImVec2 p = ImGui::GetCursorScreenPos();
    float width = ImGui::GetContentRegionAvail().x;
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    float lineHeight = 2.0f;
    drawList->AddRectFilledMultiColor(
        ImVec2(p.x - 10.0f, p.y),
        ImVec2(p.x + width + 10.0f, p.y + lineHeight),
        ImGui::ColorConvertFloat4ToU32(ImVec4(separatorColor.x, separatorColor.y, separatorColor.z, 0.0f)),
        ImGui::ColorConvertFloat4ToU32(separatorColor),
        ImGui::ColorConvertFloat4ToU32(separatorColor),
        ImGui::ColorConvertFloat4ToU32(ImVec4(separatorColor.x, separatorColor.y, separatorColor.z, 0.0f))
    );

    ImGui::Dummy(ImVec2(0, lineHeight + 5.0f));

    ImGui::PopID();

    ImGui::Unindent(10.0f);

    ImGui::Spacing();
    ImGui::Spacing();

}
void DrawAlignedSeparator(const char* label, bool rgbMode, ImVec4 customColor) {

    ImVec4 sepColor;
    if (customColor.x != 0 || customColor.y != 0 || customColor.z != 0 || customColor.w != 0) {

        sepColor = customColor;
    }
    else {

        sepColor = rgbMode ?
            ImVec4(
                0.5f + 0.5f * sinf(time_since_start * rgbCycleSpeed),
                0.5f + 0.5f * sinf(time_since_start * rgbCycleSpeed + 2.0f * MY_PI / 3.0f),
                0.5f + 0.5f * sinf(time_since_start * rgbCycleSpeed + 4.0f * MY_PI / 3.0f),
                0.8f
            ) : ImVec4(main_color.x, main_color.y, main_color.z, 0.8f);
    }

    ImVec4 textColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    ImGuiStyle& style = ImGui::GetStyle();
    float windowWidth = ImGui::GetWindowWidth();
    float contentWidth = ImGui::GetContentRegionAvail().x;

    float lineWidth = contentWidth * 0.75f;

    float borderMargin = style.WindowPadding.x + style.ItemSpacing.x;

    ImGui::Dummy(ImVec2(0, 5.0f));
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    if (label && label[0] != '\0') {

        float textWidth = ImGui::CalcTextSize(label).x;
        float textHeight = ImGui::GetTextLineHeight();
        float textPadding = 12.0f;

        ImVec2 windowPos = ImGui::GetWindowPos();

        float lineY = cursorPos.y + textHeight * 0.5f;
        float textY = cursorPos.y;

        float startX = windowPos.x + (windowWidth - lineWidth) / 2.0f;
        float textStartX = windowPos.x + (windowWidth - textWidth) / 2.0f;

        float lineThickness = 0.8f;

        float lineLeftEnd = textStartX - textPadding;

        float leftBorderMargin = 16.0f;
        float minStartX = windowPos.x + leftBorderMargin;
        startX = (startX > minStartX) ? startX : minStartX;

        drawList->AddCircleFilled(
            ImVec2(startX, lineY),
            lineThickness / 2.0f,
            ImGui::ColorConvertFloat4ToU32(sepColor),
            12
        );

        drawList->AddLine(
            ImVec2(startX, lineY),
            ImVec2(lineLeftEnd, lineY),
            ImGui::ColorConvertFloat4ToU32(sepColor),
            lineThickness
        );

        drawList->AddText(
            ImVec2(textStartX + 1.0f, textY + 1.0f),
            ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 0.4f)),
            label
        );

        drawList->AddText(
            ImVec2(textStartX, textY),
            ImGui::ColorConvertFloat4ToU32(textColor),
            label
        );

        ImGui::Dummy(ImVec2(0, textHeight + 2.0f));

        float lineRightStart = textStartX + textWidth + textPadding;

        float maxRight = windowPos.x + windowWidth - borderMargin;
        float lineRightEnd = (startX + lineWidth < maxRight) ? (startX + lineWidth) : maxRight;

        drawList->AddLine(
            ImVec2(lineRightStart, lineY),
            ImVec2(lineRightEnd, lineY),
            ImGui::ColorConvertFloat4ToU32(sepColor),
            lineThickness
        );

        drawList->AddCircleFilled(
            ImVec2(lineRightEnd, lineY),
            lineThickness / 2.0f,
            ImGui::ColorConvertFloat4ToU32(sepColor),
            12
        );

        ImGui::Dummy(ImVec2(0, textHeight));
    }
    else {

        ImVec2 windowPos = ImGui::GetWindowPos();
        float startX = windowPos.x + (windowWidth - lineWidth) / 2.0f;
        float startY = ImGui::GetCursorScreenPos().y;

        float minStartX = windowPos.x + borderMargin;
        startX = (startX > minStartX) ? startX : minStartX;

        float endX = (startX + lineWidth < windowPos.x + windowWidth - borderMargin) ?
            (startX + lineWidth) : (windowPos.x + windowWidth - borderMargin);

        float lineThickness = 0.8f;

        drawList->AddCircleFilled(
            ImVec2(startX, startY),
            lineThickness / 2.0f,
            ImGui::ColorConvertFloat4ToU32(sepColor),
            12
        );

        drawList->AddLine(
            ImVec2(startX, startY),
            ImVec2(endX, startY),
            ImGui::ColorConvertFloat4ToU32(sepColor),
            lineThickness
        );

        drawList->AddCircleFilled(
            ImVec2(endX, startY),
            lineThickness / 2.0f,
            ImGui::ColorConvertFloat4ToU32(sepColor),
            12
        );

        ImGui::Dummy(ImVec2(0, 5.0f));
    }

    ImGui::Dummy(ImVec2(0, 5.0f));
}

void CreateConsole() {
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
}

void UpdateTime() {
    static auto start_time = std::chrono::high_resolution_clock::now();
    auto current_time = std::chrono::high_resolution_clock::now();
    time_since_start = std::chrono::duration<float>(current_time - start_time).count();

    if (rgbModeEnabled) {

        float timeOffset = time_since_start * rgbCycleSpeed;
        float r = 0.5f + 0.5f * sinf(timeOffset);
        float g = 0.5f + 0.5f * sinf(timeOffset + 2.0f * MY_PI / 3.0f);
        float b = 0.5f + 0.5f * sinf(timeOffset + 4.0f * MY_PI / 3.0f);

        ImVec4 rgbColor = ImVec4(r, g, b, 1.0f);

        ImGuiStyle& style = ImGui::GetStyle();

        style.TabBarBorderSize = 1.0f;
        style.TabRounding = 4.0f;

        style.Colors[ImGuiCol_Button] = ImVec4(r * 0.6f, g * 0.6f, b * 0.6f, 0.8f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 0.9f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(r, g, b, 1.0f);

        style.Colors[ImGuiCol_SliderGrab] = rgbColor;
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(r * 1.2f, g * 1.2f, b * 1.2f, 1.0f);
        style.Colors[ImGuiCol_CheckMark] = rgbColor;

        style.Colors[ImGuiCol_Separator] = rgbColor;
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(r * 1.1f, g * 1.1f, b * 1.1f, 1.0f);
        style.Colors[ImGuiCol_SeparatorActive] = ImVec4(r * 1.2f, g * 1.2f, b * 1.2f, 1.0f);

        style.Colors[ImGuiCol_Tab] = ImVec4(r * 0.5f, g * 0.5f, b * 0.5f, 0.8f);
        style.Colors[ImGuiCol_TabHovered] = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 0.9f);
        style.Colors[ImGuiCol_TabActive] = rgbColor;
        style.Colors[ImGuiCol_TabUnfocused] = ImVec4(r * 0.3f, g * 0.3f, b * 0.3f, 0.8f);
        style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(r * 0.6f, g * 0.6f, b * 0.6f, 0.9f);

        style.Colors[ImGuiCol_Border] = rgbColor;

        style.Colors[ImGuiCol_Header] = ImVec4(r * 0.4f, g * 0.4f, b * 0.4f, 0.8f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(r * 0.6f, g * 0.6f, b * 0.6f, 0.9f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 1.0f);

        style.Colors[ImGuiCol_FrameBg] = ImVec4(r * 0.2f, g * 0.2f, b * 0.2f, 0.8f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(r * 0.3f, g * 0.3f, b * 0.3f, 0.9f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(r * 0.4f, g * 0.4f, b * 0.4f, 1.0f);

        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 1.0f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(r, g, b, 1.0f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(r * 1.2f, g * 1.2f, b * 1.2f, 1.0f);

        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(r * 0.4f, g * 0.4f, b * 0.4f, 0.8f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(r * 0.6f, g * 0.6f, b * 0.6f, 1.0f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 1.0f);
        style.Colors[ImGuiCol_PlotLines] = rgbColor;
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(r * 1.2f, g * 1.2f, b * 1.2f, 1.0f);
        style.Colors[ImGuiCol_PlotHistogram] = rgbColor;
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(r * 1.2f, g * 1.2f, b * 1.2f, 1.0f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(r * 0.5f, g * 0.5f, b * 0.5f, 1.0f);
    }
    else {

        ImGuiStyle& style = ImGui::GetStyle();

        style.TabBarBorderSize = 1.0f;
        style.TabRounding = 4.0f;

        style.Colors[ImGuiCol_Button] = ImVec4(main_color.x * 0.6f, main_color.y * 0.6f, main_color.z * 0.6f, 1.0f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(main_color.x * 0.7f, main_color.y * 0.7f, main_color.z * 0.7f, 1.0f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(main_color.x * 0.8f, main_color.y * 0.8f, main_color.z * 0.8f, 1.0f);
        style.Colors[ImGuiCol_CheckMark] = main_color;
        style.Colors[ImGuiCol_SliderGrab] = main_color;
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(main_color.x * 1.2f, main_color.y * 1.2f, main_color.z * 1.2f, 1.0f);

        style.Colors[ImGuiCol_Separator] = main_color;
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(main_color.x * 1.2f, main_color.y * 1.2f, main_color.z * 1.2f, 1.0f);
        style.Colors[ImGuiCol_SeparatorActive] = ImVec4(main_color.x * 1.4f, main_color.y * 1.4f, main_color.z * 1.4f, 1.0f);

        style.Colors[ImGuiCol_Tab] = ImVec4(main_color.x * 0.5f, main_color.y * 0.5f, main_color.z * 0.5f, 1.0f);
        style.Colors[ImGuiCol_TabHovered] = main_color;
        style.Colors[ImGuiCol_TabActive] = main_color;
        style.Colors[ImGuiCol_TabUnfocused] = ImVec4(main_color.x * 0.3f, main_color.y * 0.3f, main_color.z * 0.3f, 1.0f);
        style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(main_color.x * 0.5f, main_color.y * 0.5f, main_color.z * 0.5f, 1.0f);

        style.Colors[ImGuiCol_Border] = main_color;

        style.Colors[ImGuiCol_FrameBg] = ImVec4(main_color.x * 0.2f, main_color.y * 0.2f, main_color.z * 0.2f, 1.0f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(main_color.x * 0.3f, main_color.y * 0.3f, main_color.z * 0.3f, 1.0f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(main_color.x * 0.4f, main_color.y * 0.4f, main_color.z * 0.4f, 1.0f);
        style.Colors[ImGuiCol_Header] = ImVec4(main_color.x * 0.4f, main_color.y * 0.4f, main_color.z * 0.4f, 1.0f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(main_color.x * 0.5f, main_color.y * 0.5f, main_color.z * 0.5f, 1.0f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(main_color.x * 0.6f, main_color.y * 0.6f, main_color.z * 0.6f, 1.0f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(main_color.x * 0.4f, main_color.y * 0.4f, main_color.z * 0.4f, 1.0f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(main_color.x * 0.6f, main_color.y * 0.6f, main_color.z * 0.6f, 1.0f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(main_color.x * 0.8f, main_color.y * 0.8f, main_color.z * 0.8f, 1.0f);
        style.Colors[ImGuiCol_ScrollbarGrab] = main_color;
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(main_color.x * 1.1f, main_color.y * 1.1f, main_color.z * 1.1f, 1.0f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(main_color.x * 1.2f, main_color.y * 1.2f, main_color.z * 1.2f, 1.0f);
    }
}

std::wstring ConvertToWide(const char* str) {
    int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
    std::wstring wstr(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str, -1, &wstr[0], len);
    return wstr;
}

namespace Spoofing {
    bool ProcessIsolation = false;
    bool Hider = true;
}

void CreateConfiguration(const char* file_path) {
    std::ofstream ofs(file_path, std::ios::binary);
    if (ofs) {

        float default_gain = 1.0f;
        float default_exp_gain = 1.0f;
        float default_vunits_gain = 1.0f;

        bool default_isolation = true;
        bool default_hider = false;
        DWORD default_hotkey = VK_INSERT;
        float default_bitrate = 64000.0f;

        ImVec4 default_main_color = ImVec4(0.5f, 0.8f, 0.5f, 1.0f);
        ImVec4 default_clear_color = ImVec4(0.05f, 0.05f, 0.05f, 0.94f);

        bool default_reverb = false;
        float default_reverb_mix = 0.2f;
        float default_reverb_size = 0.5f;
        float default_reverb_damping = 0.5f;
        float default_reverb_width = 1.0f;

        bool default_rgb_mode = false;
        float default_rgb_speed = 0.6f;

        int default_channel_mode = 1;

        float default_bass_eq = 0.0f;
        float default_mid_eq = 0.0f;
        float default_high_eq = 0.0f;
        bool default_bass_boost = false;

        bool default_energy_enabled = false;
        float default_energy_value = 510000.0f;

        float default_panning = 0.0f;
        bool default_in_head_left = false;
        bool default_in_head_right = false;

        ofs.write(reinterpret_cast<const char*>(&default_gain), sizeof(default_gain));
        ofs.write(reinterpret_cast<const char*>(&default_exp_gain), sizeof(default_exp_gain));
        ofs.write(reinterpret_cast<const char*>(&default_vunits_gain), sizeof(default_vunits_gain));
        ofs.write(reinterpret_cast<const char*>(&default_isolation), sizeof(default_isolation));
        ofs.write(reinterpret_cast<const char*>(&default_hider), sizeof(default_hider));
        ofs.write(reinterpret_cast<const char*>(&default_hotkey), sizeof(default_hotkey));
        ofs.write(reinterpret_cast<const char*>(&default_bitrate), sizeof(default_bitrate));

        ofs.write(reinterpret_cast<const char*>(&default_main_color), sizeof(default_main_color));
        ofs.write(reinterpret_cast<const char*>(&default_clear_color), sizeof(default_clear_color));
        ofs.write(reinterpret_cast<const char*>(&default_reverb), sizeof(default_reverb));
        ofs.write(reinterpret_cast<const char*>(&default_reverb_mix), sizeof(default_reverb_mix));
        ofs.write(reinterpret_cast<const char*>(&default_reverb_size), sizeof(default_reverb_size));
        ofs.write(reinterpret_cast<const char*>(&default_reverb_damping), sizeof(default_reverb_damping));
        ofs.write(reinterpret_cast<const char*>(&default_reverb_width), sizeof(default_reverb_width));
        ofs.write(reinterpret_cast<const char*>(&default_rgb_mode), sizeof(default_rgb_mode));
        ofs.write(reinterpret_cast<const char*>(&default_rgb_speed), sizeof(default_rgb_speed));
        ofs.write(reinterpret_cast<const char*>(&default_channel_mode), sizeof(default_channel_mode));
        ofs.write(reinterpret_cast<const char*>(&default_bass_eq), sizeof(default_bass_eq));
        ofs.write(reinterpret_cast<const char*>(&default_mid_eq), sizeof(default_mid_eq));
        ofs.write(reinterpret_cast<const char*>(&default_high_eq), sizeof(default_high_eq));
        ofs.write(reinterpret_cast<const char*>(&default_bass_boost), sizeof(default_bass_boost));
        ofs.write(reinterpret_cast<const char*>(&default_energy_enabled), sizeof(default_energy_enabled));
        ofs.write(reinterpret_cast<const char*>(&default_energy_value), sizeof(default_energy_value));
        ofs.write(reinterpret_cast<const char*>(&default_panning), sizeof(default_panning));
        ofs.write(reinterpret_cast<const char*>(&default_in_head_left), sizeof(default_in_head_left));
        ofs.write(reinterpret_cast<const char*>(&default_in_head_right), sizeof(default_in_head_right));
        ofs.close();
    }
}

void SaveConfiguration(const char* file_path) {
    std::ofstream ofs(file_path, std::ios::binary);
    if (ofs) {

        ofs.write(reinterpret_cast<const char*>(&Gain), sizeof(Gain));
        ofs.write(reinterpret_cast<const char*>(&ExpGain), sizeof(ExpGain));
        ofs.write(reinterpret_cast<const char*>(&VunitsGain), sizeof(VunitsGain));

        ofs.write(reinterpret_cast<const char*>(&Spoofing::ProcessIsolation), sizeof(Spoofing::ProcessIsolation));
        ofs.write(reinterpret_cast<const char*>(&Spoofing::Hider), sizeof(Spoofing::Hider));
        ofs.write(reinterpret_cast<const char*>(&TOGGLE_HOTKEY), sizeof(TOGGLE_HOTKEY));
        ofs.write(reinterpret_cast<const char*>(&bitrateValue), sizeof(bitrateValue));

        ofs.write(reinterpret_cast<const char*>(&main_color), sizeof(main_color));
        ofs.write(reinterpret_cast<const char*>(&clear_color), sizeof(clear_color));

        ofs.write(reinterpret_cast<const char*>(&reverbEnabled), sizeof(reverbEnabled));
        ofs.write(reinterpret_cast<const char*>(&reverbMix), sizeof(reverbMix));
        ofs.write(reinterpret_cast<const char*>(&reverbSize), sizeof(reverbSize));
        ofs.write(reinterpret_cast<const char*>(&reverbDamping), sizeof(reverbDamping));
        ofs.write(reinterpret_cast<const char*>(&reverbWidth), sizeof(reverbWidth));

        ofs.write(reinterpret_cast<const char*>(&rgbModeEnabled), sizeof(rgbModeEnabled));
        ofs.write(reinterpret_cast<const char*>(&rgbCycleSpeed), sizeof(rgbCycleSpeed));

        ofs.write(reinterpret_cast<const char*>(&audioChannelMode), sizeof(audioChannelMode));

        ofs.write(reinterpret_cast<const char*>(&bassEQ), sizeof(bassEQ));
        ofs.write(reinterpret_cast<const char*>(&midEQ), sizeof(midEQ));
        ofs.write(reinterpret_cast<const char*>(&highEQ), sizeof(highEQ));
        ofs.write(reinterpret_cast<const char*>(&bassBoostEnabled), sizeof(bassBoostEnabled));

        ofs.write(reinterpret_cast<const char*>(&energyEnabled), sizeof(energyEnabled));
        ofs.write(reinterpret_cast<const char*>(&energyValue), sizeof(energyValue));

        ofs.write(reinterpret_cast<const char*>(&panningValue), sizeof(panningValue));
        ofs.write(reinterpret_cast<const char*>(&inHeadLeft), sizeof(inHeadLeft));
        ofs.write(reinterpret_cast<const char*>(&inHeadRight), sizeof(inHeadRight));

        ofs.close();
    }
}

void LoadConfiguration(const char* file_path) {
    std::ifstream ifs(file_path, std::ios::binary);
    if (ifs) {

        ifs.read(reinterpret_cast<char*>(&Gain), sizeof(Gain));
        ifs.read(reinterpret_cast<char*>(&ExpGain), sizeof(ExpGain));

        if (ifs.peek() != EOF) {
            ifs.read(reinterpret_cast<char*>(&VunitsGain), sizeof(VunitsGain));
        }

        ifs.read(reinterpret_cast<char*>(&Spoofing::ProcessIsolation), sizeof(Spoofing::ProcessIsolation));
        ifs.read(reinterpret_cast<char*>(&Spoofing::Hider), sizeof(Spoofing::Hider));

        if (ifs.peek() != EOF) {
            ifs.read(reinterpret_cast<char*>(&TOGGLE_HOTKEY), sizeof(TOGGLE_HOTKEY));
        }

        if (ifs.peek() != EOF) {
            ifs.read(reinterpret_cast<char*>(&bitrateValue), sizeof(bitrateValue));

            bitrateValue = (bitrateValue < 16000.0f) ? 16000.0f : (bitrateValue > 510000.0f ? 510000.0f : bitrateValue);
        }

        if (ifs.peek() != EOF) {
            ifs.read(reinterpret_cast<char*>(&main_color), sizeof(main_color));
            if (ifs.peek() != EOF) {
                ifs.read(reinterpret_cast<char*>(&clear_color), sizeof(clear_color));
            }
        }

        if (ifs.peek() != EOF) {
            ifs.read(reinterpret_cast<char*>(&reverbEnabled), sizeof(reverbEnabled));

            if (ifs.peek() != EOF) {
                ifs.read(reinterpret_cast<char*>(&reverbMix), sizeof(reverbMix));
                ifs.read(reinterpret_cast<char*>(&reverbSize), sizeof(reverbSize));
                ifs.read(reinterpret_cast<char*>(&reverbDamping), sizeof(reverbDamping));
                ifs.read(reinterpret_cast<char*>(&reverbWidth), sizeof(reverbWidth));
            }
        }

        if (ifs.peek() != EOF) {
            ifs.read(reinterpret_cast<char*>(&rgbModeEnabled), sizeof(rgbModeEnabled));

            if (ifs.peek() != EOF) {
                ifs.read(reinterpret_cast<char*>(&rgbCycleSpeed), sizeof(rgbCycleSpeed));

                rgbCycleSpeed = (rgbCycleSpeed < 0.1f) ? 0.1f : (rgbCycleSpeed > 2.0f ? 2.0f : rgbCycleSpeed);
            }
        }

        if (ifs.peek() != EOF) {
            ifs.read(reinterpret_cast<char*>(&audioChannelMode), sizeof(audioChannelMode));

            audioChannelMode = (audioChannelMode < 0) ? 0 : (audioChannelMode > 1 ? 1 : audioChannelMode);
        }

        if (ifs.peek() != EOF) {
            ifs.read(reinterpret_cast<char*>(&bassEQ), sizeof(bassEQ));
            if (ifs.peek() != EOF) {
                ifs.read(reinterpret_cast<char*>(&midEQ), sizeof(midEQ));
                if (ifs.peek() != EOF) {
                    ifs.read(reinterpret_cast<char*>(&highEQ), sizeof(highEQ));
                    if (ifs.peek() != EOF) {
                        ifs.read(reinterpret_cast<char*>(&bassBoostEnabled), sizeof(bassBoostEnabled));
                    }
                }
            }
        }

        if (ifs.peek() != EOF) {
            ifs.read(reinterpret_cast<char*>(&energyEnabled), sizeof(energyEnabled));
            if (ifs.peek() != EOF) {
                ifs.read(reinterpret_cast<char*>(&energyValue), sizeof(energyValue));

                energyValue = (energyValue < 100000.0f) ? 100000.0f : (energyValue > 1000000.0f ? 1000000.0f : energyValue);
            }
        }

        if (ifs.peek() != EOF) {
            ifs.read(reinterpret_cast<char*>(&panningValue), sizeof(panningValue));
            if (ifs.peek() != EOF) {
                ifs.read(reinterpret_cast<char*>(&inHeadLeft), sizeof(inHeadLeft));
                if (ifs.peek() != EOF) {
                    ifs.read(reinterpret_cast<char*>(&inHeadRight), sizeof(inHeadRight));
                }
            }
        }

        ifs.close();

        if (hwnd) {
            UnregisterHotKey(hwnd, 1);
            RegisterHotKey(hwnd, 1, MOD_NOREPEAT, TOGGLE_HOTKEY);
        }
    }
    else {

        CreateConfiguration(file_path);
    }
}

void ResetConfiguration() {

    Gain = 1.0f;
    ExpGain = 1.0f;
    VunitsGain = 1.0f;

    Spoofing::ProcessIsolation = true;
    Spoofing::Hider = false;
    TOGGLE_HOTKEY = VK_INSERT;
    bitrateValue = 64000.0f;

    main_color = ImVec4(0.5f, 0.8f, 0.5f, 1.0f);
    clear_color = ImVec4(0.05f, 0.05f, 0.05f, 0.94f);

    reverbEnabled = false;
    reverbMix = 0.2f;
    reverbSize = 0.5f;
    reverbDamping = 0.5f;
    reverbWidth = 1.0f;

    rgbModeEnabled = false;
    rgbCycleSpeed = 0.6f;

    audioChannelMode = 1;

    bassEQ = 0.0f;
    midEQ = 0.0f;
    highEQ = 0.0f;
    bassBoostEnabled = false;

    energyEnabled = false;
    energyValue = 510000.0f;

    panningValue = 0.0f;
    inHeadLeft = false;
    inHeadRight = false;

    if (hwnd) {
        UnregisterHotKey(hwnd, 1);
        RegisterHotKey(hwnd, 1, MOD_NOREPEAT, TOGGLE_HOTKEY);
    }
}

void ToggleWindowVisibility() {
    show_imgui_window ? ::ShowWindow(hwnd, SW_HIDE) : ::ShowWindow(hwnd, SW_SHOW);
    show_imgui_window = !show_imgui_window;
}

float GetPulsatingValue(float speed = 1.0f, float min = 0.7f, float max = 1.0f) {
    return min + (max - min) * 0.5f * (1.0f + sinf(time_since_start * speed));
}

void DrawSeparator(ImVec4 color = ImVec4(0.7f, 0.0f, 0.0f, 1.0f), float thickness = 1.0f) {

    if (rgbModeEnabled) {

        float timeOffset = time_since_start * rgbCycleSpeed;
        float r = 0.5f + 0.5f * sinf(timeOffset);
        float g = 0.5f + 0.5f * sinf(timeOffset + 2.0f * MY_PI / 3.0f);
        float b = 0.5f + 0.5f * sinf(timeOffset + 4.0f * MY_PI / 3.0f);

        color = ImVec4(r, g, b, 1.0f);
    }

    ImGui::PushStyleColor(ImGuiCol_Separator, color);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 10));
    ImGui::Separator();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::Spacing();
}

void DrawCornerDecorations(const ImVec2& topLeft, const ImVec2& bottomRight, const ImVec4& color, float thickness = 1.0f, float cornerLength = 10.0f) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    drawList->AddLine(
        ImVec2(topLeft.x, topLeft.y),
        ImVec2(topLeft.x + cornerLength, topLeft.y),
        ImGui::ColorConvertFloat4ToU32(color),
        thickness
    );
    drawList->AddLine(
        ImVec2(topLeft.x, topLeft.y),
        ImVec2(topLeft.x, topLeft.y + cornerLength),
        ImGui::ColorConvertFloat4ToU32(color),
        thickness
    );

    drawList->AddLine(
        ImVec2(bottomRight.x, topLeft.y),
        ImVec2(bottomRight.x - cornerLength, topLeft.y),
        ImGui::ColorConvertFloat4ToU32(color),
        thickness
    );
    drawList->AddLine(
        ImVec2(bottomRight.x, topLeft.y),
        ImVec2(bottomRight.x, topLeft.y + cornerLength),
        ImGui::ColorConvertFloat4ToU32(color),
        thickness
    );

    drawList->AddLine(
        ImVec2(topLeft.x, bottomRight.y),
        ImVec2(topLeft.x + cornerLength, bottomRight.y),
        ImGui::ColorConvertFloat4ToU32(color),
        thickness
    );
    drawList->AddLine(
        ImVec2(topLeft.x, bottomRight.y),
        ImVec2(topLeft.x, bottomRight.y - cornerLength),
        ImGui::ColorConvertFloat4ToU32(color),
        thickness
    );

    drawList->AddLine(
        ImVec2(bottomRight.x, bottomRight.y),
        ImVec2(bottomRight.x - cornerLength, bottomRight.y),
        ImGui::ColorConvertFloat4ToU32(color),
        thickness
    );
    drawList->AddLine(
        ImVec2(bottomRight.x, bottomRight.y),
        ImVec2(bottomRight.x, bottomRight.y - cornerLength),
        ImGui::ColorConvertFloat4ToU32(color),
        thickness
    );
}

void DrawSectionBorder(const ImVec2& min, const ImVec2& max, const ImVec4& color, float thickness = 1.0f, float rounding = 4.0f) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    drawList->AddRect(
        min,
        max,
        ImGui::ColorConvertFloat4ToU32(color),
        rounding,
        0,
        thickness
    );

    ImVec4 bgColor = ImVec4(0.05f, 0.05f, 0.05f, 0.3f);
    drawList->AddRectFilled(
        ImVec2(min.x + 1, min.y + 1),
        ImVec2(max.x - 1, max.y - 1),
        ImGui::ColorConvertFloat4ToU32(bgColor),
        rounding - 1.0f
    );
}

void DrawMainWindowBorder()
{
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImVec2 windowEnd = ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y);

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImVec4 borderColor;
    if (rgbModeEnabled) {
        float timeOffset = time_since_start * rgbCycleSpeed;
        float r = 0.5f + 0.5f * sinf(timeOffset);
        float g = 0.5f + 0.5f * sinf(timeOffset + 2.0f * MY_PI / 3.0f);
        float b = 0.5f + 0.5f * sinf(timeOffset + 4.0f * MY_PI / 3.0f);
        borderColor = ImVec4(r, g, b, 1.0f);
    }
    else {

        float pulse = AnimateSin(1.5f, 0.9f, 1.0f, 0.0f);
        borderColor = ImVec4(main_color.x * pulse, main_color.y * pulse, main_color.z * pulse, 1.0f);
    }

    float thickness = 1.0f;

    drawList->AddLine(
        ImVec2(windowPos.x, windowPos.y),
        ImVec2(windowEnd.x - 1, windowPos.y),
        ImGui::ColorConvertFloat4ToU32(borderColor),
        thickness
    );

    drawList->AddLine(
        ImVec2(windowEnd.x - 1, windowPos.y),
        ImVec2(windowEnd.x - 1, windowEnd.y - 1),
        ImGui::ColorConvertFloat4ToU32(borderColor),
        thickness
    );

    drawList->AddLine(
        ImVec2(windowEnd.x - 1, windowEnd.y - 1),
        ImVec2(windowPos.x, windowEnd.y - 1),
        ImGui::ColorConvertFloat4ToU32(borderColor),
        thickness
    );

    drawList->AddLine(
        ImVec2(windowPos.x, windowEnd.y - 1),
        ImVec2(windowPos.x, windowPos.y),
        ImGui::ColorConvertFloat4ToU32(borderColor),
        thickness
    );
}

void SectionHeader(const char* label) {
    ImGui::Spacing();
    ImGui::Spacing();

    ImVec4 headerColor;
    if (rgbModeEnabled) {

        float timeOffset = time_since_start * rgbCycleSpeed;
        float r = 0.5f + 0.5f * sinf(timeOffset);
        float g = 0.5f + 0.5f * sinf(timeOffset + 2.0f * MY_PI / 3.0f);
        float b = 0.5f + 0.5f * sinf(timeOffset + 4.0f * MY_PI / 3.0f);

        headerColor = ImVec4(r, g, b, 1.0f);
    }
    else {
        float pulse = AnimateSin(1.5f, 0.7f, 1.0f, 0.0f);
        headerColor = ImVec4(main_color.x * pulse, main_color.y * pulse, main_color.z * pulse, 1.0f);
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    float width = ImGui::GetWindowWidth() - 30.0f;

    ImVec2 textSize = ImGui::CalcTextSize(label);
    float textX = (width - textSize.x) / 2.0f + 15.0f;

    ImVec2 startPos = ImGui::GetCursorScreenPos();
    float separatorY = startPos.y + textSize.y / 2.0f;

    float headerHeight = textSize.y + 8.0f;
    ImVec4 bgColor = ImVec4(0.12f, 0.12f, 0.12f, 0.8f);
    ImVec4 bgColorDarker = ImVec4(0.08f, 0.08f, 0.08f, 0.9f);

    drawList->AddRectFilledMultiColor(
        ImVec2(startPos.x + 8.0f, startPos.y - 2.0f),
        ImVec2(startPos.x + width - 8.0f, startPos.y + headerHeight),
        ImGui::ColorConvertFloat4ToU32(bgColorDarker),
        ImGui::ColorConvertFloat4ToU32(bgColorDarker),
        ImGui::ColorConvertFloat4ToU32(bgColor),
        ImGui::ColorConvertFloat4ToU32(bgColor)
    );

    ImVec4 borderColor = headerColor;
    borderColor.w = 0.7f;
    drawList->AddRect(
        ImVec2(startPos.x + 8.0f, startPos.y - 2.0f),
        ImVec2(startPos.x + width - 8.0f, startPos.y + headerHeight),
        ImGui::ColorConvertFloat4ToU32(borderColor),
        4.0f, ImDrawFlags_RoundCornersAll, 1.0f
    );

    float lineWidth = (textX - 20.0f) * 0.8f;
    if (lineWidth > 0) {
        drawList->AddRectFilledMultiColor(
            ImVec2(startPos.x + 15.0f, separatorY),
            ImVec2(startPos.x + 15.0f + lineWidth, separatorY + 1.0f),
            ImGui::ColorConvertFloat4ToU32(ImVec4(headerColor.x, headerColor.y, headerColor.z, 0.0f)),
            ImGui::ColorConvertFloat4ToU32(headerColor),
            ImGui::ColorConvertFloat4ToU32(headerColor),
            ImGui::ColorConvertFloat4ToU32(ImVec4(headerColor.x, headerColor.y, headerColor.z, 0.0f))
        );
    }

    float rightLineStart = textX + textSize.x + 5.0f;
    float rightLineWidth = (width - (rightLineStart - startPos.x)) * 0.8f;
    if (rightLineWidth > 0) {
        drawList->AddRectFilledMultiColor(
            ImVec2(rightLineStart, separatorY),
            ImVec2(rightLineStart + rightLineWidth, separatorY + 1.0f),
            ImGui::ColorConvertFloat4ToU32(headerColor),
            ImGui::ColorConvertFloat4ToU32(ImVec4(headerColor.x, headerColor.y, headerColor.z, 0.0f)),
            ImGui::ColorConvertFloat4ToU32(ImVec4(headerColor.x, headerColor.y, headerColor.z, 0.0f)),
            ImGui::ColorConvertFloat4ToU32(headerColor)
        );
    }

    ImGui::Dummy(ImVec2(1.0f, 2.0f));

    ImVec4 shadowColor = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
    drawList->AddText(
        ImVec2(startPos.x + textX + 1.0f, startPos.y + 4.0f + 1.0f),
        ImGui::ColorConvertFloat4ToU32(shadowColor),
        label
    );

    drawList->AddText(
        ImVec2(startPos.x + textX, startPos.y + 4.0f),
        ImGui::ColorConvertFloat4ToU32(headerColor),
        label
    );

    ImGui::Dummy(ImVec2(1.0f, headerHeight + 2.0f));
    ImGui::Spacing();
}

void DrawSectionEnd() {

    ImGui::Spacing();
}

void DrawSlider(const char* label, float* value, float min, float max, const char* tooltip = nullptr) {

    AnimatedProfessionalSlider(label, value, min, max, "%.1f", tooltip);
}

struct BandPassFilter {
    float a0 = 0.0f, a1 = 0.0f, a2 = 0.0f, b1 = 0.0f, b2 = 0.0f;
    float x1, x2, y1, y2;
    float gain = 1.0f;

    BandPassFilter() : x1(0), x2(0), y1(0), y2(0) {}

    void reset() {
        x1 = x2 = y1 = y2 = 0;
    }

    float process(float sample) {

        float result = a0 * sample + a1 * x1 + a2 * x2 - b1 * y1 - b2 * y2;
        x2 = x1;
        x1 = sample;
        y2 = y1;
        y1 = result;
        return result * gain;
    }
};

BandPassFilter bassFilter, midFilter, highFilter;
BandPassFilter deesingFilter;

class FreeverbReverb {
private:

    static constexpr int NUM_COMBS = 8;
    static constexpr int NUM_ALLPASSES = 4;
    static constexpr float FIXED_GAIN = 0.015f;
    static constexpr float SCALE_WET = 3.0f;
    static constexpr float SCALE_DRY = 2.0f;
    static constexpr float SCALE_DAMP = 0.4f;
    static constexpr float SCALE_ROOM = 0.28f;
    static constexpr float OFFSET_ROOM = 0.7f;
    static constexpr float INITIAL_ROOM = 0.5f;
    static constexpr float INITIAL_DAMP = 0.5f;
    static constexpr float INITIAL_WET = 1.0f / SCALE_WET;
    static constexpr float INITIAL_DRY = 0.0f;
    static constexpr float INITIAL_WIDTH = 1.0f;
    static constexpr float INITIAL_MODE = 0.0f;
    static constexpr float FREEZE_MODE = 0.5f;

    static constexpr int STEREO_SPREAD = 23;

    static constexpr int COMB_TUNING_L[NUM_COMBS] = { 1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617 };
    static constexpr int ALLPASS_TUNING_L[NUM_ALLPASSES] = { 556, 441, 341, 225 };

    struct Comb {
        float* buffer = nullptr;
        int bufsize = 0;
        int bufidx = 0;
        float feedback = 0.0f;
        float filterstore = 0.0f;
        float damp1 = 0.0f;
        float damp2 = 0.0f;

        ~Comb() {
            if (buffer) delete[] buffer;
            buffer = nullptr;
        }

        void init(int size) {
            bufsize = size;
            bufidx = 0;
            filterstore = 0.0f;

            if (buffer) delete[] buffer;
            buffer = new float[size]();
        }

        inline float process(float input) {
            float output = buffer[bufidx];
            filterstore = (output * damp2) + (filterstore * damp1);

            buffer[bufidx] = input + (filterstore * feedback);
            if (++bufidx >= bufsize) bufidx = 0;

            return output;
        }

        void mute() {
            filterstore = 0.0f;
            if (buffer) {
                memset(buffer, 0, bufsize * sizeof(float));
            }
        }

        void setdamp(float val) {
            damp1 = val;
            damp2 = 1.0f - val;
        }

        void setfeedback(float val) {
            feedback = val;
        }
    };

    struct Allpass {
        float* buffer = nullptr;
        int bufsize = 0;
        int bufidx = 0;
        float feedback = 0.5f;

        ~Allpass() {
            if (buffer) delete[] buffer;
            buffer = nullptr;
        }

        void init(int size) {
            bufsize = size;
            bufidx = 0;

            if (buffer) delete[] buffer;
            buffer = new float[size]();
        }

        inline float process(float input) {
            float output = buffer[bufidx];
            buffer[bufidx] = input + (output * feedback);
            if (++bufidx >= bufsize) bufidx = 0;

            return output - input;
        }

        void mute() {
            if (buffer) {
                memset(buffer, 0, bufsize * sizeof(float));
            }
        }

        void setfeedback(float val) {
            feedback = val;
        }
    };

    Comb combL[NUM_COMBS];
    Comb combR[NUM_COMBS];
    Allpass allpassL[NUM_ALLPASSES];
    Allpass allpassR[NUM_ALLPASSES];

    float gain = FIXED_GAIN;
    float roomsize = INITIAL_ROOM * SCALE_ROOM + OFFSET_ROOM;
    float damp = INITIAL_DAMP * SCALE_DAMP;
    float wet = INITIAL_WET * SCALE_WET;
    float wet1 = INITIAL_WIDTH / 2.0f * wet;
    float wet2 = (1.0f - INITIAL_WIDTH) / 2.0f * wet;
    float dry = INITIAL_DRY * SCALE_DRY;
    float width = INITIAL_WIDTH;
    float mode = INITIAL_MODE;

    int sampleRate = 48000;
    int channels = 2;
    bool initialized = false;
    float sampleRateRatio = 1.0f;

public:
    FreeverbReverb() = default;

    ~FreeverbReverb() {

    }

    void init(int rate, int numChannels) {

        sampleRate = rate;
        channels = numChannels;
        initialized = false;

        sampleRateRatio = (float)sampleRate / 44100.0f;

        try {

            for (int i = 0; i < NUM_COMBS; i++) {
                int adjustedSize = (int)(COMB_TUNING_L[i] * sampleRateRatio);
                if (adjustedSize < 10) adjustedSize = 10;

                combL[i].init(adjustedSize);
                combR[i].init(adjustedSize + STEREO_SPREAD);
            }

            for (int i = 0; i < NUM_ALLPASSES; i++) {
                int adjustedSize = (int)(ALLPASS_TUNING_L[i] * sampleRateRatio);
                if (adjustedSize < 10) adjustedSize = 10;

                allpassL[i].init(adjustedSize);
                allpassR[i].init(adjustedSize + STEREO_SPREAD);

                allpassL[i].setfeedback(0.5f);
                allpassR[i].setfeedback(0.5f);
            }

            updateParams(0.8f, 0.2f, 1.0f, 0.5f);
            mute();

            initialized = true;
        }
        catch (...) {
            initialized = false;
        }
    }

    void updateParams(float size, float dampening, float reverbWidth, float mix) {
        if (!initialized) return;

        setRoomSize(size);
        setDamp(dampening);
        setWidth(reverbWidth);
        setWet(mix);
        setDry(1.0f - mix);
    }

    void process(float* inBuffer, int numSamples) {
        if (!initialized || !inBuffer || numSamples <= 0) return;

        if (channels == 1) {
            for (int i = 0; i < numSamples; i++) {
                float input = inBuffer[i] * gain;
                float outL = 0.0f;
                float outR = 0.0f;

                for (int j = 0; j < NUM_COMBS; j++) {
                    outL += combL[j].process(input);
                    outR += combR[j].process(input);
                }

                for (int j = 0; j < NUM_ALLPASSES; j++) {
                    outL = allpassL[j].process(outL);
                    outR = allpassR[j].process(outR);
                }

                inBuffer[i] = outL * wet1 + outR * wet2 + inBuffer[i] * dry;
            }
        }

        else if (channels >= 2) {
            for (int i = 0; i < numSamples; i++) {
                float inputL = inBuffer[i * 2] * gain;
                float inputR = inBuffer[i * 2 + 1] * gain;
                float outL = 0.0f;
                float outR = 0.0f;

                for (int j = 0; j < NUM_COMBS; j++) {
                    outL += combL[j].process(inputL);
                    outR += combR[j].process(inputR);
                }

                for (int j = 0; j < NUM_ALLPASSES; j++) {
                    outL = allpassL[j].process(outL);
                    outR = allpassR[j].process(outR);
                }

                float outL2 = outL * wet1 + outR * wet2;
                float outR2 = outR * wet1 + outL * wet2;

                inBuffer[i * 2] = outL2 + inputL * dry;
                inBuffer[i * 2 + 1] = outR2 + inputR * dry;
            }
        }
    }

    void setRoomSize(float value) {
        if (!initialized) return;

        roomsize = value * SCALE_ROOM + OFFSET_ROOM;

        for (int i = 0; i < NUM_COMBS; i++) {
            combL[i].setfeedback(roomsize);
            combR[i].setfeedback(roomsize);
        }
    }

    void setDamp(float value) {
        if (!initialized) return;

        damp = value * SCALE_DAMP;

        for (int i = 0; i < NUM_COMBS; i++) {
            combL[i].setdamp(damp);
            combR[i].setdamp(damp);
        }
    }

    void setWet(float value) {
        if (!initialized) return;

        wet = value * SCALE_WET;
        updateWetValues();
    }

    void setDry(float value) {
        if (!initialized) return;

        dry = value * SCALE_DRY;
    }

    void setWidth(float value) {
        if (!initialized) return;

        width = value;
        updateWetValues();
    }

    void setFreeze(bool freezeMode) {
        if (!initialized) return;

        if (freezeMode) {
            roomsize = 1.0f;
            damp = 0.0f;

            for (int i = 0; i < NUM_COMBS; i++) {
                combL[i].setfeedback(1.0f);
                combR[i].setfeedback(1.0f);
                combL[i].setdamp(0.0f);
                combR[i].setdamp(0.0f);
            }
        }
        else {

            for (int i = 0; i < NUM_COMBS; i++) {
                combL[i].setfeedback(roomsize);
                combR[i].setfeedback(roomsize);
                combL[i].setdamp(damp);
                combR[i].setdamp(damp);
            }
        }
    }

    void mute() {
        if (!initialized) return;

        for (int i = 0; i < NUM_COMBS; i++) {
            combL[i].mute();
            combR[i].mute();
        }

        for (int i = 0; i < NUM_ALLPASSES; i++) {
            allpassL[i].mute();
            allpassR[i].mute();
        }
    }

private:

    void updateWetValues() {
        wet1 = wet * (width / 2.0f + 0.5f);
        wet2 = wet * ((1.0f - width) / 2.0f);
    }
};

FreeverbReverb reverbProcessor;

void InitEQFilters() {

    bassFilter.a0 = 0.035f;
    bassFilter.a1 = 0.070f;
    bassFilter.a2 = 0.035f;
    bassFilter.b1 = -1.85f;
    bassFilter.b2 = 0.88f;
    bassFilter.gain = 4.0f;

    midFilter.a0 = 0.22f;
    midFilter.a1 = 0.0f;
    midFilter.a2 = -0.22f;
    midFilter.b1 = -1.90f;
    midFilter.b2 = 0.92f;
    midFilter.gain = 3.5f;

    highFilter.a0 = 0.75f;
    highFilter.a1 = -1.30f;
    highFilter.a2 = 0.75f;
    highFilter.b1 = -1.25f;
    highFilter.b2 = 0.70f;
    highFilter.gain = 4.2f;

    deesingFilter.a0 = 0.87f;
    deesingFilter.a1 = -1.65f;
    deesingFilter.a2 = 0.87f;
    deesingFilter.b1 = -1.65f;
    deesingFilter.b2 = 0.85f;
    deesingFilter.gain = 0.75f;

    reverbProcessor.init(48000, 2);
}

const char* FormatPanningText(float value) {
    static char buffer[32];

    if (value < -0.05f) {

        float percentage = -value * 10.0f;
        percentage = Min(percentage, 100.0f);
        snprintf(buffer, sizeof(buffer), "L %.0f%%", percentage);
    }
    else if (value > 0.05f) {

        float percentage = value * 10.0f;
        percentage = Min(percentage, 100.0f);
        snprintf(buffer, sizeof(buffer), "R %.0f%%", percentage);
    }
    else {

        snprintf(buffer, sizeof(buffer), "CENTER");
    }

    return buffer;
}

void ApplyAudioEffects(float* audioBuffer, int bufferSize, int channels) {

    if (!audioBuffer || bufferSize <= 0 || channels <= 0) {
        return;
    }

    bool bufferIsSilent = true;
    for (int i = 0; i < bufferSize * channels; ++i) {
        if (audioBuffer[i] != 0.0f) {
            bufferIsSilent = false;
            break;
        }
    }

    if (bufferIsSilent) {

        return;
    }

    if (!audioBuffer || bufferSize <= 0 || channels <= 0) {
        return;
    }

    static float prevBassEQ = 0.0f;
    static float prevMidEQ = 0.0f;
    static float prevHighEQ = 0.0f;
    static float prevGain = 1.0f;
    static float prevExpGain = 1.0f;
    static float prevVunitsGain = 1.0f;
    static bool prevBassBoostEnabled = false;

    const float smoothingFactor = 0.2f;

    float smoothBassEQ = prevBassEQ + smoothingFactor * (bassEQ - prevBassEQ);
    float smoothMidEQ = prevMidEQ + smoothingFactor * (midEQ - prevMidEQ);
    float smoothHighEQ = prevHighEQ + smoothingFactor * (highEQ - prevHighEQ);
    float smoothGain = prevGain + smoothingFactor * (Gain - prevGain);
    float smoothExpGain = prevExpGain + smoothingFactor * (ExpGain - prevExpGain);
    float smoothVunitsGain = prevVunitsGain + smoothingFactor * (VunitsGain - prevVunitsGain);

    if (Gain == 0.0f) {
        smoothGain = 0.0f;
        prevGain = 0.0f;
    }

    if (VunitsGain == 0.0f) {
        smoothVunitsGain = 0.0f;
        prevVunitsGain = 0.0f;
    }

    if (smoothVunitsGain < 0.0001f && VunitsGain != 0.0f) smoothVunitsGain = 0.0001f;

    prevBassEQ = smoothBassEQ;
    prevMidEQ = smoothMidEQ;
    prevHighEQ = smoothHighEQ;
    if (Gain != 0.0f) prevGain = smoothGain;
    prevExpGain = smoothExpGain;
    if (VunitsGain != 0.0f) prevVunitsGain = smoothVunitsGain;

    bassFilter.reset();
    midFilter.reset();
    highFilter.reset();
    deesingFilter.reset();

    float* processedBuffer = nullptr;
    try {
        processedBuffer = new float[bufferSize * channels];
        if (!processedBuffer) {
            return;
        }

        memcpy(processedBuffer, audioBuffer, bufferSize * channels * sizeof(float));

        float maxAmplitude = 0.0f;
        for (int i = 0; i < bufferSize * channels; i++) {
            float absValue = fabsf(processedBuffer[i]);
            if (absValue > maxAmplitude) {
                maxAmplitude = absValue;
            }
        }

        float safetyScale = 1.0f;
        if (maxAmplitude > 0.9f) {
            safetyScale = 0.9f / maxAmplitude;
            for (int i = 0; i < bufferSize * channels; i++) {
                processedBuffer[i] *= safetyScale;
            }
        }

        for (int i = 0; i < bufferSize; i++) {

            float interpolationFactor = static_cast<float>(i) / bufferSize;
            float bassBoostTransition = 0.0f;

            if (bassBoostEnabled != prevBassBoostEnabled) {
                bassBoostTransition = bassBoostEnabled ? interpolationFactor : (1.0f - interpolationFactor);
            }
            else {
                bassBoostTransition = bassBoostEnabled ? 1.0f : 0.0f;
            }

            for (int ch = 0; ch < channels; ch++) {
                int idx = i * channels + ch;

                float original = processedBuffer[idx];

                float dynamicBassScale = 1.0f;
                float absInput = fabsf(original);
                if (absInput > 0.3f) {

                    dynamicBassScale = 1.0f - ((absInput - 0.3f) / 0.7f) * 0.7f;
                    dynamicBassScale = (dynamicBassScale < 0.3f) ? 0.3f : dynamicBassScale;
                }

                float bassInputSafe = (original < -0.97f) ? -0.97f : (original > 0.97f ? 0.97f : original);

                float bassEQScaled = (smoothBassEQ / 25.0f) * (1.0f + (smoothBassEQ / 70.0f));
                float bassOut = bassFilter.process(bassInputSafe) * bassEQScaled * dynamicBassScale;

                bassOut = tanh(bassOut * 1.1f);

                if (bassBoostEnabled) {

                    float deepBassInput = (original < -0.95f) ? -0.95f : (original > 0.95f ? 0.95f : original);

                    float extremeBass = bassFilter.process(bassFilter.process(deepBassInput)) * 1.5f;

                    float boostAttenuationFactor = 1.0f - Min(0.8f, absInput * 0.6f);
                    extremeBass *= boostAttenuationFactor;

                    float safeBassBoost = tanh(extremeBass * 0.5f) * 0.5f;

                    bassOut += (extremeBass + safeBassBoost) * bassBoostTransition * dynamicBassScale * 1.5f;
                }

                bassOut = (bassOut < -2.0f) ? -2.0f : (bassOut > 2.0f ? 2.0f : bassOut);

                float midInputSafe = (original < -0.97f) ? -0.97f : (original > 0.97f ? 0.97f : original);

                float midEQScaled = (smoothMidEQ / 25.0f) * (1.0f + (smoothMidEQ / 80.0f));
                float midOut = midFilter.process(midInputSafe) * midEQScaled;

                midOut = tanh(midOut * 1.1f);

                midOut = (midOut < -1.8f) ? -1.8f : (midOut > 1.8f ? 1.8f : midOut);

                float highInputSafe = (original < -0.97f) ? -0.97f : (original > 0.97f ? 0.97f : original);
                float highPassed = highFilter.process(highInputSafe);

                float deEssed = deesingFilter.process(highPassed);

                float highEQScaled = (smoothHighEQ / 25.0f) * (1.0f + (smoothHighEQ / 60.0f));
                float highOut = deEssed * highEQScaled;

                highOut = tanh(highOut * 1.1f);

                if (highOut > 1.2f) {

                    highOut = 1.2f + (highOut - 1.2f) * 0.2f;
                }
                else if (highOut < -1.2f) {

                    highOut = -1.2f + (highOut + 1.2f) * 0.2f;
                }

                float eq_mix = 0.85f;

                float combined = original * (1.0f - eq_mix) + (original + bassOut + midOut + highOut) * eq_mix;

                combined = (combined < -2.5f) ? -2.5f : (combined > 2.5f ? 2.5f : combined);

                if (fabsf(combined) > 1.5f) {

                    combined = 1.5f * tanh(combined / 1.5f);
                }
                else if (fabsf(combined) > 1.0f) {

                    float limitFactor = 0.2f + 0.8f * ((1.5f - fabsf(combined)) / 0.5f);
                    combined = combined * limitFactor + (combined > 0 ? 1.0f : -1.0f) * (1.0f - limitFactor);
                }

                processedBuffer[idx] = combined;
            }
        }

        prevBassBoostEnabled = bassBoostEnabled;

        if (reverbEnabled && reverbMix > 0.0f) {

            reverbProcessor.updateParams(reverbSize, reverbDamping, reverbWidth, reverbMix);

            reverbProcessor.process(processedBuffer, bufferSize);
        }

        if (energyEnabled) {
            float energyMod = sinf(time_since_start * 8.0f) * 0.3f + 0.8f;

            float energyFactor = 1.0f + Min(1000.0f, (energyValue / 2500.0f)) * energyMod;

            for (int i = 0; i < bufferSize * channels; i++) {
                processedBuffer[i] *= energyFactor;

                if (processedBuffer[i] > 1.5f) {
                    processedBuffer[i] = 1.5f + (processedBuffer[i] - 1.5f) * 0.1f;
                }
                else if (processedBuffer[i] < -1.5f) {
                    processedBuffer[i] = -1.5f + (processedBuffer[i] + 1.5f) * 0.1f;
                }
            }
        }

        float totalGain = smoothGain * smoothExpGain;

        float safeVunitsGain = (smoothVunitsGain < 0.0001f) ? 0.0001f : smoothVunitsGain;
        float vUnitsDecibels = 20.0f * log10f(safeVunitsGain);
        float vUnitsMultiplier = powf(10.0f, vUnitsDecibels / 20.0f);

        vUnitsMultiplier = Min(vUnitsMultiplier, 50000.0f);

        if (smoothVunitsGain > 10.0f && smoothExpGain > 5.0f) {

            float combinedReductionFactor = 1.0f - Min(0.1f, (smoothVunitsGain * smoothExpGain) / 2000.0f);
            vUnitsMultiplier *= combinedReductionFactor;

            vUnitsMultiplier = Min(vUnitsMultiplier, 30000.0f);
        }

        if (smoothVunitsGain > 1000.0f) {

            float extraBoost = Min(50.0f, powf((smoothVunitsGain - 1000.0f) / 6000.0f, 1.5f) * 1.5f + 1.0f);
            vUnitsMultiplier = Min(vUnitsMultiplier * extraBoost, 50000.0f);
        }
        else {
            vUnitsMultiplier = Min(vUnitsMultiplier, 50000.0f);
        }

        totalGain = Min(totalGain * vUnitsMultiplier, 250000.0f);

        float limiterThreshold, limiterRatio, finalSafetyScale;
        if (totalGain > 10000.0f) {
            limiterThreshold = 0.5f;
            limiterRatio = 0.005f;
            finalSafetyScale = 0.4f;
        }
        else if (totalGain > 1000.0f) {
            limiterThreshold = 0.35f;
            limiterRatio = 0.015f;
            finalSafetyScale = 0.55f;
        }
        else if (totalGain > 100.0f) {
            limiterThreshold = 0.45f;
            limiterRatio = 0.03f;
            finalSafetyScale = 0.7f;
        }
        else if (totalGain > 50.0f) {
            limiterThreshold = 0.6f;
            limiterRatio = 0.05f;
            finalSafetyScale = 0.8f;
        }
        else {
            limiterThreshold = 0.8f;
            limiterRatio = 0.1f;
            finalSafetyScale = 1.0f;
        }

        for (int i = 0; i < bufferSize; i++) {

            float gainInterpolationFactor = static_cast<float>(i) / bufferSize;
            float frameGain = totalGain;

            if (totalGain > 20.0f) {

                float smoothStartRatio = 0.1f + 0.9f * gainInterpolationFactor;
                frameGain *= smoothStartRatio;
            }

            for (int ch = 0; ch < channels; ch++) {
                int idx = i * channels + ch;

                float sample = processedBuffer[idx];

                if (sample > 0.4f) {
                    float excess = sample - 0.4f;
                    sample = 0.4f + excess * 0.7f;
                }
                else if (sample < -0.4f) {
                    float excess = -0.4f - sample;
                    sample = -0.4f - excess * 0.7f;
                }

                if (sample > 0.7f) {
                    float excess = sample - 0.7f;
                    sample = 0.7f + excess * 0.4f;
                }
                else if (sample < -0.7f) {
                    float excess = -0.7f - sample;
                    sample = -0.7f - excess * 0.4f;
                }

                if (totalGain > 200.0f) {
                    if (sample > 0.85f) {
                        float excess = sample - 0.85f;
                        sample = 0.85f + excess * 0.2f;
                    }
                    else if (sample < -0.85f) {
                        float excess = -0.85f - sample;
                        sample = -0.85f - excess * 0.2f;
                    }
                }

                sample *= frameGain;

                sample *= finalSafetyScale;

                if (smoothExpGain > 5.0f) {

                    float rageGainFactor = Min(1.0f, smoothExpGain / 100.0f);

                    static float sEnvelope = 0.0f;
                    float currentAbs = fabsf(sample);
                    float attackTime = 0.0008f;
                    float releaseTime = 0.05f;

                    if (currentAbs > sEnvelope) {
                        sEnvelope = sEnvelope + attackTime * (currentAbs - sEnvelope);
                    }
                    else {
                        sEnvelope = sEnvelope + releaseTime * (currentAbs - sEnvelope);
                    }

                    static float prevSEnvelope = 0.0f;
                    float sRise = sEnvelope - prevSEnvelope;
                    bool isPotentialS = (sRise > 0.05f) && (sEnvelope > 0.3f);

                    if (isPotentialS) {

                        float sReduction = 0.2f * rageGainFactor * sEnvelope;
                        if (smoothExpGain > 80.0f) {
                            sReduction *= 1.7f;
                        }

                        if (sample > 0) {
                            sample -= sReduction;
                        }
                        else {
                            sample += sReduction;
                        }

                        float softLimit = 0.85f - (0.2f * rageGainFactor);
                        if (sample > softLimit) {
                            sample = softLimit + (sample - softLimit) * 5.0f;
                        }
                        else if (sample < -softLimit) {
                            sample = -softLimit + (sample + softLimit) * 5.0f;
                        }
                    }

                    prevSEnvelope = sEnvelope;

                    float clarityFactor = Min(0.7f, (smoothExpGain - 5.0f) / 90.0f);

                    float presenceBoost = midFilter.process(sample) * 0.5f * clarityFactor;
                    float harmonic = tanh(sample * 0.8f) * 0.35f * clarityFactor;
                    if (smoothExpGain > 80.0f) {
                        presenceBoost *= 0.4f;
                        harmonic *= 0.4f;
                    }

                    sample = sample * (1.0f - clarityFactor * 1.2f) + (sample + presenceBoost + harmonic) * clarityFactor * 1.2f;

                    float attackSharpness = Min(1.0f, fabs(sample * 2.5f));
                    sample *= (0.9f + attackSharpness * 0.1f);

                    static float prevSample = 0.0f;
                    if (smoothExpGain > 100.0f) {
                        sample = 0.7f * sample + 0.3f * prevSample;
                    }
                    prevSample = sample;

                }

                if (sample > limiterThreshold) {
                    float excess = sample - limiterThreshold;

                    sample = limiterThreshold + excess * limiterRatio;
                }
                else if (sample < -limiterThreshold) {
                    float excess = -limiterThreshold - sample;

                    sample = -limiterThreshold - excess * limiterRatio;
                }

                if (smoothExpGain > 20.0f) {

                    float sibilantThreshold = limiterThreshold - (0.15f * Min(1.0f, (smoothExpGain - 20.0f) / 100.0f));

                    float sBandEnergy = deesingFilter.process(sample * 0.4f);
                    float absEnergy = fabsf(sBandEnergy);

                    if (absEnergy > 0.15f) {
                        float sLimitFactor = Min(0.8f, (absEnergy - 0.15f) * 4.0f);
                        float reductionAmount = sLimitFactor * (fabsf(sample) - sibilantThreshold) * 0.7f;

                        if (reductionAmount > 0) {
                            if (sample > 0) {
                                sample -= reductionAmount;
                            }
                            else {
                                sample += reductionAmount;
                            }

                            float compensationAmount = reductionAmount * 0.15f;
                            sample += (sample > 0) ? compensationAmount : -compensationAmount;
                        }
                    }
                }

                if (totalGain > 50.0f) {

                    sample = 0.95f * tanh(sample * 0.9f);
                }
                else if (fabs(sample) > 0.95f) {

                    sample = 0.95f * (sample > 0 ? 1.0f : -1.0f) +
                        0.05f * sample;
                }

                sample = sample > 10.0f ? 10.0f : (sample < -10.0f ? -10.0f : sample);

                processedBuffer[idx] = sample;
            }
        }

        if (channels == 2 && audioChannelMode == 1) {

            float panBoostFactor = 1.0f;
            if (totalGain > 50.0f) {

                panBoostFactor = 1.0f + Min(0.5f, (totalGain - 50.0f) / 500.0f);
            }

            float normalizedPanning = panningValue / 10.0f;

            float panAngle = (normalizedPanning + 1.0f) * (MY_PI / 4.0f);
            float leftGain = cosf(panAngle) * (1.0f + (panBoostFactor - 1.0f) * 0.5f);
            float rightGain = sinf(panAngle) * (1.0f + (panBoostFactor - 1.0f) * 0.5f);

            float microDelay = fabsf(normalizedPanning) * 0.2f;

            static float prevLeftSample = 0.0f;
            static float prevRightSample = 0.0f;

            for (int i = 0; i < bufferSize; i++) {

                int leftIdx = i * 2;
                int rightIdx = i * 2 + 1;

                if (leftIdx < bufferSize * channels && rightIdx < bufferSize * channels) {
                    float leftSample = processedBuffer[leftIdx];
                    float rightSample = processedBuffer[rightIdx];

                    float leftPanned = leftSample * leftGain;
                    float rightPanned = rightSample * rightGain;

                    float crossfeedAmount = 0.1f * (1.0f - fabsf(normalizedPanning));
                    leftPanned += rightSample * crossfeedAmount;
                    rightPanned += leftSample * crossfeedAmount;

                    if (normalizedPanning < 0) {

                        rightPanned = rightPanned * (1.0f - microDelay) + prevRightSample * microDelay;
                    }
                    else if (normalizedPanning > 0) {

                        leftPanned = leftPanned * (1.0f - microDelay) + prevLeftSample * microDelay;
                    }

                    prevLeftSample = leftSample;
                    prevRightSample = rightSample;

                    processedBuffer[leftIdx] = leftPanned;
                    processedBuffer[rightIdx] = rightPanned;
                }
            }
        }

        if (channels == 2 && audioChannelMode == 1) {

            float inHeadBoostFactor = 1.5f;
            float inHeadReductionFactor = 0.6f;

            if (totalGain > 50.0f) {

                inHeadBoostFactor = 1.5f + Min(0.5f, (totalGain - 50.0f) / 500.0f);
                inHeadReductionFactor = 0.6f - Min(0.2f, (totalGain - 50.0f) / 1000.0f);
            }

            static float leftDelayBuffer[8] = { 0 };
            static float rightDelayBuffer[8] = { 0 };
            const int delayBufferSize = 8;
            static int delayBufferIndex = 0;

            for (int i = 0; i < bufferSize; i++) {
                int leftIdx = i * 2;
                int rightIdx = i * 2 + 1;

                if (leftIdx < bufferSize * channels && rightIdx < bufferSize * channels) {

                    float leftSample = processedBuffer[leftIdx];
                    float rightSample = processedBuffer[rightIdx];

                    leftDelayBuffer[delayBufferIndex] = leftSample;
                    rightDelayBuffer[delayBufferIndex] = rightSample;

                    int nextBufferIndex = (delayBufferIndex + 1) % delayBufferSize;

                    if (inHeadLeft) {

                        float leftEarEffect = leftSample * inHeadBoostFactor;

                        float boneConduction = (leftSample + rightSample) * 0.15f;
                        leftEarEffect += boneConduction;

                        float rightEarEffect = rightSample * inHeadReductionFactor;

                        float delayedLeft = leftDelayBuffer[(delayBufferIndex + delayBufferSize - 3) % delayBufferSize];
                        rightEarEffect += delayedLeft * 0.05f;

                        float bassBoost = leftDelayBuffer[(delayBufferIndex + delayBufferSize - 2) % delayBufferSize] * 0.2f;
                        leftEarEffect += bassBoost;

                        processedBuffer[leftIdx] = leftEarEffect;
                        processedBuffer[rightIdx] = rightEarEffect;
                    }

                    else if (inHeadRight) {

                        float rightEarEffect = rightSample * inHeadBoostFactor;

                        float boneConduction = (leftSample + rightSample) * 0.15f;
                        rightEarEffect += boneConduction;

                        float leftEarEffect = leftSample * inHeadReductionFactor;

                        float delayedRight = rightDelayBuffer[(delayBufferIndex + delayBufferSize - 3) % delayBufferSize];
                        leftEarEffect += delayedRight * 0.05f;

                        float bassBoost = rightDelayBuffer[(delayBufferIndex + delayBufferSize - 2) % delayBufferSize] * 0.2f;
                        rightEarEffect += bassBoost;

                        processedBuffer[leftIdx] = leftEarEffect;
                        processedBuffer[rightIdx] = rightEarEffect;
                    }

                    delayBufferIndex = nextBufferIndex;
                }
            }
        }

        memcpy(audioBuffer, processedBuffer, bufferSize * channels * sizeof(float));

        if (channels == 1 && audioChannelMode == 1) {

            for (int i = 0; i < bufferSize; ++i) {
                float monoSample = audioBuffer[i];
                audioBuffer[i * 2] = monoSample;
                audioBuffer[i * 2 + 1] = monoSample;
            }
        }

    }
    catch (...) {

    }

    if (processedBuffer) {
        delete[] processedBuffer;
    }
}

extern "C" opus_int32 custom_opus_encode(OpusEncoder* st, const opus_int16* pcm, int frame_size,
    unsigned char* data, opus_int32 max_data_bytes) {

    if (!st || !pcm || !data || frame_size <= 0 || max_data_bytes <= 0) {

        return opus_encode(st, pcm, frame_size, data, max_data_bytes);
    }

    int channels = 2;

    opus_int32 channels_i32 = 0;
    if (opus_encoder_ctl(st, 1029, &channels_i32) == OPUS_OK) {
        channels = (int)channels_i32;
    }

    try {

        int bitrate = static_cast<int>(bitrateValue);
        bitrate = (bitrate < 16000) ? 16000 : (bitrate > 510000 ? 510000 : bitrate);
        opus_encoder_ctl(st, 4002, bitrate);

        switch (opusEncodingMethod) {
        case 0:
            opus_encoder_ctl(st, 4006, 1);
            opus_encoder_ctl(st, 4030, 0);
            break;
        case 1:
            opus_encoder_ctl(st, 4006, 0);
            opus_encoder_ctl(st, 4030, 0);
            break;
        case 2:
            opus_encoder_ctl(st, 4006, 1);
            opus_encoder_ctl(st, 4030, 1);
            break;
        }

        int actualChannels = audioChannelMode + 1;
        opus_encoder_ctl(st, 4022, actualChannels);

        switch (opusSignalType) {
        case 0:
            opus_encoder_ctl(st, 4024, 1000);
            break;
        case 1:
            opus_encoder_ctl(st, 4024, 3001);
            break;
        case 2:
            opus_encoder_ctl(st, 4024, 3002);
            break;
        }

        int total_samples = frame_size * channels;

        double rms = 0.0;
        for (int i = 0; i < total_samples; i++) {
            rms += static_cast<double>(pcm[i]) * pcm[i];
        }
        rms = sqrt(rms / total_samples);

        bool is_silent = (rms < 30.0);

        static bool was_silent_prev_frame = false;

        static opus_int16* prev_noise_buffer = nullptr;
        static int prev_noise_size = 0;

        bool is_transition = (is_silent != was_silent_prev_frame);

        if (is_silent) {

            if (!prev_noise_buffer || prev_noise_size != total_samples) {

                if (prev_noise_buffer) {
                    delete[] prev_noise_buffer;
                }

                prev_noise_buffer = new opus_int16[total_samples];
                prev_noise_size = total_samples;

                srand(static_cast<unsigned int>(time(nullptr)));
                for (int i = 0; i < total_samples; i++) {

                    prev_noise_buffer[i] = 15 + (rand() % 10);
                    if (rand() % 2) {
                        prev_noise_buffer[i] = -prev_noise_buffer[i];
                    }
                }
            }

            opus_int16* noise_pcm = new opus_int16[total_samples];
            if (!noise_pcm) {
                return opus_encode(st, pcm, frame_size, data, max_data_bytes);
            }

            if (is_transition && was_silent_prev_frame == false) {

                for (int i = 0; i < total_samples; i++) {

                    float mix_ratio = static_cast<float>(i) / total_samples;
                    noise_pcm[i] = static_cast<opus_int16>(
                        pcm[i] * (1.0f - mix_ratio) + prev_noise_buffer[i] * mix_ratio
                        );
                }
            }
            else {

                for (int i = 0; i < total_samples; i++) {

                    int variation = (rand() % 5) - 2;
                    noise_pcm[i] = prev_noise_buffer[i] + variation;
                }
            }

            opus_int32 result = opus_encode(st, noise_pcm, frame_size, data, max_data_bytes);

            memcpy(prev_noise_buffer, noise_pcm, total_samples * sizeof(opus_int16));
            delete[] noise_pcm;

            if (result < 0) {

                opus_encoder_ctl(st, 4016, 1);
                result = opus_encode(st, pcm, frame_size, data, max_data_bytes);
                opus_encoder_ctl(st, 4016, 0);

                if (result < 0) {

                    opus_int16* dc_pcm = new opus_int16[total_samples];
                    if (!dc_pcm) {
                        return opus_encode(st, pcm, frame_size, data, max_data_bytes);
                    }

                    for (int i = 0; i < total_samples; i++) {
                        dc_pcm[i] = 64;
                    }

                    result = opus_encode(st, dc_pcm, frame_size, data, max_data_bytes);
                    delete[] dc_pcm;
                }
            }

            was_silent_prev_frame = true;
            return result;
        }
        else {

            if (is_transition && was_silent_prev_frame && prev_noise_buffer) {

                opus_int16* transition_pcm = new opus_int16[total_samples];
                if (transition_pcm) {

                    for (int i = 0; i < total_samples; i++) {
                        float mix_ratio = static_cast<float>(i) / total_samples;
                        transition_pcm[i] = static_cast<opus_int16>(
                            prev_noise_buffer[i] * (1.0f - mix_ratio) + pcm[i] * mix_ratio
                            );
                    }

                    float* audioBuffer = new float[frame_size * channels];
                    if (audioBuffer) {

                        for (int i = 0; i < total_samples; i++) {
                            audioBuffer[i] = transition_pcm[i] / 32768.0f;
                        }

                        ApplyAudioEffects(audioBuffer, frame_size, channels);

                        opus_int16* processedPcm = new opus_int16[total_samples];
                        if (processedPcm) {
                            for (int i = 0; i < total_samples; i++) {
                                processedPcm[i] = (opus_int16)(audioBuffer[i] * 32767.0f);
                            }

                            opus_int32 result = opus_encode(st, processedPcm, frame_size, data, max_data_bytes);

                            delete[] processedPcm;
                            delete[] audioBuffer;
                            delete[] transition_pcm;

                            was_silent_prev_frame = false;
                            return result;
                        }
                        delete[] audioBuffer;
                    }
                    delete[] transition_pcm;
                }

            }

            float* audioBuffer = new float[frame_size * channels];
            if (!audioBuffer) {

                was_silent_prev_frame = false;
                return opus_encode(st, pcm, frame_size, data, max_data_bytes);
            }

            for (int i = 0; i < total_samples; i++) {
                audioBuffer[i] = pcm[i] / 32768.0f;
            }

            ApplyAudioEffects(audioBuffer, frame_size, channels);

            opus_int16* processedPcm = new opus_int16[total_samples];
            if (!processedPcm) {
                delete[] audioBuffer;
                was_silent_prev_frame = false;
                return opus_encode(st, pcm, frame_size, data, max_data_bytes);
            }

            for (int i = 0; i < total_samples; i++) {
                processedPcm[i] = (opus_int16)(audioBuffer[i] * 32767.0f);
            }

            opus_int32 result = opus_encode(st, processedPcm, frame_size, data, max_data_bytes);

            delete[] audioBuffer;
            delete[] processedPcm;

            was_silent_prev_frame = false;
            return result;
        }
    }
    catch (...) {

        return opus_encode(st, pcm, frame_size, data, max_data_bytes);
    }
}

namespace utilities {
    namespace ui {
        void start() {

            WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"discord.com", nullptr };
            ::RegisterClassExW(&wc);
            hwnd = ::CreateWindowW(wc.lpszClassName, L"discord.com",
                WS_POPUP | WS_VISIBLE,
                100, 100, 500, 500, nullptr, nullptr, wc.hInstance, nullptr);

            if (!CreateDeviceD3D(hwnd)) {
                CleanupDeviceD3D();
                ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
                return;
            }

            RegisterHotKey(hwnd, 1, MOD_NOREPEAT, TOGGLE_HOTKEY);

            ::ShowWindow(hwnd, SW_SHOWDEFAULT);
            ::UpdateWindow(hwnd);

            BOOL value = TRUE;
            DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));

            SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE);
            SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW | WS_EX_LAYERED);

            SetLayeredWindowAttributes(hwnd, 0, 250, LWA_ALPHA);
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;

            io.IniFilename = NULL;

            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

            io.ConfigWindowsMoveFromTitleBarOnly = true;

            ImGui_ImplWin32_Init(hwnd);
            ImGui_ImplDX9_Init(g_pd3dDevice);

            ImGuiStyle& style = ImGui::GetStyle();

            ImGui::SetNextWindowSizeConstraints(ImVec2(360, 280), ImVec2(520, 420));

            style.WindowRounding = 10.0f;
            style.ChildRounding = 8.0f;
            style.FrameRounding = 6.0f;
            style.PopupRounding = 8.0f;
            style.ScrollbarRounding = 6.0f;
            style.GrabRounding = 5.0f;
            style.TabRounding = 8.0f;
            style.WindowBorderSize = 0.0f;
            style.FrameBorderSize = 0.0f;
            style.PopupBorderSize = 1.0f;

            style.WindowPadding = ImVec2(12, 12);
            style.FramePadding = ImVec2(10, 5);
            style.ItemSpacing = ImVec2(10, 8);
            style.ItemInnerSpacing = ImVec2(8, 6);
            style.CellPadding = ImVec2(6, 4);
            style.TouchExtraPadding = ImVec2(0, 0);
            style.IndentSpacing = 22.0f;
            style.ScrollbarSize = 12.0f;
            style.GrabMinSize = 12.0f;

            style.Alpha = 1.0f;
            style.DisabledAlpha = 0.65f;

            ImVec4* colors = ImGui::GetStyle().Colors;

            if (colors) {
                memset(colors, 0, sizeof(ImVec4) * ImGuiCol_COUNT);
            }
            colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
            colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
            colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.12f, 0.98f);
            colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.12f, 0.16f, 0.90f);
            colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.12f, 0.15f, 0.99f);
            colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.25f, 0.50f);
            colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
            colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.18f, 0.23f, 0.80f);
            colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.25f, 0.30f, 0.80f);
            colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.30f, 0.35f, 0.80f);
            colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.12f, 0.16f, 1.00f);
            colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.20f, 1.00f);
            colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.12f, 0.12f, 0.16f, 0.75f);
            colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
            colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.13f, 0.60f);
            colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.40f, 0.40f, 0.45f, 0.40f);
            colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.50f, 0.50f, 0.55f, 0.50f);
            colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.60f, 0.60f, 0.65f, 0.60f);
            colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.95f, 1.00f);
            colors[ImGuiCol_SliderGrab] = ImVec4(0.70f, 0.70f, 0.80f, 0.80f);
            colors[ImGuiCol_SliderGrabActive] = ImVec4(0.85f, 0.85f, 0.95f, 1.00f);
            colors[ImGuiCol_Button] = ImVec4(0.30f, 0.32f, 0.40f, 0.80f);
            colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.37f, 0.45f, 1.00f);
            colors[ImGuiCol_ButtonActive] = ImVec4(0.40f, 0.42f, 0.50f, 1.00f);
            colors[ImGuiCol_Header] = ImVec4(0.25f, 0.25f, 0.30f, 0.80f);
            colors[ImGuiCol_HeaderHovered] = ImVec4(0.35f, 0.35f, 0.40f, 0.80f);
            colors[ImGuiCol_HeaderActive] = ImVec4(0.40f, 0.40f, 0.45f, 0.80f);
            colors[ImGuiCol_Separator] = ImVec4(0.30f, 0.30f, 0.35f, 0.50f);
            colors[ImGuiCol_SeparatorHovered] = ImVec4(0.40f, 0.40f, 0.45f, 0.78f);
            colors[ImGuiCol_SeparatorActive] = ImVec4(0.50f, 0.50f, 0.55f, 1.00f);
            colors[ImGuiCol_ResizeGrip] = ImVec4(0.30f, 0.30f, 0.35f, 0.39f);
            colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.40f, 0.40f, 0.45f, 0.80f);
            colors[ImGuiCol_ResizeGripActive] = ImVec4(0.50f, 0.50f, 0.55f, 1.00f);
            colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.23f, 0.86f);
            colors[ImGuiCol_TabHovered] = ImVec4(0.25f, 0.25f, 0.30f, 0.90f);
            colors[ImGuiCol_TabActive] = ImVec4(0.30f, 0.30f, 0.38f, 0.95f);
            colors[ImGuiCol_TabUnfocused] = ImVec4(0.12f, 0.12f, 0.16f, 0.97f);
            colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.18f, 0.18f, 0.23f, 1.00f);
            colors[ImGuiCol_PlotLines] = ImVec4(0.60f, 0.60f, 0.70f, 1.00f);
            colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
            colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
            colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.45f, 0.00f, 1.00f);
            colors[ImGuiCol_TableHeaderBg] = ImVec4(0.18f, 0.18f, 0.23f, 1.00f);
            colors[ImGuiCol_TableBorderStrong] = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);
            colors[ImGuiCol_TableBorderLight] = ImVec4(0.25f, 0.25f, 0.30f, 0.50f);
            colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.03f);
            colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
            colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
            colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
            colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
            colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
            colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

            const ImVec4 accent_color = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
            colors[ImGuiCol_CheckMark] = accent_color;
            colors[ImGuiCol_SliderGrab] = accent_color;
            colors[ImGuiCol_SliderGrabActive] = ImVec4(accent_color.x * 1.1f, accent_color.y * 1.1f, accent_color.z * 1.1f, 1.0f);
            colors[ImGuiCol_Button] = ImVec4(accent_color.x * 0.4f, accent_color.y * 0.4f, accent_color.z * 0.4f, 0.8f);
            colors[ImGuiCol_ButtonHovered] = ImVec4(accent_color.x * 0.5f, accent_color.y * 0.5f, accent_color.z * 0.5f, 0.9f);
            colors[ImGuiCol_ButtonActive] = ImVec4(accent_color.x * 0.6f, accent_color.y * 0.6f, accent_color.z * 0.6f, 1.0f);
            colors[ImGuiCol_Header] = ImVec4(accent_color.x * 0.2f, accent_color.y * 0.2f, accent_color.z * 0.2f, 0.8f);
            colors[ImGuiCol_HeaderHovered] = ImVec4(accent_color.x * 0.3f, accent_color.y * 0.3f, accent_color.z * 0.3f, 0.9f);
            colors[ImGuiCol_HeaderActive] = ImVec4(accent_color.x * 0.4f, accent_color.y * 0.4f, accent_color.z * 0.4f, 1.0f);
            colors[ImGuiCol_Tab] = ImVec4(accent_color.x * 0.2f, accent_color.y * 0.2f, accent_color.z * 0.2f, 0.9f);
            colors[ImGuiCol_TabHovered] = ImVec4(accent_color.x * 0.3f, accent_color.y * 0.3f, accent_color.z * 0.3f, 0.9f);
            colors[ImGuiCol_TabActive] = ImVec4(accent_color.x * 0.4f, accent_color.y * 0.4f, accent_color.z * 0.4f, 1.0f);

            style.FramePadding = ImVec2(12, 6);
            style.ItemSpacing = ImVec2(10, 8);
            style.ItemInnerSpacing = ImVec2(8, 6);

            style.ScrollbarSize = 10.0f;
            style.ScrollbarRounding = 6.0f;

            style.GrabMinSize = 10.0f;
            style.GrabRounding = 4.0f;

            style.WindowBorderSize = 1.0f;
            style.ChildBorderSize = 1.0f;
            style.PopupBorderSize = 1.0f;
            style.FrameBorderSize = 0.0f;
            style.TabBorderSize = 0.0f;

            style.TabBarBorderSize = 1.0f;

            InitEQFilters();

            colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
            colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
            colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.98f);
            colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.80f);
            colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.95f);
            colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
            colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
            colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.80f);
            colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.25f, 0.25f, 0.90f);
            colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
            colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_TitleBgActive] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
            colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.80f);
            colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
            colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.50f);
            colors[ImGuiCol_ScrollbarGrab] = main_color;
            colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.90f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.70f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_CheckMark] = main_color;
            colors[ImGuiCol_SliderGrab] = main_color;
            colors[ImGuiCol_SliderGrabActive] = ImVec4(0.90f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_Button] = ImVec4(0.12f, 0.12f, 0.12f, 0.90f);
            colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
            colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
            colors[ImGuiCol_Header] = ImVec4(0.26f, 0.26f, 0.26f, 0.90f);
            colors[ImGuiCol_HeaderHovered] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
            colors[ImGuiCol_HeaderActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
            colors[ImGuiCol_Separator] = ImVec4(0.50f, 0.00f, 0.00f, 0.80f);
            colors[ImGuiCol_SeparatorHovered] = main_color;
            colors[ImGuiCol_SeparatorActive] = ImVec4(0.90f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.26f, 0.26f, 0.50f);
            colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.32f, 0.32f, 0.32f, 0.67f);
            colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.40f, 0.40f, 0.95f);
            colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.15f, 0.95f);
            colors[ImGuiCol_TabHovered] = main_color;
            colors[ImGuiCol_TabActive] = main_color;
            colors[ImGuiCol_TabUnfocused] = ImVec4(0.10f, 0.10f, 0.10f, 0.90f);
            colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.60f, 0.00f, 0.00f, 0.90f);
            colors[ImGuiCol_PlotLines] = main_color;
            colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_PlotHistogram] = main_color;
            colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.90f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_TextSelectedBg] = ImVec4(0.70f, 0.00f, 0.00f, 0.50f);
            colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 0.90f, 0.00f, 1.00f);
            colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
            colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
            colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
            colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.50f);

            bool done = false;
            while (!done) {
                MSG msg;
                while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
                    if (msg.message == WM_QUIT)
                        done = true;
                    if (msg.message == WM_HOTKEY && msg.wParam == 1) {
                        ToggleWindowVisibility();
                    }
                    ::TranslateMessage(&msg);
                    ::DispatchMessage(&msg);
                }
                if (done) break;

                UpdateTime();

                if (show_imgui_window) {
                    ImGui_ImplDX9_NewFrame();
                    ImGui_ImplWin32_NewFrame();
                    ImGui::NewFrame();

                    RECT Rect;
                    GetClientRect(hwnd, &Rect);

                    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
                    ImGui::SetNextWindowSize(ImVec2((float)(Rect.right - Rect.left), (float)(Rect.bottom - Rect.top)), ImGuiCond_Always);
                    ImGui::SetNextWindowBgAlpha(1.0f);

                    float pulse = GetPulsatingValue(2.0f, 0.8f, 1.0f);
                    if (!rgbModeEnabled) {

                        style.Colors[ImGuiCol_SliderGrab] = ImVec4(main_color.x * pulse, main_color.y * pulse, main_color.z * pulse, 1.0f);
                        style.Colors[ImGuiCol_CheckMark] = ImVec4(main_color.x * pulse, main_color.y * pulse, main_color.z * pulse, 1.0f);
                        style.Colors[ImGuiCol_TabActive] = ImVec4(main_color.x * pulse, main_color.y * pulse, main_color.z * pulse, 1.0f);
                    }

                    ImGui::Begin("Discord", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);

                    DrawMainWindowBorder();

                    DrawInnerContentBorder();

                    ImVec4 titleColor;
                    if (rgbModeEnabled) {

                        float timeOffset = time_since_start * rgbCycleSpeed;
                        float r = 0.5f + 0.5f * sinf(timeOffset);
                        float g = 0.5f + 0.5f * sinf(timeOffset + 2.0f * MY_PI / 3.0f);
                        float b = 0.5f + 0.5f * sinf(timeOffset + 4.0f * MY_PI / 3.0f);
                        titleColor = ImVec4(r, g, b, 1.0f);
                    }
                    else {

                        titleColor = main_color;
                    }

                    ImGui::BeginGroup();

                    float windowWidth = ImGui::GetWindowWidth();
                    ImVec2 textSize = ImGui::CalcTextSize("Barletta Hook | Made by Ghost of 1337");
                    float closeButtonSize = 24.0f;
                    float closeButtonSpacing = 10.0f;
                    float titlePadding = 5.0f;

                    ImGui::SetCursorPosX((windowWidth - textSize.x - closeButtonSize - closeButtonSpacing) * 0.5f);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + titlePadding);
                    ImGui::PushStyleColor(ImGuiCol_Text, titleColor);
                    ImGui::Text("Barletta Hook | Made by Ghost of 1337");
                    ImGui::PopStyleColor();

                    ImGui::SameLine(windowWidth - closeButtonSize - 25.0f);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - titlePadding);

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.1f, 0.1f, 0.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.0f, 0.0f, 0.8f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 0.9f));
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

                    ImGui::PushFont(ImGui::GetFont());
                    ImGui::SetWindowFontScale(0.8f);
                    if (ImGui::Button("X", ImVec2(closeButtonSize - 4.0f, closeButtonSize - 4.0f))) {
                        ToggleWindowVisibility();
                    }
                    ImGui::SetWindowFontScale(1.0f);
                    ImGui::PopFont();

                    ImGui::PopStyleVar(2);
                    ImGui::PopStyleColor(4);

                    ImGui::EndGroup();

                    ImGui::Spacing();
                    float borderMargin = 15.0f;
                    float sepWidth = windowWidth - (2 * borderMargin);
                    ImVec2 sepStartPos = ImGui::GetCursorScreenPos();
                    sepStartPos.x = ImGui::GetWindowPos().x + borderMargin;

                    ImGui::GetWindowDrawList()->AddLine(
                        sepStartPos,
                        ImVec2(sepStartPos.x + sepWidth, sepStartPos.y),
                        ImGui::ColorConvertFloat4ToU32(titleColor),
                        1.0f
                    );
                    ImGui::Spacing();
                    ImGui::Spacing();

                    if (ImGui::BeginTabBar("MainTabBar", ImGuiTabBarFlags_FittingPolicyScroll)) {

                        if (ImGui::BeginTabItem("Encoder")) {

                            DrawNestedFrame("Opus Encoder", rgbModeEnabled);

                            ImVec4 titleColor;
                            if (rgbModeEnabled) {

                                float timeOffset = time_since_start * rgbCycleSpeed;
                                float r = 0.5f + 0.5f * sinf(timeOffset);
                                float g = 0.5f + 0.5f * sinf(timeOffset + 2.0f * MY_PI / 3.0f);
                                float b = 0.5f + 0.5f * sinf(timeOffset + 4.0f * MY_PI / 3.0f);
                                titleColor = ImVec4(r, g, b, 1.0f);
                            }
                            else {

                                titleColor = main_color;
                            }

                            ImGui::Dummy(ImVec2(0, 8.0f));

                            float borderMargin = 15.0f;
                            float sepWidth = ImGui::GetWindowWidth() - (2 * borderMargin);
                            ImVec2 sepStartPos = ImGui::GetCursorScreenPos();
                            sepStartPos.x = ImGui::GetWindowPos().x + borderMargin;

                            ImGui::GetWindowDrawList()->AddLine(
                                sepStartPos,
                                ImVec2(sepStartPos.x + sepWidth, sepStartPos.y),
                                ImGui::ColorConvertFloat4ToU32(titleColor),
                                1.0f
                            );

                            ImGui::Dummy(ImVec2(0, 5.0f));

                            DrawAlignedSeparator("Opus Gain", rgbModeEnabled);

                            DrawSlider("Gain", &Gain, 1.0f, 90.0f, "On dB checker its ~20dB");
                            DrawSlider("Rage Gain", &ExpGain, 1.0f, 120.0f, "On dB checker its ~60-65dB");
                            DrawSlider("vUnits Gain", &VunitsGain, 1.0f, 100000000000.0f, "Increase = more clear audio");

                            float encoderControlWidth = ImGui::GetWindowWidth();
                            float encoderContentWidth = encoderControlWidth * 0.8f;
                            float encoderLeftMargin = (encoderControlWidth - encoderContentWidth) * 0.5f;

                            ImGui::SetCursorPosX(encoderLeftMargin + encoderContentWidth / 2 - 60);
                            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 3));
                            if (ImGui::Checkbox("Energy", &energyEnabled)) {

                                if (energyEnabled) {

                                    energyValue = 510000.0f;
                                }
                            }
                            ImGui::PopStyleVar();

                            if (energyEnabled) {

                                DrawSlider("Energy Value", &energyValue, 100000.0f, 1000000.0f, "Energy effect level");
                            }

                            DrawAlignedSeparator("Opus Encoder", rgbModeEnabled);

                            DrawSlider("Bitrate", &bitrateValue, 16000.0f, 510000.0f, "Bitrate change (higher = better quality but more bandwidth)");

                            DrawAlignedSeparator("Signal Type", rgbModeEnabled);

                            float signalComboWidth = 240.0f;
                            float signalCenterPosX = (encoderControlWidth - signalComboWidth) * 0.5f;
                            ImGui::SetCursorPosX(signalCenterPosX);

                            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
                            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));

                            ImVec4 signalComboColor;
                            if (rgbModeEnabled) {
                                float timeOffset = time_since_start * rgbCycleSpeed + 0.5f;
                                signalComboColor = ImVec4(
                                    0.5f + 0.5f * sinf(timeOffset + 4.0f * MY_PI / 3.0f),
                                    0.5f + 0.5f * sinf(timeOffset + 2.0f * MY_PI / 3.0f),
                                    0.5f + 0.5f * sinf(timeOffset),
                                    0.7f
                                );
                            }
                            else {
                                float pulse = GetPulsatingValue(1.0f, 0.7f, 0.8f);
                                signalComboColor = ImVec4(main_color.x * pulse, main_color.y * pulse, main_color.z * pulse, 0.7f);
                            }

                            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(signalComboColor.x, signalComboColor.y, signalComboColor.z, 0.8f));
                            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(signalComboColor.x, signalComboColor.y, signalComboColor.z, 0.9f));
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 0.95f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.25f, 0.9f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.35f, 0.35f, 0.35f, 0.95f));
                            ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.12f, 0.12f, 0.12f, 0.98f));
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
                            ImGui::PushItemWidth(signalComboWidth);

                            if (ImGui::BeginCombo("##SignalType", opusSignalTypes[opusSignalType])) {
                                for (int i = 0; i < IM_ARRAYSIZE(opusSignalTypes); i++) {
                                    const bool is_selected = (opusSignalType == i);
                                    if (ImGui::Selectable(opusSignalTypes[i], is_selected)) {
                                        opusSignalType = i;
                                    }
                                    if (is_selected) {
                                        ImGui::SetItemDefaultFocus();
                                    }
                                }
                                ImGui::EndCombo();
                            }

                            if (ImGui::IsItemHovered()) {
                                ImGui::BeginTooltip();
                                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 25.0f);
                                ImGui::TextUnformatted("Select audio content type for optimal encoding:");
                                ImGui::Bullet(); ImGui::TextUnformatted("Auto: Automatically detect content type");
                                ImGui::Bullet(); ImGui::TextUnformatted("Voice: Optimize for speech (better clarity)");
                                ImGui::Bullet(); ImGui::TextUnformatted("Music: Optimize for music (better dynamics)");
                                ImGui::PopTextWrapPos();
                                ImGui::EndTooltip();
                            }

                            ImGui::PopItemWidth();
                            ImGui::PopStyleColor(7);
                            ImGui::PopStyleVar(2);

                            ImGui::Spacing();

                            DrawAlignedSeparator("Encoding Method", rgbModeEnabled);

                            float methodComboWidth = 240.0f;
                            float methodCenterPosX = (encoderControlWidth - methodComboWidth) * 0.5f;
                            ImGui::SetCursorPosX(methodCenterPosX);

                            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
                            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));

                            ImVec4 methodComboColor;
                            if (rgbModeEnabled) {
                                float timeOffset = time_since_start * rgbCycleSpeed + 1.0f;
                                methodComboColor = ImVec4(
                                    0.5f + 0.5f * sinf(timeOffset),
                                    0.5f + 0.5f * sinf(timeOffset + 2.0f * MY_PI / 3.0f),
                                    0.5f + 0.5f * sinf(timeOffset + 4.0f * MY_PI / 3.0f),
                                    0.7f
                                );
                            }
                            else {
                                float pulse = GetPulsatingValue(1.0f, 0.7f, 0.8f);
                                methodComboColor = ImVec4(main_color.x * pulse, main_color.y * pulse, main_color.z * pulse, 0.7f);
                            }

                            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(methodComboColor.x, methodComboColor.y, methodComboColor.z, 0.8f));
                            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(methodComboColor.x, methodComboColor.y, methodComboColor.z, 0.9f));
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 0.95f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.25f, 0.9f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.35f, 0.35f, 0.35f, 0.95f));
                            ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.12f, 0.12f, 0.12f, 0.98f));
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
                            ImGui::PushItemWidth(methodComboWidth);

                            if (ImGui::BeginCombo("##EncodingMethod", opusEncodingMethods[opusEncodingMethod])) {
                                for (int i = 0; i < IM_ARRAYSIZE(opusEncodingMethods); i++) {
                                    const bool is_selected = (opusEncodingMethod == i);
                                    if (ImGui::Selectable(opusEncodingMethods[i], is_selected)) {
                                        opusEncodingMethod = i;
                                    }
                                    if (is_selected) {
                                        ImGui::SetItemDefaultFocus();
                                    }
                                }
                                ImGui::EndCombo();
                            }

                            if (ImGui::IsItemHovered()) {
                                ImGui::BeginTooltip();
                                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 25.0f);
                                ImGui::TextUnformatted("Select Opus encoding method:");
                                ImGui::Bullet(); ImGui::TextUnformatted("VBR (Variable): Best quality, variable bitrate");
                                ImGui::Bullet(); ImGui::TextUnformatted("CBR (Constant): Fixed bitrate, consistent bandwidth");
                                ImGui::Bullet(); ImGui::TextUnformatted("CVBR (Constrained): Balanced quality with bitrate constraints");
                                ImGui::PopTextWrapPos();
                                ImGui::EndTooltip();
                            }

                            ImGui::PopItemWidth();
                            ImGui::PopStyleColor(7);
                            ImGui::PopStyleVar(2);

                            ImGui::Spacing();

                            DrawAlignedSeparator("Channel Mode", rgbModeEnabled);

                            float comboWidth = 240.0f;
                            float centerPosX = (encoderControlWidth - comboWidth) * 0.5f;
                            ImGui::SetCursorPosX(centerPosX);

                            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
                            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));

                            ImVec4 channelComboColor;
                            if (rgbModeEnabled) {
                                float timeOffset = time_since_start * rgbCycleSpeed;
                                channelComboColor = ImVec4(
                                    0.5f + 0.5f * sinf(timeOffset),
                                    0.5f + 0.5f * sinf(timeOffset + 2.0f * MY_PI / 3.0f),
                                    0.5f + 0.5f * sinf(timeOffset + 4.0f * MY_PI / 3.0f),
                                    0.7f
                                );
                            }
                            else {
                                float pulse = GetPulsatingValue(1.0f, 0.7f, 0.8f);
                                channelComboColor = ImVec4(main_color.x * pulse, main_color.y * pulse, main_color.z * pulse, 0.7f);
                            }

                            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(channelComboColor.x, channelComboColor.y, channelComboColor.z, 0.8f));
                            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(channelComboColor.x, channelComboColor.y, channelComboColor.z, 0.9f));
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 0.95f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.25f, 0.9f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.35f, 0.35f, 0.35f, 0.95f));
                            ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.12f, 0.12f, 0.12f, 0.98f));
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
                            ImGui::PushItemWidth(comboWidth);

                            int tempChannelMode = audioChannelMode;

                            const char* prefix = audioChannelMode == 0 ? "[M] " : "[S] ";
                            const char* display_names[2] = { "Mono (1 Channel)", "Stereo (2 Channels)" };

                            char channelModeLabel[64];
                            snprintf(channelModeLabel, sizeof(channelModeLabel), "%s%s", prefix, display_names[tempChannelMode]);
                            if (ImGui::BeginCombo("##ChannelModeCombo", channelModeLabel)) {
                                for (int i = 0; i < IM_ARRAYSIZE(display_names); i++) {
                                    const bool is_selected = (tempChannelMode == i);
                                    const char* item_prefix = i == 0 ? "[M] " : "[S] ";

                                    char selectableLabel[64];
                                    snprintf(selectableLabel, sizeof(selectableLabel), "%s%s", item_prefix, display_names[i]);
                                    if (ImGui::Selectable(selectableLabel, is_selected)) {
                                        tempChannelMode = i;
                                        audioChannelMode = tempChannelMode;
                                    }

                                    if (is_selected)
                                        ImGui::SetItemDefaultFocus();
                                }
                                ImGui::EndCombo();
                            }

                            channels = (audioChannelMode == 0) ? 1 : 2;

                            if (ImGui::IsItemHovered()) {
                                ImGui::BeginTooltip();
                                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 25.0f);
                                ImGui::TextUnformatted("Select audio channel configuration:");
                                ImGui::Bullet(); ImGui::TextUnformatted("Mono: Single channel");
                                ImGui::Bullet(); ImGui::TextUnformatted("Stereo: Dual channels");
                                ImGui::PopTextWrapPos();
                                ImGui::EndTooltip();
                            }

                            ImGui::PopItemWidth();
                            ImGui::PopStyleColor(7);
                            ImGui::PopStyleVar(2);

                            ImGui::Spacing();
                            float comboWidthFrame = 240.0f;
                            float centerPosXFrame = (encoderControlWidth - comboWidthFrame) * 0.5f;
                            ImGui::SetCursorPosX(centerPosXFrame);
                            ImGui::PushItemWidth(comboWidthFrame);

                            const char* frameLabels[6] = { "120 (2.5ms)", "240 (5ms)", "480 (10ms)", "960 (20ms)", "1920 (40ms)", "2880 (60ms)" };
                            const int frameSizes[6] = { 120, 240, 480, 960, 1920, 2880 };

                            char frameComboLabel[32];
                            snprintf(frameComboLabel, sizeof(frameComboLabel), "%s", frameLabels[opusFrameSizeIdx]);
                            if (ImGui::BeginCombo("##OpusFrameSizeCombo", frameComboLabel)) {
                                for (int i = 0; i < 6; ++i) {
                                    bool is_selected = (opusFrameSizeIdx == i);
                                    if (ImGui::Selectable(frameLabels[i], is_selected)) {
                                        opusFrameSizeIdx = i;
                                        opusFrameSize = frameSizes[i];
                                    }
                                    if (is_selected)
                                        ImGui::SetItemDefaultFocus();
                                }
                                ImGui::EndCombo();
                            }
                            opusFrameSize = frameSizes[opusFrameSizeIdx];
                            ImGui::PopItemWidth();
                            if (ImGui::IsItemHovered()) {
                                ImGui::BeginTooltip();
                                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 25.0f);
                                ImGui::TextUnformatted(
                                    "Select the Opus frame size for encoding.\n"
                                    "- 120: 2.5ms, ultra-low latency, lowest quality\n"
                                    "- 240: 5ms, very low latency\n"
                                    "- 480: 10ms, low latency\n"
                                    "- 960: 20ms, standard (recommended)\n"
                                    "- 1920: 40ms, higher quality, higher latency\n"
                                    "- 2880: 60ms, maximum quality, highest latency"
                                );
                                ImGui::PopTextWrapPos();
                                ImGui::EndTooltip();
                            }

                            {
                                ImGui::Spacing();
                                float comboWidthPacket = comboWidthFrame;
                                float centerPosXPacket = centerPosXFrame;
                                ImGui::SetCursorPosX(centerPosXPacket);
                                ImGui::PushItemWidth(comboWidthPacket);

                                static int opusPacketUnit = 0;
                                static int opusPacketBytes = 400;
                                static int opusPacketMs = 20;
                                static float opusPacketMult = 1.0f;
                                static const char* unitLabels[] = { "Bytes", "Milliseconds", "Multiple of Frame" };
                                static int lastUnit = 0;
                                int sampleRate = 48000;
                                int channels = 2;
                                int frameSize = opusFrameSize;
                                int frameSamples = frameSize * channels;
                                int bytesPerSample = 2;

                                int minBytes = frameSamples * bytesPerSample;
                                int maxBytes = 20000;
                                int minMs = int(1000.0f * (float)frameSamples / (sampleRate * channels));
                                int maxMs = 500;
                                float minMult = 1.0f;
                                float maxMult = 40.0f;

                                auto clamp = [](auto val, auto minv, auto maxv) { return val < minv ? minv : (val > maxv ? maxv : val); };
                                opusPacketBytes = clamp(opusPacketBytes, minBytes, maxBytes);
                                opusPacketMs = clamp(opusPacketMs, minMs, maxMs);
                                opusPacketMult = clamp(opusPacketMult, minMult, maxMult);

                                auto update_from_unit = [&]() {
                                    if (opusPacketUnit == 0) {
                                        int totalSamples = opusPacketBytes / bytesPerSample;
                                        opusPacketMs = int((float)totalSamples / (sampleRate * channels) * 1000.0f);
                                        opusPacketMult = (float)totalSamples / (float)frameSamples;
                                    }
                                    else if (opusPacketUnit == 1) {
                                        int totalSamples = (opusPacketMs * sampleRate * channels) / 1000;
                                        opusPacketBytes = totalSamples * bytesPerSample;
                                        opusPacketMult = (float)totalSamples / (float)frameSamples;
                                    }
                                    else {
                                        int totalSamples = int(opusPacketMult * frameSamples);
                                        opusPacketBytes = totalSamples * bytesPerSample;
                                        opusPacketMs = int((float)totalSamples / (sampleRate * channels) * 1000.0f);
                                    }

                                    opusPacketBytes = clamp(opusPacketBytes, minBytes, maxBytes);
                                    opusPacketMs = clamp(opusPacketMs, minMs, maxMs);
                                    opusPacketMult = clamp(opusPacketMult, minMult, maxMult);
                                    };
                                if (frameSize != frameSizes[opusFrameSizeIdx]) {
                                    frameSize = frameSizes[opusFrameSizeIdx];
                                    frameSamples = frameSize * channels;
                                    update_from_unit();
                                }
                                if (opusPacketUnit != lastUnit) {
                                    lastUnit = opusPacketUnit;
                                    update_from_unit();
                                }

                                ImVec4 colorSafe = ImVec4(0.2f, 1.0f, 0.2f, 1.0f);
                                ImVec4 colorWarn = ImVec4(1.0f, 1.0f, 0.2f, 1.0f);
                                ImVec4 colorRisk = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
                                ImVec4 sliderColor;
                                const char* warnMsg = nullptr;
                                if (opusPacketMs < 10 || opusPacketBytes < 200) {
                                    sliderColor = colorRisk;
                                    warnMsg = "Warning: Very low packet size! May cause choppy audio and high overhead.";
                                }
                                else if (opusPacketMs > 120 || opusPacketBytes > 4000) {
                                    sliderColor = colorRisk;
                                    warnMsg = "Warning: Very large packet size! May cause high latency or Opus errors.";
                                }
                                else if (opusPacketMs < 20 || opusPacketBytes < 400) {
                                    sliderColor = colorWarn;
                                    warnMsg = "Low packet size: Lower latency but less efficient.";
                                }
                                else if (opusPacketMs > 60 || opusPacketBytes > 2000) {
                                    sliderColor = colorWarn;
                                    warnMsg = "Large packet: Higher latency but more efficient.";
                                }
                                else {
                                    sliderColor = colorSafe;
                                }
                                ImGui::PushStyleColor(ImGuiCol_SliderGrab, sliderColor);
                                ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, sliderColor);
                                ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(sliderColor.x * 0.5f, sliderColor.y * 0.5f, sliderColor.z * 0.5f, 1.0f));

                                ImGui::SetCursorPosX(centerPosXPacket);
                                ImGui::Combo("Packet Size Unit##PacketUnit", &opusPacketUnit, unitLabels, IM_ARRAYSIZE(unitLabels));

                                ImGui::SetCursorPosX(centerPosXPacket);
                                if (opusPacketUnit == 0) {
                                    ImGui::SliderInt("Packet Size (Bytes)##Bytes", &opusPacketBytes, minBytes, maxBytes);
                                    if (ImGui::IsItemEdited()) update_from_unit();
                                }
                                else if (opusPacketUnit == 1) {
                                    ImGui::SliderInt("Packet Size (ms)##Ms", &opusPacketMs, minMs, maxMs);
                                    if (ImGui::IsItemEdited()) update_from_unit();
                                }
                                else {
                                    ImGui::SliderFloat("Packet Size (x Frame)##Mult", &opusPacketMult, minMult, maxMult, "%.2fx");
                                    if (ImGui::IsItemEdited()) update_from_unit();
                                }

                                if (warnMsg) {
                                    ImGui::SetCursorPosX(centerPosXPacket);
                                    ImGui::TextColored(sliderColor, warnMsg);
                                }

                                if (ImGui::IsItemHovered()) {
                                    ImGui::BeginTooltip();
                                    ImGui::Text("%d bytes | %d ms | %.2fx frame", opusPacketBytes, opusPacketMs, opusPacketMult);
                                    ImGui::Text("Latency: %.1f ms", (float)opusPacketMs);
                                    if (warnMsg) {
                                        ImGui::TextColored(sliderColor, warnMsg);
                                    }
                                    else if (opusPacketMs <= 20) {
                                        ImGui::Text("Lower latency, higher overhead");
                                    }
                                    else {
                                        ImGui::Text("Higher latency, more efficient");
                                    }
                                    ImGui::EndTooltip();
                                }

                                ImGui::SetCursorPosX(centerPosXPacket);
                                ImGui::Text("%d bytes  |  %d ms  |  %.2fx frame", opusPacketBytes, opusPacketMs, opusPacketMult);
                                ImGui::PopStyleColor(3);
                                ImGui::PopItemWidth();
                            }

                            ImGui::Spacing();
                            ImGui::Spacing();

                            DrawAlignedSeparator("Effects", rgbModeEnabled);
                            ImGui::SetCursorPosX(encoderLeftMargin + encoderContentWidth / 2 - 80);
                            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 3));
                            if (ImGui::Checkbox("Enable Reverb", &reverbEnabled)) {

                                if (reverbEnabled) {
                                    reverbProcessor.init(48000, 2);
                                }
                                else {
                                    reverbProcessor.mute();
                                }
                            }
                            ImGui::PopStyleVar();

                            if (reverbEnabled) {
                                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12, 10));

                                DrawSlider("Mix", &reverbMix, 0.0f, 1.0f, "Controls the wet/dry effect");

                                DrawSlider("Room Size", &reverbSize, 0.0f, 1.0f, "Controls the apparent size");

                                DrawSlider("Damping", &reverbDamping, 0.0f, 1.0f, "Controls the absorption of high frequencies");

                                DrawSlider("Width", &reverbWidth, 0.0f, 1.0f, "Controls the stereo spread of the reverb effect");

                                ImGui::PopStyleVar();
                            }

                            if (audioChannelMode == 0) {

                                const char* panMsg = "Panning disabled in mono mode";
                                ImVec2 textSize = ImGui::CalcTextSize(panMsg);
                                float windowWidth = ImGui::GetWindowWidth();
                                ImGui::SetCursorPosX((windowWidth - textSize.x) * 0.5f);
                                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), panMsg);

                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));

                                ImVec4 greyColor = ImVec4(0.7f, 0.7f, 0.7f, 0.8f);

                                DrawAlignedSeparator("Panning", false, greyColor);
                                ImGui::PopStyleColor();
                            }
                            else {
                                DrawAlignedSeparator("Panning", rgbModeEnabled);
                            }

                            if (audioChannelMode == 0) {

                                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);

                                ImVec4 gray = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
                                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.35f, 0.35f, 0.35f, 1.0f));
                                ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
                                ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.45f, 0.45f, 0.45f, 1.0f));
                                ImGui::PushStyleColor(ImGuiCol_SliderGrab, gray);
                                ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));

                                bool prevRgb = rgbModeEnabled;
                                ImVec4 prevMain = main_color;
                                rgbModeEnabled = false;
                                main_color = gray;
                                AnimatedPanningSlider("Balance", &panningValue, -10.0f, 10.0f);
                                rgbModeEnabled = prevRgb;
                                main_color = prevMain;
                                ImGui::PopStyleColor(5);
                                ImGui::PopStyleVar();

                                if (panningValue != 0.0f) {
                                    panningValue = 0.0f;
                                }
                            }
                            else {

                                AnimatedPanningSlider("Balance", &panningValue, -10.0f, 10.0f);
                            }

                            ImGui::Spacing();

                            float checkboxWidth = 120;
                            float spacing = 20;
                            float totalWidth = checkboxWidth * 2 + spacing;
                            float leftPos = (encoderControlWidth - totalWidth) * 0.5f;

                            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 3));

                            if (audioChannelMode == 0) {

                                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);

                                ImGui::SetCursorPosX(leftPos);
                                ImGui::Checkbox("In Head Left", &inHeadLeft);

                                ImGui::SameLine(leftPos + checkboxWidth + spacing);
                                ImGui::Checkbox("In Head Right", &inHeadRight);

                                ImGui::PopStyleVar();

                                if (inHeadLeft || inHeadRight) {
                                    inHeadLeft = false;
                                    inHeadRight = false;
                                }
                            }
                            else {

                                ImGui::SetCursorPosX(leftPos);
                                if (ImGui::Checkbox("In Head Left", &inHeadLeft)) {

                                }

                                ImGui::SameLine(leftPos + checkboxWidth + spacing);
                                if (ImGui::Checkbox("In Head Right", &inHeadRight)) {

                                }
                            }
                            ImGui::PopStyleVar();

                            ImGui::EndTabItem();
                        }
                        if (ImGui::BeginTabItem("EQ")) {

                            ImVec4 titleColor;
                            if (rgbModeEnabled) {

                                float timeOffset = time_since_start * rgbCycleSpeed;
                                float r = 0.5f + 0.5f * sinf(timeOffset);
                                float g = 0.5f + 0.5f * sinf(timeOffset + 2.0f * MY_PI / 3.0f);
                                float b = 0.5f + 0.5f * sinf(timeOffset + 4.0f * MY_PI / 3.0f);
                                titleColor = ImVec4(r, g, b, 1.0f);
                            }
                            else {

                                titleColor = main_color;
                            }

                            ImGui::Dummy(ImVec2(0, 8.0f));

                            float borderMargin = 15.0f;
                            float sepWidth = ImGui::GetWindowWidth() - (2 * borderMargin);
                            ImVec2 sepStartPos = ImGui::GetCursorScreenPos();
                            sepStartPos.x = ImGui::GetWindowPos().x + borderMargin;

                            ImGui::GetWindowDrawList()->AddLine(
                                sepStartPos,
                                ImVec2(sepStartPos.x + sepWidth, sepStartPos.y),
                                ImGui::ColorConvertFloat4ToU32(titleColor),
                                1.0f
                            );

                            ImGui::Dummy(ImVec2(0, 5.0f));

                            DrawAlignedSeparator("Equalizer Settings", rgbModeEnabled);

                            float eqControlWidth = ImGui::GetWindowWidth();
                            float eqContentWidth = eqControlWidth * 0.8f;
                            float eqLeftMargin = (eqControlWidth - eqContentWidth) * 0.5f;

                            ImGui::SetCursorPosX(eqLeftMargin);
                            DrawSlider("Bass", &bassEQ, 0.0f, 70.0f, "Boosts low frequencies (50-250Hz)");

                            ImGui::SetCursorPosX(eqLeftMargin);
                            DrawSlider("Pierce", &midEQ, 0.0f, 70.0f, "Boosts piercing frequencies (250-4000Hz)");

                            ImGui::SetCursorPosX(eqLeftMargin);
                            DrawSlider("Wide", &highEQ, 0.0f, 70.0f, "Boosts wide frequencies (4000-20000Hz)");

                            ImGui::EndTabItem();
                        }
                        if (ImGui::BeginTabItem("Decoder")) {

                            ImVec4 titleColor;
                            if (rgbModeEnabled) {

                                float timeOffset = time_since_start * rgbCycleSpeed;
                                float r = 0.5f + 0.5f * sinf(timeOffset);
                                float g = 0.5f + 0.5f * sinf(timeOffset + 2.0f * MY_PI / 3.0f);
                                float b = 0.5f + 0.5f * sinf(timeOffset + 4.0f * MY_PI / 3.0f);
                                titleColor = ImVec4(r, g, b, 1.0f);
                            }
                            else {

                                titleColor = main_color;
                            }

                            ImGui::Dummy(ImVec2(0, 8.0f));

                            float borderMargin = 15.0f;
                            float sepWidth = ImGui::GetWindowWidth() - (2 * borderMargin);
                            ImVec2 sepStartPos = ImGui::GetCursorScreenPos();
                            sepStartPos.x = ImGui::GetWindowPos().x + borderMargin;

                            ImGui::GetWindowDrawList()->AddLine(
                                sepStartPos,
                                ImVec2(sepStartPos.x + sepWidth, sepStartPos.y),
                                ImGui::ColorConvertFloat4ToU32(titleColor),
                                1.0f
                            );

                            ImGui::Dummy(ImVec2(0, 5.0f));

                            DrawAlignedSeparator("Decoding Options", rgbModeEnabled);

                            float decoderWindowWidth = ImGui::GetWindowWidth();
                            float decoderContentWidth = decoderWindowWidth * 0.8f;
                            float decoderLeftMargin = (decoderWindowWidth - decoderContentWidth) * 0.5f;

                            ImGui::SetCursorPosX(decoderLeftMargin + decoderContentWidth / 2 - 60);
                            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 3));
                            ImGui::Checkbox("dB Checker", &Spoofing::Hider);
                            ImGui::PopStyleVar();

                            DrawAlignedSeparator("", rgbModeEnabled);

                            ImGui::EndTabItem();
                        }

                        if (ImGui::BeginTabItem("Settings")) {

                            ImGui::Text("Toggle Hotkey: %s", GetKeyName(TOGGLE_HOTKEY));

                            static bool isChangingHotkey = false;
                            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

                            if (ImGui::Button(isChangingHotkey ? "Press a key..." : "Change Hotkey", ImVec2(150, 30))) {
                                isChangingHotkey = true;
                            }

                            ImGui::PopStyleVar();

                            if (isChangingHotkey) {
                                for (int key = 1; key < 256; key++) {
                                    if (GetAsyncKeyState(key) & 0x8000) {

                                        ChangeHotkey(key);
                                        isChangingHotkey = false;
                                        break;
                                    }
                                }
                            }

                            DrawAlignedSeparator("Spoofing Options", rgbModeEnabled);

                            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 3));
                            if (ImGui::Checkbox("Streamproof", &Spoofing::Hider)) {
                                SetWindowDisplayAffinity(hwnd, Spoofing::Hider ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE);
                            }
                            ImGui::PopStyleVar();
                            DrawAlignedSeparator("Color Settings", rgbModeEnabled);

                            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 3));
                            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.65f);

                            float currentWidth = ImGui::GetContentRegionAvail().x;
                            ImGui::Checkbox("RGB Toggle", &rgbModeEnabled);

                            ImGui::Spacing();
                            ImGui::Separator();
                            ImGui::Spacing();

                            if (!rgbModeEnabled) {

                                ImGuiColorEditFlags colorFlags = ImGuiColorEditFlags_NoInputs |
                                    ImGuiColorEditFlags_PickerHueBar;

                                ImGui::Spacing();
                                ImGui::Separator();
                                ImGui::Spacing();

                                static bool windowTopMost = utilities::globals::windowTopMost;
                                if (ImGui::Checkbox("Keep window on top", &windowTopMost)) {
                                    utilities::globals::windowTopMost = windowTopMost;

                                    if (IsWindowVisible(hwnd)) {

                                        WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
                                        GetWindowPlacement(hwnd, &wp);

                                        SetWindowPos(
                                            hwnd,
                                            windowTopMost ? HWND_TOPMOST : HWND_NOTOPMOST,
                                            0, 0, 0, 0,
                                            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW
                                        );

                                        SetWindowPlacement(hwnd, &wp);
                                    }
                                }
                                if (ImGui::IsItemHovered()) {
                                    ImGui::SetTooltip("Keep the window on top of other windows when enabled");
                                }

                                ImGui::Spacing();
                                ImGui::Separator();
                                ImGui::Spacing();

                                ImGui::Text("Choose colors for UI elements:");
                                ImGui::Spacing();

                                if (ImGui::ColorEdit3("Accent Color", (float*)&main_color, colorFlags)) {

                                    style.Colors[ImGuiCol_ScrollbarGrab] = main_color;
                                    style.Colors[ImGuiCol_CheckMark] = main_color;
                                    style.Colors[ImGuiCol_SliderGrab] = main_color;
                                    style.Colors[ImGuiCol_TabActive] = main_color;
                                    style.Colors[ImGuiCol_TabHovered] = main_color;

                                    style.Colors[ImGuiCol_FrameBg] = ImVec4(main_color.x * 0.15f, main_color.y * 0.15f, main_color.z * 0.15f, 0.8f);
                                    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(main_color.x * 0.20f, main_color.y * 0.20f, main_color.z * 0.20f, 0.9f);
                                    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(main_color.x * 0.25f, main_color.y * 0.25f, main_color.z * 0.25f, 1.0f);
                                }

                                if (ImGui::ColorEdit3("Background Color", (float*)&clear_color, colorFlags)) {

                                    style.Colors[ImGuiCol_WindowBg] = clear_color;
                                    style.Colors[ImGuiCol_ChildBg] = clear_color;
                                    style.Colors[ImGuiCol_PopupBg] = clear_color;
                                }
                            }

                            ImGui::PopItemWidth();
                            ImGui::PopStyleVar();
                            ImGui::EndTabItem();
                        }

                        if (ImGui::BeginTabItem("Infos")) {

                            DrawAlignedSeparator("Hook Information", rgbModeEnabled);

                            std::string processInfo = GetProcessName();

                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.00f));

                            size_t pos = 0;
                            size_t prevPos = 0;
                            std::string line;

                            while ((pos = processInfo.find('\n', prevPos)) != std::string::npos) {
                                line = processInfo.substr(prevPos, pos - prevPos);

                                if (line.find("Hooked:") != std::string::npos) {
                                    ImVec4 accentColor = rgbModeEnabled ?
                                        ImVec4(
                                            0.5f + 0.5f * sinf(time_since_start * rgbCycleSpeed),
                                            0.5f + 0.5f * sinf(time_since_start * rgbCycleSpeed + 2.0f * MY_PI / 3.0f),
                                            0.5f + 0.5f * sinf(time_since_start * rgbCycleSpeed + 4.0f * MY_PI / 3.0f),
                                            1.0f
                                        ) : main_color;
                                    ImGui::PushStyleColor(ImGuiCol_Text, accentColor);
                                    ImGui::Text("%s", line.c_str());
                                    ImGui::PopStyleColor();
                                }
                                else {
                                    ImGui::Text("%s", line.c_str());
                                }
                                prevPos = pos + 1;
                            }

                            if (prevPos < processInfo.length()) {
                                line = processInfo.substr(prevPos);
                                if (line.find("Hooked:") != std::string::npos) {
                                    ImVec4 accentColor = rgbModeEnabled ?
                                        ImVec4(
                                            0.5f + 0.5f * sinf(time_since_start * rgbCycleSpeed),
                                            0.5f + 0.5f * sinf(time_since_start * rgbCycleSpeed + 2.0f * MY_PI / 3.0f),
                                            0.5f + 0.5f * sinf(time_since_start * rgbCycleSpeed + 4.0f * MY_PI / 3.0f),
                                            1.0f
                                        ) : main_color;
                                    ImGui::PushStyleColor(ImGuiCol_Text, accentColor);
                                    ImGui::Text("%s", line.c_str());
                                    ImGui::PopStyleColor();
                                }
                                else {
                                    ImGui::Text("%s", line.c_str());
                                }
                            }
                            ImGui::PopStyleColor();

                            DrawAlignedSeparator("Process Handler & Injection", rgbModeEnabled);
                            ImGui::Spacing();

                            ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "Process Handler Status:");
                            bool handlerConnected = true;
                            ImGui::SameLine();
                            ImGui::TextColored(handlerConnected ? ImVec4(0.2f, 1.0f, 0.2f, 1.0f) : ImVec4(1.0f, 0.3f, 0.3f, 1.0f), handlerConnected ? "Connected" : "Not Connected");
                            ImGui::Separator();
                            ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "Handler PID:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.95f, 0.95f, 0.95f, 1.0f), std::to_string(GetCurrentProcessId()).c_str());
                            ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "Handler Threads:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.95f, 0.95f, 0.95f, 1.0f), std::to_string(GetCurrentProcessThreadCount()).c_str());
                            ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "Handler Uptime:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.95f, 0.95f, 0.95f, 1.0f), GetCurrentProcessUptime().c_str());
                            ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "Handler Memory:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.95f, 0.95f, 0.95f, 1.0f), GetCurrentProcessMemoryUsage().c_str());

                            ImGui::Separator();

                            ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "Injection Source:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), GetInjectionSource().c_str());
                            ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "Source PID:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), GetSourcePID().c_str());
                            ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "Injection Method:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), GetInjectionMethod().c_str());
                            ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "Injection Time:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), GetInjectionTime().c_str());
                            ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "Target Module Base:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), GetTargetModuleBase().c_str());
                            ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "Integrity Level:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), GetIntegrityLevel().c_str());
                            ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "Session ID:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), GetSessionId().c_str());
                            ImGui::Spacing();

                            DrawAlignedSeparator("Audio Encoder/Decoder Status", rgbModeEnabled);
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "Opus Version:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.95f, 0.95f, 0.95f, 1.0f), "1.3.1");
                            ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "Audio Device:"); ImGui::SameLine();
                            ImGui::TextColored(ImVec4(0.4f, 0.95f, 0.7f, 1.0f), GetDefaultAudioDeviceName().c_str());
                            ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "Sample Rate:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.95f, 0.95f, 0.95f, 1.0f), GetAudioSampleRate().c_str());
                            ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "Channels:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.95f, 0.95f, 0.95f, 1.0f), GetAudioChannels().c_str());
                            ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "Buffer Size:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.95f, 0.95f, 0.95f, 1.0f), GetAudioBufferSize().c_str());
                            ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "Last Error:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), GetAudioLastError().c_str());
                            ImGui::Spacing();

                            DrawAlignedSeparator("Environment", rgbModeEnabled);
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "User:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.95f, 0.95f, 0.95f, 1.0f), GetCurrentUserName().c_str());
                            ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "OS:"); ImGui::SameLine();
                            ImGui::TextColored(ImVec4(0.4f, 0.95f, 0.7f, 1.0f), GetWindowsVersionString().c_str());
                            ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "Build Date:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.95f, 0.95f, 0.95f, 1.0f), GetBuildDateTime().c_str());
                            ImGui::Spacing();

                            DrawAlignedSeparator("Version Information", rgbModeEnabled);

                            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.06f, 0.8f));
                            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
                            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 4));
                            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 2));

                            float windowWidth = ImGui::GetWindowWidth();
                            float creditWidth = windowWidth * 0.5f;
                            ImGui::SetCursorPosX((windowWidth - creditWidth) * 0.5f);

                            ImGui::BeginChild("CreditsPanel", ImVec2(creditWidth, 160.0f), true);

                            ImVec2 panelMin = ImGui::GetWindowPos();
                            ImVec2 panelMax = ImVec2(panelMin.x + creditWidth, panelMin.y + 120.0f);
                            ImVec4 accentColor = rgbModeEnabled ?
                                ImVec4(0.5f + 0.5f * sinf(ImGui::GetTime() * 0.5f),
                                    0.5f + 0.5f * sinf(ImGui::GetTime() * 0.5f + 2.0f),
                                    0.5f + 0.5f * sinf(ImGui::GetTime() * 0.5f + 4.0f), 0.7f) :
                                ImVec4(0.7f, 0.3f, 0.3f, 0.5f);

                            DrawCornerDecorations(panelMin, panelMax, accentColor, 1.0f, 8.0f);

                            ImGui::SetCursorPosX((creditWidth - ImGui::CalcTextSize("Version: 2.6").x) * 0.5f);
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.95f, 1.00f));
                            ImGui::Text("Version: 2.6");
                            ImGui::PopStyleColor();
                            ImGui::Spacing();

                            float dividerWidth = creditWidth * 0.6f;
                            ImGui::SetCursorPosX((creditWidth - dividerWidth) * 0.5f);
                            ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
                            ImGui::Separator();
                            ImGui::PopStyleColor();

                            ImGui::SetCursorPosX((creditWidth - ImGui::CalcTextSize("Made by:").x) * 0.5f);
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.00f));
                            ImGui::Text("Made by:");
                            ImGui::PopStyleColor();

                            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

                            float nameWidth = 0;
                            const char* name = "Ghost of 1337";
                            for (int i = 0; i < strlen(name); i++) {
                                nameWidth += ImGui::CalcTextSize(&name[i], &name[i + 1]).x;
                            }
                            ImGui::SetCursorPosX((creditWidth - nameWidth) * 0.5f);

                            float time = ImGui::GetTime();
                            const char* creditName = "Ghost of 1337";
                            for (int i = 0; i < strlen(creditName); i++) {

                                float t = time * 0.3f + i * 0.05f;
                                float hue = 0.5f + 0.5f * sinf(t);
                                float s = 0.7f + 0.3f * sinf(t * 0.7f);
                                float v = 0.9f + 0.1f * sinf(t * 0.5f);
                                float r, g, b;
                                ImGui::ColorConvertHSVtoRGB(hue, s, v, r, g, b);

                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(r, g, b, 1.0f));
                                char c[2] = { creditName[i], '\0' };
                                ImGui::Text("%s", c);
                                ImGui::SameLine(0, 0);
                                ImGui::PopStyleColor();
                            }
                            ImGui::PopStyleVar();
                            ImGui::NewLine();
                            ImGui::Spacing();

                            ImGui::SetCursorPosX((creditWidth - dividerWidth) * 0.5f);
                            ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
                            ImGui::Separator();
                            ImGui::PopStyleColor();

                            ImGui::SetCursorPosX((creditWidth - ImGui::CalcTextSize("Thanks to:").x) * 0.5f);
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.00f));
                            ImGui::Text("Thanks to:");
                            ImGui::PopStyleColor();
                            ImGui::Spacing();

                            const char* names[] = { "Cryart", "Akee", "Swatted", "Habibi", "Senz" };
                            for (int j = 0; j < 5; j++) {

                                float currentNameWidth = 0;
                                for (int i = 0; i < strlen(names[j]); i++) {
                                    currentNameWidth += ImGui::CalcTextSize(&names[j][i], &names[j][i + 1]).x;
                                }
                                ImGui::SetCursorPosX((creditWidth - currentNameWidth) * 0.5f);

                                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                                for (int i = 0; i < strlen(names[j]); i++) {

                                    float namePhase = j * 0.7f;
                                    float t = time * 0.3f + i * 0.05f + namePhase;
                                    float hue = 0.5f + 0.5f * sinf(t);
                                    float s = 0.7f + 0.3f * sinf(t * 0.7f);
                                    float v = 0.9f + 0.1f * sinf(t * 0.5f);
                                    float r, g, b;
                                    ImGui::ColorConvertHSVtoRGB(hue, s, v, r, g, b);
                                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(r, g, b, 1.0f));
                                    char c[2] = { names[j][i], '\0' };
                                    ImGui::Text("%s", c);
                                    ImGui::SameLine(0, 0);
                                    ImGui::PopStyleColor();
                                }
                                ImGui::PopStyleVar();
                                ImGui::NewLine();
                            }

                            ImGui::EndChild();

                            ImGui::PopStyleVar(3);
                            ImGui::PopStyleColor(1);

                            DrawAlignedSeparator("Hotkeys", rgbModeEnabled);

                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.00f));
                            ImGui::Text("%s - Toggle window visibility", GetKeyName(TOGGLE_HOTKEY));
                            ImGui::PopStyleColor();
                            ImGui::EndTabItem();

                        }

                        ImGui::EndTabBar();

                        StyleTabBar();
                    }

                    ImGuiStyle& style = ImGui::GetStyle();
                    style.TabRounding = 4.0f;

                    ImGui::End();
                    ImGui::EndFrame();
                }

                g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
                g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
                g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

                D3DCOLOR clear_col_dx = D3DCOLOR_RGBA(
                    (int)(clear_color.x * 255.0f),
                    (int)(clear_color.y * 255.0f),
                    (int)(clear_color.z * 255.0f),
                    255
                );
                g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
                if (g_pd3dDevice->BeginScene() >= 0) {
                    if (show_imgui_window) {
                        ImGui::Render();
                        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
                    }
                    g_pd3dDevice->EndScene();
                }

                if (g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr) == D3DERR_DEVICELOST)
                    g_DeviceLost = true;
            }

            UnregisterHotKey(hwnd, 1);
            ImGui_ImplDX9_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
            CleanupDeviceD3D();
            ::DestroyWindow(hwnd);
            ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        }
    }
}

bool CreateDeviceD3D(HWND hWnd) {
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
        return false;

    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

    D3DDISPLAYMODE displayMode;
    if (FAILED(g_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode))) {
        return false;
    }

    g_d3dpp.BackBufferFormat = displayMode.Format;

    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void CleanupDeviceD3D() {
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = nullptr; }
}

void ResetDevice() {
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL) {
        IM_ASSERT(0);
    }
    ImGui_ImplDX9_CreateDeviceObjects();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZING:

        break;
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED) {

        }
        g_ResizeWidth = (UINT)LOWORD(lParam);
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        if ((wParam & 0xfff0) == SC_SIZE)
            return 0;
        if ((wParam & 0xfff0) == SC_CLOSE) {

            ToggleWindowVisibility();
            return 0;
        }
        break;
    case WM_NCHITTEST:

    {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        ScreenToClient(hWnd, &pt);

        float titleBarHeight = 28.0f;

        float closeButtonSize = 22.0f;
        float closeButtonRight = 20.0f;
        float closeButtonTop = 6.0f;

        RECT rect;
        GetClientRect(hWnd, &rect);
        float windowWidth = (float)(rect.right - rect.left);

        const float hitAreaPadding = 8.0f;
        float closeButtonLeft = windowWidth - closeButtonRight - closeButtonSize;

        if (pt.x >= closeButtonLeft - hitAreaPadding &&
            pt.x <= closeButtonLeft + closeButtonSize + hitAreaPadding &&
            pt.y >= closeButtonTop - hitAreaPadding &&
            pt.y <= closeButtonTop + closeButtonSize + hitAreaPadding) {
            return HTCLIENT;
        }

        if (pt.y <= titleBarHeight) {
            return HTCAPTION;
        }
        break;
    }
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

const char* GetKeyName(int key) {
    static char keyName[32] = { 0 };

    switch (key) {
    case VK_F1: return "F1";
    case VK_F2: return "F2";
    case VK_F3: return "F3";
    case VK_F4: return "F4";
    case VK_F5: return "F5";
    case VK_F6: return "F6";
    case VK_F7: return "F7";
    case VK_F8: return "F8";
    case VK_F9: return "F9";
    case VK_F10: return "F10";
    case VK_F11: return "F11";
    case VK_F12: return "F12";
    case VK_NUMPAD0: return "Numpad 0";
    case VK_NUMPAD1: return "Numpad 1";
    case VK_NUMPAD2: return "Numpad 2";
    case VK_NUMPAD3: return "Numpad 3";
    case VK_NUMPAD4: return "Numpad 4";
    case VK_NUMPAD5: return "Numpad 5";
    case VK_NUMPAD6: return "Numpad 6";
    case VK_NUMPAD7: return "Numpad 7";
    case VK_NUMPAD8: return "Numpad 8";
    case VK_NUMPAD9: return "Numpad 9";
    case VK_HOME: return "Home";
    case VK_END: return "End";
    case VK_PRIOR: return "Page Up";
    case VK_NEXT: return "Page Down";
    case VK_INSERT: return "Insert";
    case VK_DELETE: return "Delete";
    default:

        UINT scanCode = MapVirtualKey(key, MAPVK_VK_TO_VSC);
        if (scanCode > 0) {

            BYTE keyboardState[256] = { 0 };
            GetKeyNameTextA(scanCode << 16, keyName, sizeof(keyName));
        }
        else {

            sprintf_s(keyName, sizeof(keyName), "Key %d", key);
        }
        return keyName;
    }
}

void ChangeHotkey(int newKey) {
    if (hwnd) {
        UnregisterHotKey(hwnd, 1);
        TOGGLE_HOTKEY = newKey;
        RegisterHotKey(hwnd, 1, MOD_NOREPEAT, TOGGLE_HOTKEY);
    }
}

void StyleTabBar() {
    ImGuiStyle& style = ImGui::GetStyle();

    float origTabRounding = style.TabRounding;
    float origFramePadding = style.FramePadding.y;
    float origItemSpacing = style.ItemSpacing.y;

    style.TabRounding = 0.0f;
    style.TabBorderSize = 0.0f;
    style.FramePadding.y = 6.0f;
    style.ItemSpacing.y = 0.0f;

    ImVec4 tabColor;
    if (rgbModeEnabled) {
        float timeOffset = time_since_start * rgbCycleSpeed;
        float r = 0.5f + 0.5f * sinf(timeOffset);
        float g = 0.5f + 0.5f * sinf(timeOffset + 2.0f * MY_PI / 3.0f);
        float b = 0.5f + 0.5f * sinf(timeOffset + 4.0f * MY_PI / 3.0f);
        tabColor = ImVec4(r, g, b, 1.0f);
    }
    else {
        tabColor = main_color;
    }

    style.Colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.08f, 0.08f, 0.95f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(tabColor.x * 0.7f, tabColor.y * 0.7f, tabColor.z * 0.7f, 0.95f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(tabColor.x, tabColor.y, tabColor.z, 0.95f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.08f, 0.08f, 0.95f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(tabColor.x * 0.6f, tabColor.y * 0.6f, tabColor.z * 0.6f, 0.95f);

    style.Colors[ImGuiCol_Separator] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();

    float lineY = windowPos.y + ImGui::GetFrameHeight() + style.FramePadding.y * 2.0f;

    float contentWidth = windowSize.x;
    float borderWidth = 2.0f;
    float leftMargin = 12.0f;
    float rightMargin = 12.0f;

    drawList->AddLine(
        ImVec2(windowPos.x + leftMargin, lineY),
        ImVec2(windowPos.x + contentWidth - 16.0f, lineY),
        ImGui::ColorConvertFloat4ToU32(tabColor),
        1.0f
    );

    style.TabRounding = origTabRounding;
    style.FramePadding.y = origFramePadding;
    style.ItemSpacing.y = origItemSpacing;
}

void DrawNestedFrame(const char* title, bool rgbMode) {

    ImVec2 frameStartPos = ImGui::GetCursorScreenPos();

    ImGui::PushID(title);

    ImGui::Spacing();

    ImGui::Indent(10.0f);

    float frameWidth = ImGui::GetContentRegionAvail().x;

    ImVec4 frameBorderColor;
    if (rgbMode) {

        float timeOffset = time_since_start * rgbCycleSpeed;
        float r = 0.5f + 0.5f * sinf(timeOffset);
        float g = 0.5f + 0.5f * sinf(timeOffset + 2.0f * MY_PI / 3.0f);
        float b = 0.5f + 0.5f * sinf(timeOffset + 4.0f * MY_PI / 3.0f);
        frameBorderColor = ImVec4(r, g, b, 0.8f);
    }
    else {

        float pulse = GetPulsatingValue(1.5f, 0.7f, 1.0f);
        frameBorderColor = ImVec4(main_color.x * pulse, main_color.y * pulse, main_color.z * pulse, 0.8f);
    }

    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImVec2(ImGui::GetWindowWidth(), ImGui::GetContentRegionAvail().y + ImGui::GetCursorPosY() + 20.0f);

    ImVec2 frameMin = ImVec2(frameStartPos.x - 5.0f, frameStartPos.y - 2.0f);
    ImVec2 frameMax = ImVec2(frameStartPos.x + frameWidth + 5.0f, windowPos.y + windowSize.y - 5.0f);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec4 bgColor = ImVec4(0.05f, 0.05f, 0.05f, 0.2f);
    drawList->AddRectFilled(
        frameMin,
        frameMax,
        ImGui::ColorConvertFloat4ToU32(bgColor),
        10.0f
    );

    drawList->AddRect(
        frameMin,
        frameMax,
        ImGui::ColorConvertFloat4ToU32(frameBorderColor),
        10.0f,
        ImDrawFlags_RoundCornersAll,
        1.0f
    );

    ImGui::PopID();
}

float AnimateSin(float speed, float min, float max, float phase = 0.0f) {
    float timeOffset = time_since_start * speed;
    return min + (max - min) * (0.5f + 0.5f * sinf(timeOffset + phase));
}

float AnimateExpo(float speed, float min, float max) {
    float t = fmodf(time_since_start * speed, 1.0f);
    float value = t < 0.5f ? 8 * t * t * t * t : 1 - powf(-2 * t + 2, 4) / 2;
    return min + (max - min) * value;
}

float AnimateElastic(float speed, float min, float max) {
    float t = fmodf(time_since_start * speed, 1.0f);
    const float c4 = (2.0f * MY_PI) / 3.0f;
    float factor;
    if (t == 0.0f) {
        factor = 0.0f;
    }
    else if (t == 1.0f) {
        factor = 1.0f;
    }
    else {
        factor = powf(2.0f, -10.0f * t) * sinf((t * 10.0f - 0.75f) * c4) + 1.0f;
    }
    return min + (max - min) * factor;
}

void AnimateBorderCorners(const ImVec2& topLeft, const ImVec2& bottomRight, const ImVec4& color, float thickness = 2.0f) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    float cornerLength = 12.0f;

    float cornerOffset = 4.0f;

    ImVec4 pulsingColor = color;
    pulsingColor.w = GetPulsatingValue(2.0f, 0.5f, 1.0f);
    ImU32 cornerColor = ImGui::ColorConvertFloat4ToU32(pulsingColor);

    drawList->AddLine(
        ImVec2(topLeft.x + cornerOffset, topLeft.y),
        ImVec2(topLeft.x + cornerLength, topLeft.y),
        cornerColor, thickness);
    drawList->AddLine(
        ImVec2(topLeft.x, topLeft.y + cornerOffset),
        ImVec2(topLeft.x, topLeft.y + cornerLength),
        cornerColor, thickness);

    drawList->AddLine(
        ImVec2(bottomRight.x - cornerOffset, topLeft.y),
        ImVec2(bottomRight.x - cornerLength, topLeft.y),
        cornerColor, thickness);
    drawList->AddLine(
        ImVec2(bottomRight.x, topLeft.y + cornerOffset),
        ImVec2(bottomRight.x, topLeft.y + cornerLength),
        cornerColor, thickness);

    drawList->AddLine(
        ImVec2(topLeft.x + cornerOffset, bottomRight.y),
        ImVec2(topLeft.x + cornerLength, bottomRight.y),
        cornerColor, thickness);
    drawList->AddLine(
        ImVec2(topLeft.x, bottomRight.y - cornerOffset),
        ImVec2(topLeft.x, bottomRight.y - cornerLength),
        cornerColor, thickness);

    drawList->AddLine(
        ImVec2(bottomRight.x - cornerOffset, bottomRight.y),
        ImVec2(bottomRight.x - cornerLength, bottomRight.y),
        cornerColor, thickness);
    drawList->AddLine(
        ImVec2(bottomRight.x, bottomRight.y - cornerOffset),
        ImVec2(bottomRight.x, bottomRight.y - cornerLength),
        cornerColor, thickness);
}

bool AnimatedButton(const char* label, const ImVec2& size) {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4 origBtnColor = style.Colors[ImGuiCol_Button];
    ImVec4 origBtnHoverColor = style.Colors[ImGuiCol_ButtonHovered];
    ImVec4 origBtnActiveColor = style.Colors[ImGuiCol_ButtonActive];

    ImVec4 btnColor, btnHoverColor, btnActiveColor;

    if (rgbModeEnabled) {

        float timeOffsetLocal = time_since_start * rgbCycleSpeed;
        float r = 0.5f + 0.5f * sinf(timeOffsetLocal);
        float g = 0.5f + 0.5f * sinf(timeOffsetLocal + 2.0f * MY_PI / 3.0f);
        float b = 0.5f + 0.5f * sinf(timeOffsetLocal + 4.0f * MY_PI / 3.0f);

        btnColor = ImVec4(r * 0.5f, g * 0.5f, b * 0.5f, 0.9f);
        btnHoverColor = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 1.0f);
        btnActiveColor = ImVec4(r, g, b, 1.0f);
    }
    else {

        float pulse = GetPulsatingValue(2.0f, 0.8f, 1.0f);
        btnColor = ImVec4(main_color.x * 0.5f, main_color.y * 0.5f, main_color.z * 0.5f, 0.9f);
        btnHoverColor = ImVec4(main_color.x * 0.7f * pulse, main_color.y * 0.7f * pulse, main_color.z * 0.7f * pulse, 1.0f);
        btnActiveColor = ImVec4(main_color.x * pulse, main_color.y * pulse, main_color.z * pulse, 1.0f);
    }

    style.Colors[ImGuiCol_Button] = btnColor;
    style.Colors[ImGuiCol_ButtonHovered] = btnHoverColor;
    style.Colors[ImGuiCol_ButtonActive] = btnActiveColor;

    bool clicked = ImGui::Button(label, size);

    style.Colors[ImGuiCol_Button] = origBtnColor;
    style.Colors[ImGuiCol_ButtonHovered] = origBtnHoverColor;
    style.Colors[ImGuiCol_ButtonActive] = origBtnActiveColor;

    return clicked;
}

void AnimatedSlider(const char* label, float* value, float min, float max, const char* tooltip) {
    ImGui::PushID(label);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImGuiStyle& style = ImGui::GetStyle();
    float width = ImGui::GetContentRegionAvail().x;

    ImVec4 origFrameBg = style.Colors[ImGuiCol_FrameBg];
    ImVec4 origFrameBgHovered = style.Colors[ImGuiCol_FrameBgHovered];
    ImVec4 origFrameBgActive = style.Colors[ImGuiCol_FrameBgActive];
    ImVec4 origSliderGrab = style.Colors[ImGuiCol_SliderGrab];
    ImVec4 origSliderGrabActive = style.Colors[ImGuiCol_SliderGrabActive];
    float origGrabSize = style.GrabMinSize;
    float origRounding = style.FrameRounding;
    float origFramePaddingY = style.FramePadding.y;

    ImVec4 sliderGrabColor;
    ImVec4 sliderGrabActiveColor;
    ImVec4 frameBgColor;
    ImVec4 valueTextColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    if (rgbModeEnabled) {
        float timeOffset = time_since_start * rgbCycleSpeed;
        float r = 0.5f + 0.5f * sinf(timeOffset);
        float g = 0.5f + 0.5f * sinf(timeOffset + 2.0f * MY_PI / 3.0f);
        float b = 0.5f + 0.5f * sinf(timeOffset + 4.0f * MY_PI / 3.0f);

        sliderGrabColor = ImVec4(r, g, b, 0.9f);
        sliderGrabActiveColor = ImVec4(r * 1.2f, g * 1.2f, b * 1.2f, 1.0f);
        frameBgColor = ImVec4(r * 0.15f, g * 0.15f, b * 0.15f, 0.8f);
    }
    else {
        sliderGrabColor = main_color;
        sliderGrabColor.w = 0.9f;
        sliderGrabActiveColor = ImVec4(main_color.x * 1.2f, main_color.y * 1.2f, main_color.z * 1.2f, 1.0f);
        frameBgColor = ImVec4(main_color.x * 0.15f, main_color.y * 0.15f, main_color.z * 0.15f, 0.8f);
    }

    style.FrameRounding = 2.0f;
    style.Colors[ImGuiCol_FrameBg] = frameBgColor;
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(frameBgColor.x * 1.3f, frameBgColor.y * 1.3f, frameBgColor.z * 1.3f, frameBgColor.w);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(frameBgColor.x * 1.5f, frameBgColor.y * 1.5f, frameBgColor.z * 1.5f, frameBgColor.w);
    style.Colors[ImGuiCol_SliderGrab] = sliderGrabColor;
    style.Colors[ImGuiCol_SliderGrabActive] = sliderGrabActiveColor;
    style.GrabMinSize = 8.0f;

    style.FramePadding.y = 2.0f;

    float sliderWidth = width * 0.90f;
    float leftPadding = (width - sliderWidth) / 2.0f;

    char valueText[32];
    sprintf_s(valueText, "%.2f", *value);

    ImVec2 labelSize = ImGui::CalcTextSize(label);
    ImVec2 valueSize = ImGui::CalcTextSize(valueText);

    ImGui::SetCursorPosX(leftPadding + (sliderWidth - labelSize.x) / 2.0f);
    ImGui::Text("%s", label);

    ImGui::SetCursorPosX(leftPadding);
    ImGui::PushItemWidth(sliderWidth);

    bool valueChanged = ImGui::SliderFloat("##slider", value, min, max, "", ImGuiSliderFlags_NoInput);

    ImVec2 sliderMin = ImGui::GetItemRectMin();
    ImVec2 sliderMax = ImGui::GetItemRectMax();
    float sliderHeight = sliderMax.y - sliderMin.y;

    drawList->AddRectFilled(
        sliderMin,
        sliderMax,
        ImGui::ColorConvertFloat4ToU32(frameBgColor),
        style.FrameRounding
    );

    ImVec4 borderColor;
    if (rgbModeEnabled) {
        float timeOffset = time_since_start * rgbCycleSpeed;
        float r = 0.5f + 0.5f * sinf(timeOffset);
        float g = 0.5f + 0.5f * sinf(timeOffset + 2.0f * MY_PI / 3.0f);
        float b = 0.5f + 0.5f * sinf(timeOffset + 4.0f * MY_PI / 3.0f);
        borderColor = ImVec4(r * 0.6f, g * 0.6f, b * 0.6f, 0.8f);
    }
    else {
        borderColor = ImVec4(main_color.x * 0.6f, main_color.y * 0.6f, main_color.z * 0.6f, 0.8f);
    }

    drawList->AddRect(
        sliderMin,
        sliderMax,
        ImGui::ColorConvertFloat4ToU32(borderColor),
        style.FrameRounding,
        0,
        0.5f
    );

    ImVec2 textPos = ImVec2(
        sliderMin.x + (sliderMax.x - sliderMin.x - valueSize.x) / 2.0f,
        sliderMin.y + (sliderMax.y - sliderMin.y - valueSize.y) / 2.0f
    );

    drawList->AddText(
        ImVec2(textPos.x + 1, textPos.y + 1),
        ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 0.3f)),
        valueText
    );
    drawList->AddText(
        textPos,
        ImGui::ColorConvertFloat4ToU32(valueTextColor),
        valueText
    );

    if ((valueChanged || ImGui::IsItemHovered()) && tooltip) {
        ImGui::BeginTooltip();
        ImGui::Text("%s: %s", label, tooltip);
        ImGui::EndTooltip();
    }

    style.Colors[ImGuiCol_FrameBg] = origFrameBg;
    style.Colors[ImGuiCol_FrameBgHovered] = origFrameBgHovered;
    style.Colors[ImGuiCol_FrameBgActive] = origFrameBgActive;
    style.Colors[ImGuiCol_SliderGrab] = origSliderGrab;
    style.Colors[ImGuiCol_SliderGrabActive] = origSliderGrabActive;
    style.GrabMinSize = origGrabSize;
    style.FrameRounding = origRounding;
    style.FramePadding.y = origFramePaddingY;

    ImGui::PopItemWidth();
    ImGui::PopID();
}

bool AnimatedToggleButton(const char* label, bool* value) {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    float height = ImGui::GetFrameHeight();
    float width = height * 1.8f;
    float radius = height * 0.5f;

    ImGui::InvisibleButton(label, ImVec2(width, height));
    bool changed = false;
    if (ImGui::IsItemClicked()) {
        *value = !*value;
        changed = true;
    }

    float t = *value ? 1.0f : 0.0f;

    ImU32 bgColor, knobColor;
    if (rgbModeEnabled) {

        float timeOffsetLocal = time_since_start * rgbCycleSpeed;
        float r = 0.5f + 0.5f * sinf(timeOffsetLocal);
        float g = 0.5f + 0.5f * sinf(timeOffsetLocal + 2.0f * MY_PI / 3.0f);
        float b = 0.5f + 0.5f * sinf(timeOffsetLocal + 4.0f * MY_PI / 3.0f);

        ImVec4 offColor = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
        ImVec4 onColor = ImVec4(r, g, b, 1.0f);
        ImVec4 blendedColor = ImVec4(
            offColor.x + (onColor.x - offColor.x) * t,
            offColor.y + (onColor.y - offColor.y) * t,
            offColor.z + (onColor.z - offColor.z) * t,
            1.0f
        );

        bgColor = ImGui::ColorConvertFloat4ToU32(blendedColor);
        knobColor = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    }
    else {

        ImVec4 offColor = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
        ImVec4 onColor = main_color;
        ImVec4 blendedColor = ImVec4(
            offColor.x + (onColor.x - offColor.x) * t,
            offColor.y + (onColor.y - offColor.y) * t,
            offColor.z + (onColor.z - offColor.z) * t,
            1.0f
        );

        if (*value) {
            float pulse = GetPulsatingValue(3.0f, 0.9f, 1.1f);
            blendedColor.x *= pulse;
            blendedColor.y *= pulse;
            blendedColor.z *= pulse;
        }

        bgColor = ImGui::ColorConvertFloat4ToU32(blendedColor);
        knobColor = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    drawList->AddRectFilled(p, ImVec2(p.x + width, p.y + height), bgColor, height * 0.5f);

    drawList->AddRect(p, ImVec2(p.x + width, p.y + height), ImGui::ColorConvertFloat4ToU32(ImVec4(0.6f, 0.1f, 0.1f, 1.0f)), height * 0.5f, 0, 1.0f);

    float posX = p.x + radius + t * (width - radius * 2.0f);

    drawList->AddCircleFilled(ImVec2(posX + 1, p.y + radius + 1), radius * 0.8f, ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 0.3f)));

    drawList->AddCircleFilled(ImVec2(posX, p.y + radius), radius * 0.8f, knobColor);

    if (*value) {
        drawList->AddCircle(ImVec2(posX, p.y + radius), radius * 0.6f, ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 0.5f)), 0, 1.5f);
    }

    if (*value) {
        drawList->AddCircleFilled(ImVec2(posX, p.y + radius), radius * 0.3f, ImGui::ColorConvertFloat4ToU32(ImVec4(0.8f, 0.0f, 0.0f, 1.0f)));
    }

    return changed;
}

void DrawAdvancedSeparator(bool rgbMode, float widthFactor, float yFactor) {

    ImVec4 sepColor = rgbMode ?
        ImVec4(
            0.5f + 0.5f * sinf(time_since_start * rgbCycleSpeed),
            0.5f + 0.5f * sinf(time_since_start * rgbCycleSpeed + 2.0f * MY_PI / 3.0f),
            0.5f + 0.5f * sinf(time_since_start * rgbCycleSpeed + 4.0f * MY_PI / 3.0f),
            0.8f
        ) : ImVec4(main_color.x, main_color.y, main_color.z, 0.8f);

    float windowWidth = ImGui::GetWindowWidth();
    float contentWidth = ImGui::GetContentRegionAvail().x;

    float lineWidth = contentWidth * widthFactor;

    float centeringOffset = 20.0f;

    ImGui::Dummy(ImVec2(0, 2.0f * yFactor));

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();

    float startX = windowPos.x + (windowWidth - lineWidth) / 2.0f - centeringOffset;
    float startY = ImGui::GetCursorScreenPos().y;

    float rightBorderMargin = 16.0f;
    float endX = Min(startX + lineWidth, windowPos.x + windowWidth - rightBorderMargin);

    float lineThickness = 0.8f * yFactor;

    drawList->AddCircleFilled(
        ImVec2(startX, startY),
        lineThickness / 2.0f,
        ImGui::ColorConvertFloat4ToU32(sepColor),
        12
    );

    drawList->AddLine(
        ImVec2(startX, startY),
        ImVec2(endX, startY),
        ImGui::ColorConvertFloat4ToU32(sepColor),
        lineThickness
    );

    drawList->AddCircleFilled(
        ImVec2(endX, startY),
        lineThickness / 2.0f,
        ImGui::ColorConvertFloat4ToU32(sepColor),
        12
    );

    ImGui::Dummy(ImVec2(0, 2.0f * yFactor));
}

void DrawEnhancedPanel(const char* label, float alpha = 0.1f) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImVec2 contentSize = ImGui::GetContentRegionAvail();

    ImGui::BeginGroup();
    ImGui::TextUnformatted(label);
    ImGui::Spacing();

    ImGui::Indent(10.0f);

    ImVec2 contentStart = ImGui::GetCursorScreenPos();

    ImGui::PushID(label);
}

void EndEnhancedPanel(bool rgbMode) {

    ImGui::Unindent(10.0f);

    ImGui::EndGroup();

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();

    float width = max.x - min.x;
    float height = max.y - min.y;

    ImVec4 borderColor;
    if (rgbMode) {
        float timeOffset = time_since_start * rgbCycleSpeed;
        float r = 0.5f + 0.5f * sinf(timeOffset);
        float g = 0.5f + 0.5f * sinf(timeOffset + 2.0f * MY_PI / 3.0f);
        float b = 0.5f + 0.5f * sinf(timeOffset + 4.0f * MY_PI / 3.0f);
        borderColor = ImVec4(r, g, b, 0.7f);
    }
    else {
        float pulse = AnimateSin(1.5f, 0.7f, 1.0f, 0.0f);
        borderColor = ImVec4(main_color.x * pulse, main_color.y * pulse, main_color.z * pulse, 0.7f);
    }

    ImVec4 bgColor = ImVec4(0.08f, 0.08f, 0.08f, 0.3f);

    drawList->AddRectFilled(
        min,
        max,
        ImGui::ColorConvertFloat4ToU32(bgColor),
        8.0f
    );

    drawList->AddRect(
        min,
        max,
        ImGui::ColorConvertFloat4ToU32(borderColor),
        8.0f,
        ImDrawFlags_RoundCornersAll,
        1.0f
    );

    ImGui::PopID();

    ImGui::Spacing();
    ImGui::Spacing();
}

bool AnimatedProfessionalSlider(const char* label, float* value, float min, float max, const char* format, const char* tooltip) {
    ImGui::PushID(label);
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4 origFrameBg = style.Colors[ImGuiCol_FrameBg];
    ImVec4 origFrameBgHovered = style.Colors[ImGuiCol_FrameBgHovered];
    ImVec4 origFrameBgActive = style.Colors[ImGuiCol_FrameBgActive];
    ImVec4 origSliderGrab = style.Colors[ImGuiCol_SliderGrab];
    ImVec4 origSliderGrabActive = style.Colors[ImGuiCol_SliderGrabActive];
    float origRounding = style.FrameRounding;
    float origGrabRounding = style.GrabRounding;
    float origFramePaddingY = style.FramePadding.y;

    ImVec4 sliderColor, sliderActiveColor, bgColor, textColor, borderColor;

    if (rgbModeEnabled) {

        float timeOffset = time_since_start * rgbCycleSpeed;
        float r = 0.5f + 0.5f * sinf(timeOffset);
        float g = 0.5f + 0.5f * sinf(timeOffset + 2.0f * MY_PI / 3.0f);
        float b = 0.5f + 0.5f * sinf(timeOffset + 4.0f * MY_PI / 3.0f);

        sliderColor = ImVec4(r, g, b, 0.9f);
        sliderActiveColor = ImVec4(r * 1.2f, g * 1.2f, b * 1.2f, 1.0f);
        bgColor = ImVec4(r * 0.15f, g * 0.15f, b * 0.15f, 0.8f);
        borderColor = ImVec4(r * 0.6f, g * 0.6f, b * 0.6f, 0.8f);
        textColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    else {

        sliderColor = main_color;
        sliderColor.w = 0.9f;
        sliderActiveColor = ImVec4(main_color.x * 1.2f, main_color.y * 1.2f, main_color.z * 1.2f, 1.0f);
        bgColor = ImVec4(main_color.x * 0.15f, main_color.y * 0.15f, main_color.z * 0.15f, 0.8f);
        textColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        borderColor = ImVec4(main_color.x * 0.6f, main_color.y * 0.6f, main_color.z * 0.6f, 0.8f);
    }

    style.FrameRounding = 2.0f;
    style.GrabRounding = 2.0f;

    style.FramePadding.y = 1.5f;

    style.Colors[ImGuiCol_FrameBg] = bgColor;
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(bgColor.x * 1.2f, bgColor.y * 1.2f, bgColor.z * 1.2f, bgColor.w);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(bgColor.x * 1.5f, bgColor.y * 1.5f, bgColor.z * 1.5f, bgColor.w);
    style.Colors[ImGuiCol_SliderGrab] = sliderColor;
    style.Colors[ImGuiCol_SliderGrabActive] = sliderActiveColor;

    float originalGrabSize = style.GrabMinSize;
    style.GrabMinSize = 6.0f;

    float totalWidth = ImGui::GetContentRegionAvail().x;
    float sliderWidth = totalWidth * 0.85f;
    float leftPadding = (totalWidth - sliderWidth) / 2.0f;

    char labelText[64];
    sprintf_s(labelText, "%s", label);

    char valueText[32];
    sprintf_s(valueText, format, *value);

    ImVec2 labelSize = ImGui::CalcTextSize(labelText);
    ImVec2 valueSize = ImGui::CalcTextSize(valueText);

    float labelFontScale = ImGui::GetFont()->Scale;
    ImGui::GetFont()->Scale *= 0.92f;

    labelSize = ImGui::CalcTextSize(labelText);

    ImGui::SetCursorPosX(leftPadding + (sliderWidth - labelSize.x) / 2.0f);
    ImGui::Text("%s", labelText);

    ImGui::Dummy(ImVec2(0, 1.0f));

    ImGui::GetFont()->Scale = labelFontScale;

    ImGui::SetCursorPosX(leftPadding);
    ImGui::PushItemWidth(sliderWidth);

    bool changed = ImGui::SliderFloat("##slider", value, min, max, "", ImGuiSliderFlags_NoInput);

    ImVec2 sliderMin = ImGui::GetItemRectMin();
    ImVec2 sliderMax = ImGui::GetItemRectMax();
    float sliderHeight = sliderMax.y - sliderMin.y;

    drawList->AddRectFilled(
        sliderMin,
        sliderMax,
        ImGui::ColorConvertFloat4ToU32(bgColor),
        style.FrameRounding
    );

    drawList->AddRect(
        sliderMin,
        sliderMax,
        ImGui::ColorConvertFloat4ToU32(borderColor),
        style.FrameRounding,
        0,
        0.3f
    );

    ImVec2 textPos = ImVec2(
        sliderMin.x + (sliderMax.x - sliderMin.x - valueSize.x) / 2.0f,
        sliderMin.y + (sliderMax.y - sliderMin.y - valueSize.y) / 2.0f
    );

    float origFontScale = ImGui::GetFont()->Scale;
    ImGui::GetFont()->Scale *= 0.9f;

    ImVec2 newValueSize = ImGui::CalcTextSize(valueText);
    textPos = ImVec2(
        sliderMin.x + (sliderMax.x - sliderMin.x - newValueSize.x) / 2.0f,
        sliderMin.y + (sliderMax.y - sliderMin.y - newValueSize.y) / 2.0f
    );

    drawList->AddText(
        ImGui::GetFont(),
        ImGui::GetFont()->FontSize,
        ImVec2(textPos.x + 1, textPos.y + 1),
        ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 0.2f)),
        valueText
    );

    drawList->AddText(
        ImGui::GetFont(),
        ImGui::GetFont()->FontSize,
        textPos,
        ImGui::ColorConvertFloat4ToU32(textColor),
        valueText
    );

    ImGui::GetFont()->Scale = origFontScale;

    if (tooltip && ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("%s: %s", label, tooltip);
        ImGui::EndTooltip();
    }

    style.Colors[ImGuiCol_FrameBg] = origFrameBg;
    style.Colors[ImGuiCol_FrameBgHovered] = origFrameBgHovered;
    style.Colors[ImGuiCol_FrameBgActive] = origFrameBgActive;
    style.Colors[ImGuiCol_SliderGrab] = origSliderGrab;
    style.Colors[ImGuiCol_SliderGrabActive] = origSliderGrabActive;
    style.GrabMinSize = originalGrabSize;
    style.FrameRounding = origRounding;
    style.GrabRounding = origGrabRounding;
    style.FramePadding.y = origFramePaddingY;

    ImGui::PopItemWidth();
    ImGui::PopID();
    return changed;
}

bool PremiumButton(const char* label, const ImVec2& size) {

    ImVec2 buttonSize = size;

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImGui::PushID(label);

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4 origButtonCol = style.Colors[ImGuiCol_Button];
    ImVec4 origButtonHoveredCol = style.Colors[ImGuiCol_ButtonHovered];
    ImVec4 origButtonActiveCol = style.Colors[ImGuiCol_ButtonActive];

    ImVec4 buttonColor, hoverColor, activeColor;

    if (rgbModeEnabled) {
        float timeOffset = time_since_start * rgbCycleSpeed;
        float r = 0.5f + 0.5f * sinf(timeOffset);
        float g = 0.5f + 0.5f * sinf(timeOffset + 2.0f * MY_PI / 3.0f);
        float b = 0.5f + 0.5f * sinf(timeOffset + 4.0f * MY_PI / 3.0f);

        buttonColor = ImVec4(r * 0.3f, g * 0.3f, b * 0.3f, 1.0f);
        hoverColor = ImVec4(r * 0.5f, g * 0.5f, b * 0.5f, 1.0f);
        activeColor = ImVec4(r * 0.7f, g * 0.7f, b * 0.7f, 1.0f);
    }
    else {
        float pulse = AnimateSin(2.0f, 0.7f, 1.0f, 0.0f);
        buttonColor = ImVec4(main_color.x * 0.3f, main_color.y * 0.3f, main_color.z * 0.3f, 1.0f);
        hoverColor = ImVec4(main_color.x * 0.5f * pulse, main_color.y * 0.5f * pulse, main_color.z * 0.5f * pulse, 1.0f);
        activeColor = ImVec4(main_color.x * 0.7f, main_color.y * 0.7f, main_color.z * 0.7f, 1.0f);
    }

    style.Colors[ImGuiCol_Button] = buttonColor;
    style.Colors[ImGuiCol_ButtonHovered] = hoverColor;
    style.Colors[ImGuiCol_ButtonActive] = activeColor;

    if (buttonSize.x <= 0) buttonSize.x = ImGui::GetContentRegionAvail().x;
    if (buttonSize.y <= 0) buttonSize.y = ImGui::GetFrameHeight() * 1.5f;

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 textSize = ImGui::CalcTextSize(label);
    ImVec2 textPos = ImVec2(
        pos.x + (buttonSize.x - textSize.x) * 0.5f,
        pos.y + (buttonSize.y - textSize.y) * 0.5f
    );

    bool clicked = ImGui::Button("##premium_button", buttonSize);

    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();

    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();

    if (hovered || active) {
        float borderTime = time_since_start * 3.0f;
        float borderThickness = active ? 2.0f : 1.5f;
        ImVec4 borderColor = active ? activeColor : hoverColor;
        borderColor.w = 1.0f;

        float cornerSize = 10.0f;
        float animOffset = fmodf(borderTime, 8.0f) - 4.0f;
        float animPos = fabsf(animOffset) * 0.25f;

        float animatedCornerSize = cornerSize * (1.0f + animPos * 0.5f);

        drawList->AddLine(
            ImVec2(min.x, min.y + animatedCornerSize),
            ImVec2(min.x, min.y),
            ImGui::ColorConvertFloat4ToU32(borderColor),
            borderThickness
        );
        drawList->AddLine(
            ImVec2(min.x, min.y),
            ImVec2(min.x + animatedCornerSize, min.y),
            ImGui::ColorConvertFloat4ToU32(borderColor),
            borderThickness
        );

        drawList->AddLine(
            ImVec2(max.x - animatedCornerSize, min.y),
            ImVec2(max.x, min.y),
            ImGui::ColorConvertFloat4ToU32(borderColor),
            borderThickness
        );
        drawList->AddLine(
            ImVec2(max.x, min.y),
            ImVec2(max.x, min.y + animatedCornerSize),
            ImGui::ColorConvertFloat4ToU32(borderColor),
            borderThickness
        );

        drawList->AddLine(
            ImVec2(min.x, max.y - animatedCornerSize),
            ImVec2(min.x, max.y),
            ImGui::ColorConvertFloat4ToU32(borderColor),
            borderThickness
        );
        drawList->AddLine(
            ImVec2(min.x, max.y),
            ImVec2(min.x + animatedCornerSize, max.y),
            ImGui::ColorConvertFloat4ToU32(borderColor),
            borderThickness
        );

        drawList->AddLine(
            ImVec2(max.x - animatedCornerSize, max.y),
            ImVec2(max.x, max.y),
            ImGui::ColorConvertFloat4ToU32(borderColor),
            borderThickness
        );
        drawList->AddLine(
            ImVec2(max.x, max.y),
            ImVec2(max.x, max.y - animatedCornerSize),
            ImGui::ColorConvertFloat4ToU32(borderColor),
            borderThickness
        );
    }

    if (active) {

        ImVec4 shadowColor = ImVec4(0.0f, 0.0f, 0.0f, 0.7f);
        drawList->AddText(
            ImVec2(textPos.x + 1, textPos.y + 1),
            ImGui::ColorConvertFloat4ToU32(shadowColor),
            label
        );
    }

    ImVec4 textColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    drawList->AddText(textPos, ImGui::ColorConvertFloat4ToU32(textColor), label);

    style.Colors[ImGuiCol_Button] = origButtonCol;
    style.Colors[ImGuiCol_ButtonHovered] = origButtonHoveredCol;
    style.Colors[ImGuiCol_ButtonActive] = origButtonActiveCol;

    ImGui::PopID();
    return clicked;
}

void DrawInnerContentBorder()
{
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();

    float marginX = 12.0f;
    float marginY = 60.0f;
    float marginBottom = 12.0f;

    float separatorPadding = 2.0f;

    ImVec2 innerStart = ImVec2(windowPos.x + marginX + separatorPadding, windowPos.y + marginY);
    ImVec2 innerEnd = ImVec2(windowPos.x + windowSize.x - marginX - separatorPadding, windowPos.y + windowSize.y - marginBottom);

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImVec4 borderColor;
    if (rgbModeEnabled) {
        float timeOffset = time_since_start * rgbCycleSpeed;
        float r = 0.5f + 0.5f * sinf(timeOffset);
        float g = 0.5f + 0.5f * sinf(timeOffset + 2.0f * MY_PI / 3.0f);
        float b = 0.5f + 0.5f * sinf(timeOffset + 4.0f * MY_PI / 3.0f);
        borderColor = ImVec4(r, g, b, 0.7f);
    }
    else {

        float pulse = AnimateSin(1.5f, 0.9f, 1.0f, 0.0f);
        borderColor = ImVec4(main_color.x * pulse, main_color.y * pulse, main_color.z * pulse, 0.7f);
    }

    float thickness = 0.8f;

    drawList->AddLine(
        ImVec2(innerStart.x, innerStart.y),
        ImVec2(innerEnd.x, innerStart.y),
        ImGui::ColorConvertFloat4ToU32(borderColor),
        thickness
    );

    drawList->AddLine(
        ImVec2(innerEnd.x, innerStart.y),
        ImVec2(innerEnd.x, innerEnd.y),
        ImGui::ColorConvertFloat4ToU32(borderColor),
        thickness
    );

    drawList->AddLine(
        ImVec2(innerEnd.x, innerEnd.y),
        ImVec2(innerStart.x, innerEnd.y),
        ImGui::ColorConvertFloat4ToU32(borderColor),
        thickness
    );

    drawList->AddLine(
        ImVec2(innerStart.x, innerEnd.y),
        ImVec2(innerStart.x, innerStart.y),
        ImGui::ColorConvertFloat4ToU32(borderColor),
        thickness
    );
}

bool AnimatedPanningSlider(const char* label, float* value, float min, float max) {
    ImGui::PushID(label);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImGuiStyle& style = ImGui::GetStyle();
    float width = ImGui::GetContentRegionAvail().x;

    ImVec4 origFrameBg = style.Colors[ImGuiCol_FrameBg];
    ImVec4 origFrameBgHovered = style.Colors[ImGuiCol_FrameBgHovered];
    ImVec4 origFrameBgActive = style.Colors[ImGuiCol_FrameBgActive];
    ImVec4 origSliderGrab = style.Colors[ImGuiCol_SliderGrab];
    ImVec4 origSliderGrabActive = style.Colors[ImGuiCol_SliderGrabActive];
    float origGrabSize = style.GrabMinSize;
    float origRounding = style.FrameRounding;
    float origFramePaddingY = style.FramePadding.y;

    ImVec4 sliderGrabColor;
    ImVec4 sliderGrabActiveColor;
    ImVec4 frameBgColor;
    ImVec4 valueTextColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 leftColor;
    ImVec4 rightColor;
    ImVec4 centerColor;
    ImVec4 borderColor;

    if (rgbModeEnabled) {

        float timeOffset = time_since_start * rgbCycleSpeed;
        float r = 0.5f + 0.5f * sinf(timeOffset);
        float g = 0.5f + 0.5f * sinf(timeOffset + 2.0f * MY_PI / 3.0f);
        float b = 0.5f + 0.5f * sinf(timeOffset + 4.0f * MY_PI / 3.0f);

        sliderGrabColor = ImVec4(r, g, b, 0.9f);
        sliderGrabActiveColor = ImVec4(r * 1.2f, g * 1.2f, b * 1.2f, 1.0f);

        leftColor = ImVec4(r, g, b, *value < 0 ? 0.9f : 0.4f);
        rightColor = ImVec4(r, g, b, *value > 0 ? 0.9f : 0.4f);

        centerColor = ImVec4(r * 0.8f, g * 0.8f, b * 0.8f, 0.7f);
        borderColor = ImVec4(r * 0.6f, g * 0.6f, b * 0.6f, 0.8f);

        frameBgColor = ImVec4(r * 0.15f, g * 0.15f, b * 0.15f, 0.8f);
    }
    else {

        sliderGrabColor = main_color;
        sliderGrabColor.w = 0.9f;

        sliderGrabActiveColor = ImVec4(main_color.x * 1.2f, main_color.y * 1.2f, main_color.z * 1.2f, 1.0f);

        leftColor = ImVec4(main_color.x, main_color.y, main_color.z, *value < 0 ? 0.9f : 0.4f);
        rightColor = ImVec4(main_color.x, main_color.y, main_color.z, *value > 0 ? 0.9f : 0.4f);

        centerColor = ImVec4(main_color.x * 0.8f, main_color.y * 0.8f, main_color.z * 0.8f, 0.7f);
        borderColor = ImVec4(main_color.x * 0.6f, main_color.y * 0.6f, main_color.z * 0.6f, 0.8f);

        frameBgColor = ImVec4(main_color.x * 0.15f, main_color.y * 0.15f, main_color.z * 0.15f, 0.8f);
    }

    style.FrameRounding = 2.0f;
    style.Colors[ImGuiCol_FrameBg] = frameBgColor;
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(frameBgColor.x * 1.3f, frameBgColor.y * 1.3f, frameBgColor.z * 1.3f, frameBgColor.w);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(frameBgColor.x * 1.5f, frameBgColor.y * 1.5f, frameBgColor.z * 1.5f, frameBgColor.w);
    style.Colors[ImGuiCol_SliderGrab] = sliderGrabColor;
    style.Colors[ImGuiCol_SliderGrabActive] = sliderGrabActiveColor;

    style.GrabMinSize = 8.0f;
    style.FramePadding.y = 2.0f;

    float sliderWidth = width * 0.90f;
    float leftPadding = (width - sliderWidth) / 2.0f;

    const char* valueText = FormatPanningText(*value);

    ImVec2 labelSize = ImGui::CalcTextSize(label);
    ImGui::SetCursorPosX(leftPadding + (sliderWidth - labelSize.x) / 2.0f);
    ImGui::Text("%s", label);

    ImGui::SetCursorPosX(leftPadding);
    ImGui::PushItemWidth(sliderWidth);
    bool valueChanged = ImGui::SliderFloat("##panning", value, min, max, "", ImGuiSliderFlags_NoInput);

    ImVec2 sliderMin = ImGui::GetItemRectMin();
    ImVec2 sliderMax = ImGui::GetItemRectMax();
    float sliderHeight = sliderMax.y - sliderMin.y;

    drawList->AddRectFilled(sliderMin, sliderMax, ImGui::ColorConvertFloat4ToU32(frameBgColor), style.FrameRounding);
    drawList->AddRect(sliderMin, sliderMax, ImGui::ColorConvertFloat4ToU32(borderColor), style.FrameRounding, 0, 0.5f);

    float centerX = sliderMin.x + (sliderMax.x - sliderMin.x) * 0.5f;

    drawList->AddLine(
        ImVec2(centerX, sliderMin.y + 2),
        ImVec2(centerX, sliderMax.y - 2),
        ImGui::ColorConvertFloat4ToU32(centerColor),
        0.5f
    );

    float textY = sliderMin.y + (sliderHeight - ImGui::GetFontSize()) / 2.0f;

    drawList->AddText(
        ImVec2(sliderMin.x + 4, textY),
        ImGui::ColorConvertFloat4ToU32(leftColor),
        "L"
    );

    drawList->AddText(
        ImVec2(sliderMax.x - 12, textY),
        ImGui::ColorConvertFloat4ToU32(rightColor),
        "R"
    );

    ImVec2 valueSize = ImGui::CalcTextSize(valueText);
    ImVec2 textPos = ImVec2(
        sliderMin.x + (sliderMax.x - sliderMin.x - valueSize.x) / 2.0f,
        sliderMin.y + (sliderMax.y - sliderMin.y - valueSize.y) / 2.0f
    );

    drawList->AddText(
        ImVec2(textPos.x + 1, textPos.y + 1),
        ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 0.3f)),
        valueText
    );
    drawList->AddText(
        textPos,
        ImGui::ColorConvertFloat4ToU32(valueTextColor),
        valueText
    );

    style.Colors[ImGuiCol_FrameBg] = origFrameBg;
    style.Colors[ImGuiCol_FrameBgHovered] = origFrameBgHovered;
    style.Colors[ImGuiCol_FrameBgActive] = origFrameBgActive;
    style.Colors[ImGuiCol_SliderGrab] = origSliderGrab;
    style.Colors[ImGuiCol_SliderGrabActive] = origSliderGrabActive;
    style.GrabMinSize = origGrabSize;
    style.FrameRounding = origRounding;
    style.FramePadding.y = origFramePaddingY;

    ImGui::PopItemWidth();
    ImGui::PopID();
    return valueChanged;
}

std::string GetProcessName() {
    char buffer[MAX_PATH] = "Unknown";
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string path(buffer);
    size_t pos = path.find_last_of("\\/");
    std::string processName = (pos != std::string::npos) ? path.substr(pos + 1) : path;

    DWORD pid = GetCurrentProcessId();

    HANDLE hProcess = GetCurrentProcess();

    HMODULE hModule = NULL;
    DWORD flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
    GetModuleHandleExA(flags, (LPCSTR)&GetProcessName, &hModule);

    MODULEINFO moduleInfo;
    if (GetModuleInformation(hProcess, hModule, &moduleInfo, sizeof(moduleInfo))) {

        HMODULE hDiscordVoice = GetModuleHandleA("discord_voice.node");
        void* hookTargetAddr = nullptr;

        if (hDiscordVoice) {

            hookTargetAddr = (char*)hDiscordVoice + 0x863E90;
        }

        char processLine[64] = { 0 };
        char baseLine[64] = { 0 };
        char hookLine[64] = { 0 };

        sprintf_s(processLine, "Process: %s | PID: %lu", processName.c_str(), pid);
        sprintf_s(baseLine, "Base: 0x%p | Size: %uKB", moduleInfo.lpBaseOfDll, moduleInfo.SizeOfImage / 1024);
        sprintf_s(hookLine, "Hooked: 0x%p [opus_encode]", hookTargetAddr);

        return std::string(processLine) + "\n" + baseLine + "\n" + hookLine;
    }

    char processLine[64] = { 0 };
    char moduleLine[64] = { 0 };

    sprintf_s(processLine, "Process: %s | PID: %lu", processName.c_str(), pid);
    sprintf_s(moduleLine, "Module: 0x%p", (void*)hModule);

    return std::string(processLine) + "\n" + moduleLine;
}