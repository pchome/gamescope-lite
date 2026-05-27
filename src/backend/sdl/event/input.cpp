
#include <SDL3/SDL_clipboard.h>
#include <SDL3/SDL_events.h>

#include "sdlscancodetable.hpp"

#include <algorithm>

#include "./input.hpp"
#include "backend/sdl/sdl_backend.hpp"

#include "constants_include.hpp"
#include "main.hpp"

#include "steamcompmgr.hpp"
#include "wlserver.hpp"

namespace gamescope {

auto CSDLBackend::HandleInputEvent(SDL_Event event, uint32_t  /*fake_timestamp*/) -> bool {
  auto handled = false;

  switch (event.type) {
  case SDL_EVENT_CLIPBOARD_UPDATE: {
    handled = true;
    char* pClipBoard = SDL_GetClipboardText();
    char* pPrimarySelection = SDL_GetPrimarySelectionText();

    gamescope_set_selection(pClipBoard, GAMESCOPE_SELECTION_CLIPBOARD);
    gamescope_set_selection(pPrimarySelection, GAMESCOPE_SELECTION_PRIMARY);

    SDL_free(pClipBoard);
    SDL_free(pPrimarySelection);
  } break;

  case SDL_EVENT_MOUSE_MOTION: {
    handled = true;
    if (m_bApplicationGrabbed) {
      if (g_bWindowFocused) {
        wlserver_lock();
        wlserver_mousemotion(event.motion.xrel, event.motion.yrel, event.motion.timestamp);
        wlserver_unlock();
      }
    } else {
      wlserver_lock();
      wlserver_touchmotion(event.motion.x / double(g_nOutputWidthPts),
                           event.motion.y / double(g_nOutputHeightPts),
                           0,
                           event.motion.timestamp);
      wlserver_unlock();
    }
  } break;

  case SDL_EVENT_MOUSE_BUTTON_DOWN:
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    handled = true;
    wlserver_lock();
    wlserver_mousebutton(SDLButtonToLinuxButton(event.button.button), event.button.down, event.button.timestamp);
    wlserver_unlock();
  } break;

  case SDL_EVENT_MOUSE_WHEEL: {
    handled = true;
    wlserver_lock();
    wlserver_mousewheel(-event.wheel.x, -event.wheel.y, event.wheel.timestamp);
    wlserver_unlock();
  } break;

  case SDL_EVENT_FINGER_MOTION: {
    handled = true;
    wlserver_lock();
    wlserver_touchmotion(event.tfinger.x, event.tfinger.y, event.tfinger.fingerID, event.tfinger.timestamp);
    wlserver_unlock();
  } break;

  case SDL_EVENT_FINGER_DOWN: {
    handled = true;
    wlserver_lock();
    wlserver_touchdown(event.tfinger.x, event.tfinger.y, event.tfinger.fingerID, event.tfinger.timestamp);
    wlserver_unlock();
  } break;

  case SDL_EVENT_FINGER_UP: {
    handled = true;
    wlserver_lock();
    wlserver_touchup(event.tfinger.fingerID, event.tfinger.timestamp);
    wlserver_unlock();
  } break;

  case SDL_EVENT_KEY_DOWN: {
    // If this keydown event is super + one of the shortcut keys, consume the keydown event, since the corresponding
    // keyup event will be consumed by the next case statement when the user releases the key
    if ((event.key.mod & SDL_KMOD_LGUI) != 0) {
      uint32_t key = SDLScancodeToLinuxKey(event.key.scancode);
      std::array<uint32_t, 11> const shortcutKeys = {
          KEY_F, KEY_N, KEY_B, KEY_K, KEY_U, KEY_Y, KEY_I, KEY_O, KEY_S, KEY_G, KEY_GRAVE};
      if (std::ranges::find(shortcutKeys, key) != std::end(shortcutKeys)) {
        break;
      }
      handled = true;
    }
  }
    [[fallthrough]];
  case SDL_EVENT_KEY_UP: {
    uint32_t key = SDLScancodeToLinuxKey(event.key.scancode);

    if (event.type == SDL_EVENT_KEY_UP && ((event.key.mod & SDL_KMOD_LGUI) != 0)) {
      bool key_handled = true;
      switch (key) {
#if HAVE_IMGUI
      case KEY_GRAVE:
        m_Connector.Action()->TogglePopup();
        break;
#endif
      case KEY_F:
        m_Connector.Action()->ToggleFullscreen();
        break;
      case KEY_N:
        CSDLAction::SetUpscaleFilter(GamescopeUpscaleFilter::PIXEL);
        break;
      case KEY_B:
        CSDLAction::SetUpscaleFilter(GamescopeUpscaleFilter::LINEAR);
        break;
      case KEY_K:
        CSDLAction::ToggleDownscaleFilter();
        break;
      case KEY_U:
        CSDLAction::ToggleUpscaleFilterFSR();
        break;
      case KEY_Y:
        CSDLAction::ToggleUpscaleFilterNIS();
        break;
      case KEY_I:
        CSDLAction::IncUpscaleFilterSharpness();
        break;
      case KEY_O:
        CSDLAction::DecUpscaleFilterSharpness();
        break;
#if HAVE_SCREENSHOT
      case KEY_S:
        gamescope::CScreenshotManager::Get().TakeScreenshot(true);
        break;
#endif
      case KEY_G:
        m_Connector.Action()->ToggleKeyboardGrab();
        break;
      default:
        key_handled = false;
      }
      if (key_handled) {
        handled = true;
        break;
      }
    }

    // On Wayland, clients handle key repetition
    if (event.key.repeat) {
      handled = true;
      break;
    }

    wlserver_lock();
    wlserver_key(key, event.type == SDL_EVENT_KEY_DOWN, event.key.timestamp);
    wlserver_unlock();
    handled = true;
  } break;

  default:
    break;
  }

  return handled;
}

} // namespace gamescope
