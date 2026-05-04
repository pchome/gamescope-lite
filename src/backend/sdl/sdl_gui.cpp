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

#include <print>

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
void UiLayoutSettingsTab(CSDLAction* pAction) {
  ImGui::BeginGroup();
  ImFmt::Text("Settings");
  // Resolutions
  ImFmt::Text("{}x{}", g_nNestedWidth, g_nNestedHeight);
  ImGui::SameLine();
  ImFmt::Text("->");
  ImGui::SameLine();
  ImFmt::Text("{}x{}", g_nOutputWidth, g_nOutputHeight);
  // Window control
  ImTpl::Toggle("Fullscreen", g_bFullscreen, [&pAction] { pAction->ToggleFullscreen(); });
  ImTpl::Toggle("Borderless", g_bBorderlessOutputWindow, [&pAction] { pAction->ToggleBorderless(); });
  ImGui::EndGroup();

  ImGui::SameLine();
  ImGui::Dummy({50, 50});
  ImGui::SameLine();

  ImGui::BeginGroup();
  ImFmt::Text("Filters");
  // Upscale Filter
  std::array<char const*, 4> filters = {"LINEAR", "PIXEL", "FSR", "NIS"};
  static int filter_current = static_cast<int>(g_wantedUpscaleFilter);
  ImGui::SetNextItemWidth(100);
  ImGui::Combo("Upscale Filter", &filter_current, filters.data(), filters.size());
  if (filter_current != static_cast<int>(g_wantedUpscaleFilter)) {
    gamescope::CSDLAction::SetUpscaleFilter(static_cast<GamescopeUpscaleFilter>(filter_current));
  }
  // Upscale Filter Sharpness
  struct Funcs {
    static auto Red(void* /*unused*/, int i) -> float { return -sinf(i * 0.1f); }
    static auto Green(void* /*unused*/, int i) -> float { return (1.0f - (i * 0.1f)); }
    static auto None(void* /*unused*/, int i) -> float { return i * 0.0f; }
  };
  static int func_type = 0;
  static int display_offset = defaults::minFSRSharpness;
  static int display_count = defaults::maxFSRSharpness - 3; //+ 1;
  func_type = (filter_current == 3) ? 1 : (filter_current == 2) ? 0 : 2;
  float (*func)(void*, int) = (func_type == 0) ? Funcs::Red : (func_type == 1) ? Funcs::Green : Funcs::None;
  ImGui::BeginDisabled();
  ImGui::SetNextItemWidth((ImGui::GetFontSize() * 8) - 4);
  ImGui::PlotLines("Strenght", func, nullptr, display_count, display_offset, nullptr, -1.0f, 1.0f, ImVec2(0, 20));
  ImGui::EndDisabled();
  // Sharpness
  static int sharpness = g_upscaleFilterSharpness;
  ImGui::SetNextItemWidth(100);
  ImGui::SliderInt("Filter Sharpness", &sharpness, defaults::minFSRSharpness, defaults::maxFSRSharpness, "%d");
  if (sharpness != g_upscaleFilterSharpness) {
    gamescope::CSDLAction::SetUpscaleFilterSharpness(sharpness);
  }
  // Upscale Scaler
  std::array<char const*, 6> scalers = {"AUTO", "INTEGER", "FIT", "FILL", "STRETCH", "NATIVE"};
  static int scler_current = static_cast<int>(g_wantedUpscaleScaler);
  ImGui::SetNextItemWidth(100);
  ImGui::Combo("Upscale Scaler", &scler_current, scalers.data(), scalers.size());
  if (scler_current != static_cast<int>(g_wantedUpscaleScaler)) {
    gamescope::CSDLAction::SetUpscaleScaler(static_cast<GamescopeUpscaleScaler>(scler_current));
  }
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
    if (ImGui::BeginTabItem("Debug")) {
      UiLayoutDebugTab();
      ImGui::EndTabItem();
    }
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
