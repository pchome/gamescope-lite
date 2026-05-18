#include <SDL3/SDL_render.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include "backend/sdl/sdl_action.hpp"

#include "sdl_connector.hpp"
#include "sdl_gui.hpp"

#include "GamescopeVersion.h"
#include "constants_include.hpp"
#include "main.hpp"

#include "Utils/rdb.hpp"

#include <print>
#include <ranges>

namespace gamescope {

void CSDLConnector::UiInit() {
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  // Disable imgui.ini
  io.IniFilename = nullptr;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  ImGui_ImplSDL3_InitForSDLRenderer(m_pPopup, m_pUiRenderer);
  ImGui_ImplSDLRenderer3_Init(m_pUiRenderer);
}

void CSDLConnector::UiStartFrame() {
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
}

namespace {
void UiLayoutAboutTab() {
  ImGui::SeparatorText("About");
  ImGui::LabelText("Name", "%s", gamescopeName);
  ImGui::LabelText("Version", "%s", k_szGamescopeVersion);
  ImGui::SeparatorText("ImGui");
  ImGui::LabelText("Version", "%s", ImGui::GetVersion());
}
void UiLayoutBuildTab() {
  ImGui::SeparatorText("Build");
  ImGui::LabelText("Type", "%s", build::buildtype);
  ImGui::SeparatorText("Paths");
  ImGui::LabelText("prefix", "%s", build::prefix);
  ImGui::LabelText("datadir", "%s", build::datadir);
  ImGui::LabelText("libdir", "%s", build::libdir);
  ImGui::LabelText("bindir", "%s", build::bindir);
}

#if !defined(NDEBUG)
void UiLayoutDebugTab() {
  ImGui::SeparatorText("Debug");
  static bool openDebug = false;
  static int debug = 0;
  if (ImGui::Button("Open Debug Log")) {
    debug++;
  }
  if ((debug & 1) != 0) {
    ImGui::ShowDebugLogWindow(&openDebug);
  }
  static bool openMetrics = false;
  static int metrics= 0;
  if (ImGui::Button("Open Metrics")) {
    metrics++;
  }
  if ((metrics & 1) != 0) {
    ImGui::ShowMetricsWindow(&openMetrics);
  }
  ImFmt::Text("Nested: {}x{}@{}", g_nNestedWidth, g_nNestedHeight, g_nNestedRefresh / toHz);
  ImFmt::Text("Output: {}x{}@{}", g_nOutputWidth, g_nOutputHeight, g_nOutputRefresh / toHz);
  ImFmt::Text("Aspect: {}", g_aspectRatio);

  ImGui::SeparatorText("Nested");
  ImFmt::Text("Unfocused Refresh: {}", g_nNestedUnfocusedRefresh / toHz);
  ImFmt::Text("Display Index: {}", g_nNestedDisplayIndex);

  ImGui::SeparatorText("Output");
  ImGui::BeginDisabled();
  ImTpl::Toggle("HDR Enabled", g_bOutputHDREnabled, [] {});
  ImFmt::Text("Screen Type: Internal");
  ImTpl::Toggle("Fullscreen", g_bFullscreen, [] {});
  ImGui::EndDisabled();

  ImGui::SeparatorText("Input");
  ImGui::BeginDisabled();
  ImTpl::Toggle("Force Relative Mouse", g_bForceRelativeMouse, [] {});
  ImTpl::Toggle("Keyboard Grabbed", g_bGrabbed, [] {});
  ImGui::EndDisabled();
  ImFmt::Text("Mouse Sensitivity: {}", g_mouseSensitivity);
}
#endif

// TODO: real funcs
void UiLayoutUpscaleFilterSharpnessStrenght() {
  struct Funcs {
    static auto Red(void* /*unused*/, int i) -> float { return -sinf(i * 0.1f); }
    static auto Green(void* /*unused*/, int i) -> float { return (1.0f - (i * 0.1f)); }
    static auto None(void* /*unused*/, int i) -> float { return i * 0.0f; }
  };
  static int func_type = 0;
  static int display_offset = defaults::minFSRSharpness;
  static int display_count = defaults::maxFSRSharpness - 3; //+ 1;
  auto filter_current = static_cast<int>(g_wantedUpscaleFilter);
  func_type = (filter_current == 3) ? 1 : (filter_current == 2) ? 0 : 2;
  float (*func)(void*, int) = (func_type == 0) ? Funcs::Red : (func_type == 1) ? Funcs::Green : Funcs::None;
  ImGui::BeginDisabled();
  ImGui::SetNextItemWidth((ImGui::GetFontSize() * 8) - 4);
  ImGui::PlotLines("Strenght", func, nullptr, display_count, display_offset, nullptr, -1.0f, 1.0f, ImVec2(0, 20));
  ImGui::EndDisabled();
}
void UiLayoutUpscaleFilterPreset() {
  using namespace rdb;
  using namespace std::literals;

  static int plimit = rdb::Preset_Original;
  static int prev_p = plimit;
  static auto to_W = g_nNestedWidth * UpscalingPresetValue.at(plimit);
  static auto to_H = g_nNestedHeight * UpscalingPresetValue.at(plimit);

  ImFmt::Text("{}x{} -> {}x{}", g_nNestedWidth, g_nNestedHeight, to_W, to_H);
  ImFmt::Text("MipBias += {:.4f}", rdb::MipCorrectionValue.at(plimit));

  static auto const mhint = "You shoud hijack application's MipBias,\n"sv
  "e.g. `d3d11.samplerLodBias = -1`\n"sv
  "The given number will be added to the LOD bias provided by the application.\n"sv
  "Or force it via vulkan layer in `CreateSampler` by adding to `pCreateInfo->mipLodBias` value."sv;
  ImGui::SameLine();
  ImHlp::Hint(mhint);

  ImGui::SetNextItemWidth(100);
  ImGui::Combo("Preset", &plimit, rdb::UpscalingPresetName.data(), rdb::UpscalingPresetName.size());

  if (prev_p != plimit) {
    to_W = g_nNestedWidth * UpscalingPresetValue.at(plimit);
    to_H = g_nNestedHeight * UpscalingPresetValue.at(plimit);
    prev_p = plimit;
  }
}
void UiLayoutOutputResolution(CSDLAction* pAction) {
  using namespace rdb;
  using namespace std::literals;

  static int hlimit = rdb::Height_FullD;
  static int dlimit = rdb::Divisor_16;
  static int alimit = rdb::W16_H9;

  auto max_h = rdb::HeightValue.at(hlimit);
  auto min_d = rdb::DivisorValue.at(dlimit);
  auto use_a = rdb::AspectRatioValue.at(alimit);

  static int prev_h = hlimit;
  static int prev_d = dlimit;
  static int prev_a = alimit;

  // Resolution list
  static auto list = rdb::data | where::aspect_ratio(use_a) | min::divisor(min_d) | max::height(max_h) | get::resolution | as::vector;

  // Find current resolution
  static auto res_current = [] {
    auto out_res = std::vformat("{} x {}", std::make_format_args(g_nOutputWidth, g_nOutputHeight));
    auto it = std::ranges::find(list, out_res);
    if (it != list.end()) {
      return static_cast<int>(std::ranges::distance(list.begin(), it));
    }
    return 0;
  };

  // TODO: find current / change on filter change / find fit
  static int rcheck = res_current();
  static int rcontrol = rcheck;

  // Update list on filter change
  if (prev_h != hlimit || prev_d != dlimit || prev_a != alimit) {
    list = rdb::data | where::aspect_ratio(use_a) | min::divisor(min_d) | max::height(max_h) | get::resolution | as::vector;
    prev_h = hlimit;
    prev_d = dlimit;
    prev_a = alimit;
    // Track current resolution
    rcheck = res_current();
    // Prevent resolution change on list change
    rcontrol = rcheck;
  }

  // Output resolution
  ImGui::SetNextItemWidth(100);
  ImGui::Combo("Output", &rcheck, list.data(), static_cast<int>(list.capacity()));
  if (rcheck != rcontrol) {
    auto resolution_data = rdb::data | where::aspect_ratio(use_a) | min::divisor(min_d) | max::height(max_h) | as::vector;
    if (resolution_data.capacity() > 0) {
      auto selected = resolution_data.at(rcheck);
      pAction->SetWindowResolution(selected.width, selected.height);
    }
    rcontrol = rcheck;
  }
  // Aspect ratio
  ImGui::SetNextItemWidth(100);
  ImGui::Combo("Aspect Ratio", &alimit, rdb::AspectRatioName.data(), rdb::AspectRatioName.size());

  auto sliderFlags = ImGuiSliderFlags_NoInput; // disable Ctrl+Click

  static auto const hhint = "Hide resolutions higher than [height]"sv;
  ImGui::SetNextItemWidth(100);
  ImGui::SliderInt("Max Height", &hlimit, 0, rdb::Height_COUNT - 1, rdb::HeightName.at(hlimit), sliderFlags);
  ImGui::SameLine();
  ImHlp::Hint(hhint);

  static auto const dhint = "Both Width and Height are divisible by [divisor]"sv;
  ImGui::SetNextItemWidth(100);
  ImGui::SliderInt("Min Divisor", &dlimit, 0, rdb::Divisor_COUNT - 1, rdb::DivisorName.at(dlimit), sliderFlags);
  ImGui::SameLine();
  ImHlp::Hint(dhint);
}

void UiLayoutSettingsTab(CSDLAction* pAction) {
  ImGui::BeginGroup();
  ImFmt::Text("Settings");
  // Resolutions
  ImFmt::Text("Nested {}x{}", g_nNestedWidth, g_nNestedHeight);
  ImGui::SameLine();
  ImFmt::Text("->");
  ImGui::SameLine();
  ImFmt::Text("{}x{} Output", g_nOutputWidth, g_nOutputHeight);

  UiLayoutOutputResolution(pAction);

  // Window control
  ImTpl::Toggle("Fullscreen", g_bFullscreen, [&pAction] { pAction->ToggleFullscreen(); });
  ImTpl::Toggle("Borderless", g_bBorderlessOutputWindow, [&pAction] { pAction->ToggleBorderless(); });
  ImGui::EndGroup();

  ImGui::SameLine();
  ImGui::Dummy({50, 50});
  ImGui::SameLine();

  ImGui::BeginGroup();
  ImTpl::Toggle("Fullscreen windows", g_bForceWindowsFullscreen, [] { CSDLAction::ToggleWindowsFullscreen(); });
  ImGui::Separator();
  ImFmt::Text("Filters");
  // Upscale Filter
  ImTpl::Select("Upscale Filter", g_wantedUpscaleFilter, GamescopeUpscaleFilterName, [](auto value) {
    CSDLAction::SetUpscaleFilter(static_cast<GamescopeUpscaleFilter>(value));
  });
  UiLayoutUpscaleFilterPreset();
  // Upscale Filter Sharpness
  UiLayoutUpscaleFilterSharpnessStrenght();
  // Sharpness
  static int sharpness = g_upscaleFilterSharpness;
  ImGui::SetNextItemWidth(100);
  ImGui::SliderInt("Filter Sharpness", &sharpness, defaults::minFSRSharpness, defaults::maxFSRSharpness, "%d");
  if (sharpness != g_upscaleFilterSharpness) {
    CSDLAction::SetUpscaleFilterSharpness(sharpness);
  }
  // Upscale Scaler
  ImTpl::Select("Upscale Scaler", g_wantedUpscaleScaler, GamescopeUpscaleScalerName, [](auto value) {
    CSDLAction::SetUpscaleScaler(static_cast<GamescopeUpscaleScaler>(value));
  });
  ImGui::EndGroup();

  ImGui::Separator();

  ImGui::BeginGroup();
  ImFmt::Text("Input");
  ImTpl::Toggle("Force Relative Mouse", g_bForceRelativeMouse, [&pAction] { pAction->ToggleMouseGrab(); });
  ImTpl::Toggle("Keyboard Grabbed", g_bGrabbed, [&pAction] { pAction->ToggleKeyboardGrab(); });
  // Mouse Sensitivity
  // TODO: figure out how this should work
  static float sensitivity = g_mouseSensitivity;
  ImGui::SetNextItemWidth(100);
  ImGui::SliderFloat("Mouse Sensitivity", &sensitivity, -1.0f, 1.0f, "%.3f");
  if (sensitivity != g_mouseSensitivity) {
    g_mouseSensitivity = sensitivity;
  }
  ImGui::EndGroup();
}
void UiLayoutHdrTab() {
  ImGui::BeginGroup();
  ImFmt::Text("HDR");
  // TODO
  ImTpl::Toggle("HDR Enabled", g_bOutputHDREnabled, [] { g_bOutputHDREnabled = !g_bOutputHDREnabled; });
  ImGui::EndGroup();
}
void UiLayoutMainTabs(CSDLAction* p_Action) {
  ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
  if (ImGui::BeginTabBar("MainTabBar", tab_bar_flags)) {
    if (ImGui::BeginTabItem("Settings")) {
      UiLayoutSettingsTab(p_Action);
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Hdr")) {
      UiLayoutHdrTab();
      ImGui::EndTabItem();
    }
#if !defined(NDEBUG)
    if (ImGui::BeginTabItem("Debug")) {
      UiLayoutDebugTab();
      ImGui::EndTabItem();
    }
#endif
    if (ImGui::BeginTabItem("Build")) {
      UiLayoutBuildTab();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("About")) {
      UiLayoutAboutTab();
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }
}
} // namespace

void CSDLConnector::UiLayout() {
  // ImGui::ShowDemoWindow();
  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoSavedSettings;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_HorizontalScrollbar;
  flags |= ImGuiWindowFlags_AlwaysVerticalScrollbar;

  // No close button
  bool* close = nullptr;

  if (ImGui::Begin("Settings", close, flags)) {
    // TODO:
    ImGui::SetWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::SetWindowPos({0, 0});
    // static ImGuiIO& io = ImGui::GetIO();
    ImFmt::Text("Average {:.3f} ms/frame ({:.1f} FPS)", toms / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Separator();
    UiLayoutMainTabs(Action());
  }
  ImGui::End();
}

void CSDLConnector::UiRender() {
  // Rendering
  ImGui::Render();
}

void CSDLConnector::UiPresent() {
  // SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
  // SDL_SetRenderDrawColorFloat(renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
  SDL_SetRenderDrawColorFloat(m_pUiRenderer, 0.5, 0.5, 0.5, SDL_ALPHA_OPAQUE_FLOAT); /* new color, full alpha. */
  SDL_RenderClear(m_pUiRenderer);
  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_pUiRenderer);
  SDL_RenderPresent(m_pUiRenderer);
}

void CSDLConnector::UiShutdown() {
  // Cleanup
  ImGui_ImplSDLRenderer3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
}

auto CSDLConnector::IsUiHidden() -> bool { return ((SDL_GetWindowFlags(m_pPopup) & SDL_WINDOW_HIDDEN) != 0u); }
void CSDLConnector::UiShow(bool bShow) { bShow ? SDL_ShowWindow(m_pPopup) : SDL_HideWindow(m_pPopup); }

void CSDLConnector::ListRendererDrivers() {
  for (int i = 0; i < SDL_GetNumRenderDrivers(); i++) {
    std::println("{}", SDL_GetRenderDriver(i));
  }
}
} // namespace gamescope
