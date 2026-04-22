#include <SDL2/SDL_events.h>
#include <SDL2/SDL_version.h>
#include <SDL2/SDL_video.h>

#include <csignal>

#include "../sdl_backend.hpp"
#include "./window.hpp"

#include "main.hpp"
#include "refresh_rate.h"
#include "steamcompmgr.hpp"

namespace gamescope {

void CSDLBackend::HandleWindowEvent(SDL_Event event) {
  switch (event.window.event) {
  case SDL_WINDOWEVENT_CLOSE:
    raise(SIGTERM);
    break;
  default:
    break;
  case SDL_WINDOWEVENT_SIZE_CHANGED:
    int width, height;
    SDL_GetWindowSize(m_Connector.GetSDLWindow(), &width, &height);
    g_nOutputWidthPts = width;
    g_nOutputHeightPts = height;

#if SDL_VERSION_ATLEAST(2, 26, 0)
    SDL_GetWindowSizeInPixels(m_Connector.GetSDLWindow(), &width, &height);
#endif
    g_nOutputWidth = width;
    g_nOutputHeight = height;

    [[fallthrough]];
  case SDL_WINDOWEVENT_MOVED:
  case SDL_WINDOWEVENT_SHOWN: {
    int display_index = 0;
    SDL_DisplayMode mode = {SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, nullptr};

    display_index = SDL_GetWindowDisplayIndex(m_Connector.GetSDLWindow());
    if (SDL_GetDesktopDisplayMode(display_index, &mode) == 0) {
      g_nOutputRefresh = ConvertHztomHz(mode.refresh_rate);
    }
  } break;
  case SDL_WINDOWEVENT_FOCUS_LOST:
    g_nNestedRefresh = g_nNestedUnfocusedRefresh;
    g_bWindowFocused = false;
    break;
  case SDL_WINDOWEVENT_FOCUS_GAINED:
    g_nNestedRefresh = g_nOldNestedRefresh;
    g_bWindowFocused = true;
    break;
  case SDL_WINDOWEVENT_EXPOSED:
    force_repaint();
    break;
  }
}

} // namespace gamescope
