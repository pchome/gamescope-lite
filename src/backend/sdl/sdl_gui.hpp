#pragma once
#include "imgui.h"

#include <format>

namespace {
  constexpr auto toHz = 1000;
  constexpr auto toHzf = 1000.0f;
  constexpr auto toms = 1000.0f;
}

namespace ImFmt {
template <typename... Args> void Text(std::format_string<Args...> const fmt, Args&&... args) {
  auto const text = std::format(fmt, static_cast<Args&&>(args)...);
  ImGui::TextUnformatted(text.data(), text.data() + text.size());
}
} // namespace ImFmt

namespace gamescope {} // namespace gamescope
