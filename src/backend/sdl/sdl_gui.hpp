#pragma once
#include "imgui.h"

#include <format>
#include <map>
#include <array>
#include <utility>

namespace {
constexpr auto toHz = 1000;
constexpr auto toHzf = 1000.0f;
constexpr auto toms = 1000.0f;

enum class GamescopeAspectRatio : std::uint8_t {
  W16H9,  // 16:9
  W16H10, // 16:10
  W4H3,   // 4:3
  W24H10, // 24:10
  W64H27, // 64:27
  W43H18, // 43:18
  ASPECT_RATIO_COUNT,
};

constexpr std::array<char const*, std::to_underlying(GamescopeAspectRatio::ASPECT_RATIO_COUNT)>
GamescopeAspectRatioName{"16:9", "16:10", "4:3", "24:10", "64:27", "43:18"};

std::map<GamescopeAspectRatio, double> const GamescopeAspectRatioValue{
    {GamescopeAspectRatio::W16H9, 16 / 9.0},
    {GamescopeAspectRatio::W16H10, 16 / 10.0},
    {GamescopeAspectRatio::W4H3, 4 / 3.0},
    {GamescopeAspectRatio::W24H10, 24 / 10.0},
    {GamescopeAspectRatio::W64H27, 64 / 27.0},
    {GamescopeAspectRatio::W43H18, 43 / 18.0},
};

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

namespace gamescope {} // namespace gamescope
