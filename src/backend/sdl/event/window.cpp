#include <SDL3/SDL_events.h>
#include <SDL3/SDL_version.h>
#include <SDL3/SDL_video.h>

#include <csignal>

#include "./window.hpp"
#include "backend/sdl/sdl_backend.hpp"

#include "main.hpp"
#include "refresh_rate.h"
#include "steamcompmgr.hpp"

namespace gamescope {

auto CSDLBackend::HandleWindowEvent(SDL_Event event) -> bool {
  auto handled = false;

  switch (event.window.type) {
  case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
    handled = true;
    raise(SIGTERM);
    break;
  default:
    break;
  case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
    handled = true;
    int width;
    int height;
    SDL_GetWindowSize(m_Connector.GetSDLWindow(), &width, &height);
    g_nOutputWidthPts = width;
    g_nOutputHeightPts = height;

#if SDL_VERSION_ATLEAST(2, 26, 0)
    SDL_GetWindowSizeInPixels(m_Connector.GetSDLWindow(), &width, &height);
#endif
    g_nOutputWidth = width;
    g_nOutputHeight = height;

    [[fallthrough]];
  case SDL_EVENT_WINDOW_MOVED:
  case SDL_EVENT_WINDOW_SHOWN: {
    handled = true;
    SDL_DisplayID display_index = 0;
    SDL_DisplayMode const* mode = nullptr;
    display_index = SDL_GetDisplayForWindow(m_Connector.GetSDLWindow());
    mode = SDL_GetDesktopDisplayMode(display_index);
    if (mode != nullptr) {
      g_nOutputRefresh = ConvertHztomHz(mode->refresh_rate);
    }
  } break;
  case SDL_EVENT_WINDOW_FOCUS_LOST:
    handled = true;
    g_nNestedRefresh = g_nNestedUnfocusedRefresh;
    g_bWindowFocused = false;
    break;
  case SDL_EVENT_WINDOW_FOCUS_GAINED:
    handled = true;
    g_nNestedRefresh = g_nOldNestedRefresh;
    g_bWindowFocused = true;
    break;
  case SDL_EVENT_WINDOW_EXPOSED:
    handled = true;
    force_repaint();
    break;
  }

  return handled;
}

} // namespace gamescope
