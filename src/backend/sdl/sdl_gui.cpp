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
  ImFmt::Text("Nested: {}x{}@{}", g_nNestedWidth, g_nNestedHeight, g_nNestedRefresh / toHz);
  ImFmt::Text("Output: {}x{}@{}", g_nOutputWidth, g_nOutputHeight, g_nOutputRefresh / toHz);
  ImFmt::Text("Aspect: {}", g_aspectRatio);

  ImGui::SeparatorText("Nested");
  ImFmt::Text("Unfocused Refresh: {}", g_nNestedUnfocusedRefresh / toHz);
  ImFmt::Text("Display Index: {}", g_nNestedDisplayIndex);

  ImGui::SeparatorText("Output");
  ImGui::BeginDisabled();
  auto check = !!(g_bOutputHDREnabled);
  ImGui::Checkbox("HDR Enabled", &check);
  auto check4 = !!(g_bForceInternal);
  ImGui::Checkbox("Force Internal", &check4);
  auto check5 = !!(g_bFullscreen);
  ImGui::Checkbox("Fullscreen", &check5);
  ImGui::EndDisabled();

  ImGui::SeparatorText("Input");
  ImGui::BeginDisabled();
  auto check2 = !!(g_bForceRelativeMouse);
  ImGui::Checkbox("Force Relative Mouse", &check2);
  auto check3 = !!(g_bGrabbed);
  ImGui::Checkbox("Keyboard Grabbed", &check3);
  ImGui::EndDisabled();
  ImFmt::Text("Mouse Sensitivity: {}", g_mouseSensitivity);
}
void UiLayoutSettingsTab(CSDLAction* pAction) {
  ImGui::BeginGroup();
  ImFmt::Text("Settings");
  // TODO
  ImGui::Checkbox("HDR Enabled", &g_bOutputHDREnabled);
  ImGui::Checkbox("Force Internal", &g_bForceInternal);
  // Resolutions
  ImFmt::Text("{}x{}", g_nNestedWidth, g_nNestedHeight);
  ImGui::SameLine();
  ImFmt::Text("->");
  ImGui::SameLine();
  ImFmt::Text("{}x{}", g_nOutputWidth, g_nOutputHeight);
  // Set Fullscreen
  static bool fullscreen = g_bFullscreen;
  ImGui::Checkbox("Fullscreen", &fullscreen);
  if (fullscreen != g_bFullscreen) {
    pAction->ToggleFullscreen();
  }
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
  // Force Relative Mouse mode
  static bool relative = g_bForceRelativeMouse;
  ImGui::Checkbox("Force Relative Mouse", &relative);
  if (relative != g_bForceRelativeMouse) {
    pAction->ToggleMouseGrab();
  }
  // Grab Keyboard
  static bool grabbed = g_bGrabbed;
  ImGui::Checkbox("Keyboard Grabbed", &grabbed);
  if (relative != g_bGrabbed) {
    pAction->ToggleKeyboardGrab();
  }
  // Mouse Sensitivity
  static float sensitivity = g_mouseSensitivity;
  ImGui::SetNextItemWidth(100);
  ImGui::SliderFloat("Mouse Sensitivity", &sensitivity, -1.0f, 1.0f, "%.3f");
  if (sensitivity != g_mouseSensitivity) {
    g_mouseSensitivity = sensitivity;
  }
  ImGui::EndGroup();
}

void UiLayoutMainTabs(CSDLAction* p_Action) {
  ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
  if (ImGui::BeginTabBar("MainTabBar", tab_bar_flags)) {
    if (ImGui::BeginTabItem("Settings")) {
      UiLayoutSettingsTab(p_Action);
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
    ImFmt::Text("TODO: some useful shit.");
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
