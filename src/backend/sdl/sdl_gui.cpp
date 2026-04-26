#include <SDL3/SDL_render.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include "sdl_connector.hpp"
#include "sdl_gui.hpp"

#include "GamescopeVersion.h"
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
void UiLayoutSettingsTab(SDL_Window* main_window) {
  ImGui::SeparatorText("Settings");
  ImGui::Checkbox("HDR Enabled", &g_bOutputHDREnabled);
  ImGui::Checkbox("Force Internal", &g_bForceInternal);
  static bool fullscreen = g_bFullscreen;
  ImGui::Checkbox("Fullscreen", &fullscreen);
  if (fullscreen != g_bFullscreen) {
    g_bFullscreen = !g_bFullscreen;
    SDL_SetWindowFullscreen(main_window, g_bFullscreen);
  }
  static bool relative = g_bForceRelativeMouse;
  ImGui::Checkbox("Force Relative Mouse", &relative);
  if (relative != g_bForceRelativeMouse) {
    g_bForceRelativeMouse = !g_bForceRelativeMouse;
    SDL_SetWindowRelativeMouseMode(main_window, g_bForceRelativeMouse);
    // m_bApplicationGrabbed = g_bForceRelativeMouse;
  }

  static bool grabbed = g_bGrabbed;
  ImGui::Checkbox("Keyboard Grabbed", &grabbed);

  ImGui::SliderFloat("Mouse Sensitivity", &g_mouseSensitivity, 0.0f, 2.0f, "%.3f");
}
void UiLayoutMainTabs(SDL_Window* main_window) {
  ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
  if (ImGui::BeginTabBar("MainTabBar", tab_bar_flags)) {
    if (ImGui::BeginTabItem("Settings")) {
      UiLayoutSettingsTab(main_window);
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

  // No close button
  bool* close = nullptr;

  if (ImGui::Begin("Settings", close, flags)) {
    // TODO:
    ImGui::SetWindowSize({640, 360});
    ImGui::SetWindowPos({0, 0});
    // static ImGuiIO& io = ImGui::GetIO();
    ImFmt::Text("TODO: some useful shit.");
    ImFmt::Text("Average {:.3f} ms/frame ({:.1f} FPS)", toms / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Separator();
    UiLayoutMainTabs(m_pWindow);
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
