#pragma once
#include "imgui.h"

#include <format>

namespace {
constexpr auto toHz = 1000;
constexpr auto toHzf = 1000.0f;
constexpr auto toms = 1000.0f;
} // namespace

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
} // namespace ImTpl

namespace gamescope {} // namespace gamescope
