#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>

#include "backend/sdl/sdl_connector.hpp"

#include "./sdl_action.hpp"
#include "GamescopeVersion.h"
#include "constants_include.hpp"
#include "main.hpp"

#include <algorithm>

namespace gamescope {
//////////////////
// CSDLAction
//////////////////

CSDLAction::CSDLAction(CSDLConnector* pConnector)
    : m_pConnector{pConnector} {}

void CSDLAction::TogglePopup() {
  auto flags = SDL_GetWindowFlags(m_pConnector->GetPopupWindow());
  m_pConnector->UiShow((flags & SDL_WINDOW_HIDDEN) != 0u);
}

void CSDLAction::ToggleFullscreen() {
  g_bFullscreen = !g_bFullscreen;
  SDL_SetWindowFullscreen(m_pConnector->GetSDLWindow(), g_bFullscreen);
}

void CSDLAction::SetWindowTitle(std::string const& title) {
  SDL_SetWindowTitle(m_pConnector->GetSDLWindow(), title.c_str());
  SDL_SetHint(SDL_HINT_SCREENSAVER_INHIBIT_ACTIVITY_NAME, ("Title: " + title).c_str());
  SDL_DisableScreenSaver();
}

void CSDLAction::ToggleKeyboardGrab() {
  g_bGrabbed = !g_bGrabbed;
  SDL_SetWindowKeyboardGrab(m_pConnector->GetSDLWindow(), g_bGrabbed);
  auto pAppTitle = m_pConnector->GetTitle();
  std::string title = pAppTitle ? *pAppTitle : gamescopeName;
  if (g_bGrabbed) {
    title += g_bForceRelativeMouse ? " (⌨️🖱️)" : " (⌨️)";
  } else {
    title += g_bForceRelativeMouse ? " (🖱️)" : "";
  }
  SetWindowTitle(title);
}

void CSDLAction::ToggleMouseGrab() {
  g_bForceRelativeMouse = !g_bForceRelativeMouse;
  SDL_SetWindowRelativeMouseMode(m_pConnector->GetSDLWindow(), g_bForceRelativeMouse);
  // SDL_SetWindowMouseGrab(m_pConnector->GetSDLWindow(), g_bForceRelativeMouse);
  auto pAppTitle = m_pConnector->GetTitle();
  std::string title = pAppTitle ? *pAppTitle : gamescopeName;
  if (g_bForceRelativeMouse) {
    title += g_bGrabbed ? " (⌨️🖱️)" : " (🖱️)";
  } else {
    title += g_bGrabbed ? " (⌨️)" : "";
  }
  SetWindowTitle(title);
}

void CSDLAction::SetUpscaleFilter(GamescopeUpscaleFilter filter) { g_wantedUpscaleFilter = filter; }
void CSDLAction::SetDownscaleFilter(GamescopeDownscaleFilter filter) { g_wantedDownscaleFilter = filter; }
void CSDLAction::SetUpscaleScaler(GamescopeUpscaleScaler scaler) { g_wantedUpscaleScaler = scaler; }

void CSDLAction::SetUpscaleFilterSharpness(int sharpness) {
  g_upscaleFilterSharpness = std::ranges::clamp(defaults::minFSRSharpness, defaults::maxFSRSharpness, sharpness);
}
void CSDLAction::IncUpscaleFilterSharpness() {
  g_upscaleFilterSharpness = std::ranges::min(defaults::maxFSRSharpness, g_upscaleFilterSharpness + 1);
}
void CSDLAction::DecUpscaleFilterSharpness() {
  g_upscaleFilterSharpness = std::ranges::max(defaults::minFSRSharpness, g_upscaleFilterSharpness - 1);
}
void CSDLAction::ToggleUpscaleFilter() {
  g_wantedUpscaleFilter = (g_wantedUpscaleFilter == GamescopeUpscaleFilter::FSR)   ? GamescopeUpscaleFilter::NIS
                          : (g_wantedUpscaleFilter == GamescopeUpscaleFilter::NIS) ? GamescopeUpscaleFilter::LINEAR
                                                                                   : GamescopeUpscaleFilter::FSR;
}
void CSDLAction::ToggleDownscaleFilter() {
  g_wantedDownscaleFilter = (g_wantedDownscaleFilter == GamescopeDownscaleFilter::BICUBIC)
                                ? GamescopeDownscaleFilter::LINEAR
                                : GamescopeDownscaleFilter::BICUBIC;
}
void CSDLAction::ToggleUpscaleFilterFSR() {
  g_wantedUpscaleFilter = (g_wantedUpscaleFilter == GamescopeUpscaleFilter::FSR) ? GamescopeUpscaleFilter::LINEAR
                                                                                 : GamescopeUpscaleFilter::FSR;
}
void CSDLAction::ToggleUpscaleFilterNIS() {
  g_wantedUpscaleFilter = (g_wantedUpscaleFilter == GamescopeUpscaleFilter::NIS) ? GamescopeUpscaleFilter::LINEAR
                                                                                 : GamescopeUpscaleFilter::NIS;
}

} // namespace gamescope
