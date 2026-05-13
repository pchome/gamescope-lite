#pragma once
#include "imgui.h"
#include "imgui_internal.h"

#include <format>
#include <array>
#include <utility>

namespace {
constexpr auto toHz = 1000;
constexpr auto toHzf = 1000.0f;
constexpr auto toms = 1000.0f;

constexpr std::array<char const*, 4> GamescopeUpscaleFilterName{"LINEAR", "PIXEL", "FSR", "NIS"};
constexpr std::array<char const*, 6> GamescopeUpscaleScalerName{"AUTO", "INTEGER", "FIT", "FILL", "STRETCH", "NATIVE"};
} // namespace

extern bool g_bForceWindowsFullscreen;

namespace ImFmt {
template <typename... Args>
void Text(std::format_string<Args...> const fmt, Args&&... args) {
  auto const text = std::format(fmt, std::forward<Args>(args)...);
  ImGui::TextUnformatted(text.data(), text.data() + text.size());
}
template <typename... Args>
void TextDisabled(std::format_string<Args...> const fmt, Args&&... args) {
  auto const text = std::format(fmt, std::forward<Args>(args)...);
  // ImGuiContext* g = ImGui::GetCurrentContext();
  ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetCurrentContext()->Style.Colors[ImGuiCol_TextDisabled]);
  ImGui::TextUnformatted(text.data(), text.data() + text.size());
  ImGui::PopStyleColor();
}
} // namespace ImFmt

namespace ImTpl {
void Toggle(auto name, bool control, auto cb) {
  static bool check = control;
  ImGui::Checkbox(name, &check);
  if (check != control) {
    cb();
  }
}

template <typename cT>
void Select(auto name, cT control, auto names, auto cb) {
  static int check = static_cast<int>(control);
  ImGui::SetNextItemWidth(100);
  ImGui::Combo(name, &check, names.data(), names.size());
  if (static_cast<cT>(check) != control) {
    cb(check);
  }
}

template <typename T, typename cT>
void Select(auto name, cT control, auto names, auto values, auto cb) {
  static int check = 0;
  auto value_current = values.at(static_cast<T>(check));
  ImGui::SetNextItemWidth(100);
  ImGui::Combo(name, &check, names.data(), names.size());
  if (static_cast<cT>(value_current) != control) {
    cb(value_current);
  }
}
} // namespace ImTpl

namespace ImHlp {
// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
template <typename T> void Hint(T desc) {
  ImFmt::TextDisabled("(?)");
  if (ImGui::BeginItemTooltip()) {
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
    ImGui::TextUnformatted(desc.data(), desc.data() + desc.size());
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }
}
} // namespace ImHlp

namespace gamescope {} // namespace gamescope
