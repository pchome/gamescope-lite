
#include <SDL3/SDL_clipboard.h>
#include <SDL3/SDL_events.h>

#include "sdlscancodetable.hpp"

#include <algorithm>

#include "../sdl_backend.hpp"
#include "./input.hpp"

#include "main.hpp"
#include "steamcompmgr.hpp"
#include "wlserver.hpp"

namespace gamescope {

void CSDLBackend::HandleInputEvent(SDL_Event event, uint32_t fake_timestamp) {
  switch (event.type) {
  case SDL_EVENT_CLIPBOARD_UPDATE: {
    char *pClipBoard = SDL_GetClipboardText();
    char *pPrimarySelection = SDL_GetPrimarySelectionText();

    gamescope_set_selection(pClipBoard, GAMESCOPE_SELECTION_CLIPBOARD);
    gamescope_set_selection(pPrimarySelection, GAMESCOPE_SELECTION_PRIMARY);

    SDL_free(pClipBoard);
    SDL_free(pPrimarySelection);
  } break;

  case SDL_EVENT_MOUSE_MOTION: {
    if (m_bApplicationGrabbed) {
      if (g_bWindowFocused) {
        wlserver_lock();
        wlserver_mousemotion(event.motion.xrel, event.motion.yrel, fake_timestamp);
        wlserver_unlock();
      }
    } else {
      wlserver_lock();
      wlserver_touchmotion(event.motion.x / double(g_nOutputWidthPts), event.motion.y / double(g_nOutputHeightPts), 0,
                           fake_timestamp);
      wlserver_unlock();
    }
  } break;

  case SDL_EVENT_MOUSE_BUTTON_DOWN:
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    wlserver_lock();
    wlserver_mousebutton(SDLButtonToLinuxButton(event.button.button), event.button.down, fake_timestamp);
    wlserver_unlock();
  } break;

  case SDL_EVENT_MOUSE_WHEEL: {
    wlserver_lock();
    wlserver_mousewheel(-event.wheel.x, -event.wheel.y, fake_timestamp);
    wlserver_unlock();
  } break;

  case SDL_EVENT_FINGER_MOTION: {
    wlserver_lock();
    wlserver_touchmotion(event.tfinger.x, event.tfinger.y, event.tfinger.fingerID, fake_timestamp);
    wlserver_unlock();
  } break;

  case SDL_EVENT_FINGER_DOWN: {
    wlserver_lock();
    wlserver_touchdown(event.tfinger.x, event.tfinger.y, event.tfinger.fingerID, fake_timestamp);
    wlserver_unlock();
  } break;

  case SDL_EVENT_FINGER_UP: {
    wlserver_lock();
    wlserver_touchup(event.tfinger.fingerID, fake_timestamp);
    wlserver_unlock();
  } break;

  case SDL_EVENT_KEY_DOWN: {
    // If this keydown event is super + one of the shortcut keys, consume the keydown event, since the corresponding
    // keyup event will be consumed by the next case statement when the user releases the key
    if ((event.key.mod & SDL_KMOD_LGUI) != 0) {
      uint32_t key = SDLScancodeToLinuxKey(event.key.scancode);
      const std::array<uint32_t, 10> shortcutKeys = {KEY_F, KEY_N, KEY_B, KEY_K, KEY_U,
                                                     KEY_Y, KEY_I, KEY_O, KEY_S, KEY_G};
      if (std::ranges::find(shortcutKeys, key) != std::end(shortcutKeys)) {
        break;
      }
    }
  }
    [[fallthrough]];
  case SDL_EVENT_KEY_UP: {
    uint32_t key = SDLScancodeToLinuxKey(event.key.scancode);

    if (event.type == SDL_EVENT_KEY_UP && ((event.key.mod & SDL_KMOD_LGUI) != 0)) {
      bool handled = true;
      switch (key) {
      case KEY_F:
        g_bFullscreen = !g_bFullscreen;
        SDL_SetWindowFullscreen(m_Connector.GetSDLWindow(), g_bFullscreen);
        break;
      case KEY_N:
        g_wantedUpscaleFilter = GamescopeUpscaleFilter::PIXEL;
        break;
      case KEY_B:
        g_wantedUpscaleFilter = GamescopeUpscaleFilter::LINEAR;
        break;
      case KEY_K:
        g_wantedDownscaleFilter = (g_wantedDownscaleFilter == GamescopeDownscaleFilter::BICUBIC)
                                      ? GamescopeDownscaleFilter::LINEAR
                                      : GamescopeDownscaleFilter::BICUBIC;
        break;
      case KEY_U:
        g_wantedUpscaleFilter = (g_wantedUpscaleFilter == GamescopeUpscaleFilter::FSR) ? GamescopeUpscaleFilter::LINEAR
                                                                                       : GamescopeUpscaleFilter::FSR;
        break;
      case KEY_Y:
        g_wantedUpscaleFilter = (g_wantedUpscaleFilter == GamescopeUpscaleFilter::NIS) ? GamescopeUpscaleFilter::LINEAR
                                                                                       : GamescopeUpscaleFilter::NIS;
        break;
      case KEY_I:
        g_upscaleFilterSharpness = std::min(20, g_upscaleFilterSharpness + 1);
        break;
      case KEY_O:
        g_upscaleFilterSharpness = std::max(0, g_upscaleFilterSharpness - 1);
        break;
      case KEY_S:
        gamescope::CScreenshotManager::Get().TakeScreenshot(true);
        break;
      case KEY_G:
        g_bGrabbed = !g_bGrabbed;
        SDL_SetWindowKeyboardGrab(m_Connector.GetSDLWindow(), g_bGrabbed);

        SDL_Event event;
        event.type = GetUserEventIndex(GAMESCOPE_SDL_EVENT_TITLE);
        SDL_PushEvent(&event);
        break;
      default:
        handled = false;
      }
      if (handled) {
        break;
      }
    }

    // On Wayland, clients handle key repetition
    if (event.key.repeat) {
      break;
    }

    wlserver_lock();
    wlserver_key(key, event.type == SDL_EVENT_KEY_DOWN, fake_timestamp);
    wlserver_unlock();
  } break;

  default:
    break;
  }
}

} // namespace gamescope
