#include <SDL3/SDL_events.h>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_surface.h>

#include "../sdl_backend.hpp"
#include "./custom.hpp"

#include "main.hpp"
#include "steamcompmgr.hpp"

namespace gamescope {

void CSDLBackend::SwitchMainWindowVisibility() {
  bool bVisible = m_bApplicationVisible;

  // If we are Steam Mode in nested, show the window
  // whenever we have had a first frame to match
  // what we do in embedded with Steam for testing
  // held commits, etc.
  if (steamMode) {
    bVisible |= !g_bFirstFrame;
  }

  if (m_bShown != bVisible) {
    m_bShown = bVisible;

    if (m_bShown) {
      SDL_ShowWindow(m_Connector.GetSDLWindow());
    } else {
      SDL_HideWindow(m_Connector.GetSDLWindow());
    }
  }
}

void CSDLBackend::SwitchKeyboardGrabIndicator() {
  std::shared_ptr<std::string> pAppTitle = m_pApplicationTitle;

  std::string szTitle = pAppTitle ? *pAppTitle : "Gamescope Lite";
  if (g_bGrabbed) {
    szTitle += " (grabbed)";
  }
  SDL_SetWindowTitle(m_Connector.GetSDLWindow(), szTitle.c_str());

  szTitle = "Title: " + szTitle;
  SDL_SetHint(SDL_HINT_SCREENSAVER_INHIBIT_ACTIVITY_NAME, szTitle.c_str());
  SDL_DisableScreenSaver();
}

void CSDLBackend::UseApplicationIcon() {
  std::shared_ptr<std::vector<uint32_t>> pIcon = m_pApplicationIcon;

  if (m_pIconSurface != nullptr) {
    SDL_DestroySurface(m_pIconSurface);
    m_pIconSurface = nullptr;
  }

  if (pIcon && pIcon->size() >= 3) {
    const auto uWidth = static_cast<int>((*pIcon)[0]);
    const auto uHeight = static_cast<int>((*pIcon)[1]);
    const auto uPitch = static_cast<int>(uWidth * sizeof(uint32_t));
    using namespace sdl::surface;
    auto uFormat = SDL_PixelFormat::SDL_PIXELFORMAT_RGBA8888;
    m_pCursorSurface = SDL_CreateSurfaceFrom(uWidth, uHeight, uFormat, &(*pIcon)[2], uPitch);
  }

  SDL_SetWindowIcon(m_Connector.GetSDLWindow(), m_pIconSurface);
}

void CSDLBackend::UseApplicationCursor() {
  std::shared_ptr<INestedHints::CursorInfo> pCursorInfo = m_pApplicationCursor;

  if (m_pCursorSurface != nullptr) {
    SDL_DestroySurface(m_pCursorSurface);
    m_pCursorSurface = nullptr;
  }

  if (m_pCursor != nullptr) {
    SDL_DestroyCursor(m_pCursor);
    m_pCursor = nullptr;
  }

  if (pCursorInfo) {
    auto *const uPixels = pCursorInfo->pPixels.data();
    const auto uWidth = static_cast<int>(pCursorInfo->uWidth);
    const auto uHeight = static_cast<int>(pCursorInfo->uHeight);
    const auto uPitch = static_cast<int>(uWidth * sizeof(uint32_t));
    // using namespace sdl::surface;
    // auto uFormat = SDL_GetPixelFormatForMasks(depth, Rmask, Gmask, Bmask, Amask);
    auto uFormat = SDL_PixelFormat::SDL_PIXELFORMAT_RGBA8888;
    m_pCursorSurface = SDL_CreateSurfaceFrom(uWidth, uHeight, uFormat, uPixels, uPitch);
    auto uHot_X = static_cast<int>(pCursorInfo->uXHotspot);
    auto uHot_Y = static_cast<int>(pCursorInfo->uYHotspot);
    m_pCursor = SDL_CreateColorCursor(m_pCursorSurface, uHot_X, uHot_Y);
  }

  SDL_SetCursor(m_pCursor);
}

void CSDLBackend::HandleUserEvent(SDL_Event event) {
  if (event.type == GetUserEventIndex(GAMESCOPE_SDL_EVENT_VISIBLE)) {
    SwitchMainWindowVisibility();
    return;
  }
  if (event.type == GetUserEventIndex(GAMESCOPE_SDL_EVENT_TITLE)) {
    SwitchKeyboardGrabIndicator();
    return;
  }
  if (event.type == GetUserEventIndex(GAMESCOPE_SDL_EVENT_ICON)) {
    UseApplicationIcon();
    return;
  }
  if (event.type == GetUserEventIndex(GAMESCOPE_SDL_EVENT_GRAB)) {
    SDL_SetWindowRelativeMouseMode(m_Connector.GetSDLWindow(), m_bApplicationGrabbed);
    return;
  }
  if (event.type == GetUserEventIndex(GAMESCOPE_SDL_EVENT_CURSOR)) {
    UseApplicationCursor();
    return;
  }
  if (event.type == GetUserEventIndex(GAMESCOPE_SDL_EVENT_REQ_EXIT)) {
    return;
  }
}
} // namespace gamescope
