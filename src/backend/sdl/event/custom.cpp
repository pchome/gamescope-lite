#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>

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
    SDL_FreeSurface(m_pIconSurface);
    m_pIconSurface = nullptr;
  }

  if (pIcon && pIcon->size() >= 3) {
    const uint32_t uWidth = (*pIcon)[0];
    const uint32_t uHeight = (*pIcon)[1];
    const uint32_t uPitch = uWidth * sizeof(uint32_t);
    using namespace sdl::surface;
    m_pIconSurface = SDL_CreateRGBSurfaceFrom(&(*pIcon)[2], uWidth, uHeight, depth, uPitch, Rmask, Gmask, Bmask, Amask);
  }

  SDL_SetWindowIcon(m_Connector.GetSDLWindow(), m_pIconSurface);
}

void CSDLBackend::UseApplicationCursor() {
  std::shared_ptr<INestedHints::CursorInfo> pCursorInfo = m_pApplicationCursor;

  if (m_pCursorSurface != nullptr) {
    SDL_FreeSurface(m_pCursorSurface);
    m_pCursorSurface = nullptr;
  }

  if (m_pCursor != nullptr) {
    SDL_FreeCursor(m_pCursor);
    m_pCursor = nullptr;
  }

  if (pCursorInfo) {
    auto *const uPixels = pCursorInfo->pPixels.data();
    const uint32_t uWidth = pCursorInfo->uWidth;
    const uint32_t uHeight = pCursorInfo->uHeight;
    const uint32_t uPitch = uWidth * sizeof(uint32_t);
    using namespace sdl::surface;
    m_pCursorSurface = SDL_CreateRGBSurfaceFrom(uPixels, uWidth, uHeight, depth, uPitch, Rmask, Gmask, Bmask, Amask);

    m_pCursor = SDL_CreateColorCursor(m_pCursorSurface, pCursorInfo->uXHotspot, pCursorInfo->uYHotspot);
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
    SDL_SetRelativeMouseMode(m_bApplicationGrabbed ? SDL_TRUE : SDL_FALSE);
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
