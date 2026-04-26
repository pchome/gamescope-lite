#include <SDL3/SDL_events.h>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_vulkan.h>

#include "sdl_backend.hpp"
#include "sdl_thread.hpp"
#include "sdl_gui.hpp"

#include "GamescopeVersion.h"

#include "log.hpp"
#include "main.hpp"
#include "rendervulkan.hpp"
#include "steamcompmgr.hpp"
#include "wlserver.hpp"

/*static*/ int g_nOldNestedRefresh = 0;
/*static*/ bool g_bWindowFocused = true;

/*static*/ int g_nOutputWidthPts = 0;
/*static*/ int g_nOutputHeightPts = 0;

namespace gamescope {

static LogScope sdl_log("backend");

void CSDLBackend::SDLThreadFunc() {
  pthread_setname_np(pthread_self(), "gamescope-sdl");

  m_uUserEventIdBase = SDL_RegisterEvents(GAMESCOPE_SDL_EVENT_COUNT);

  SDL_SetHint(SDL_HINT_APP_NAME, gamescopeName);
  SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, "1");
  SDL_SetAppMetadata(gamescopeName, k_szGamescopeVersion, nullptr);

  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
    m_eSDLInit = SDLInitState::SDLInit_Failure;
    m_eSDLInit.notify_all();
    return;
  }

  sdl_log.infof("Using SDL backend");

  if (!SDL_Vulkan_LoadLibrary(nullptr)) {
    fprintf(stderr, "SDL_Vulkan_LoadLibrary failed: %s\n", SDL_GetError());
    m_eSDLInit = SDLInitState::SDLInit_Failure;
    m_eSDLInit.notify_all();
    return;
  }

  unsigned int uExtCount = 0;
  m_ppEnabledExtensionNames = SDL_Vulkan_GetInstanceExtensions(&uExtCount);
  m_enabledExtensionCount = uExtCount;

  if (!m_Connector.Init()) {
    m_eSDLInit = SDLInitState::SDLInit_Failure;
    m_eSDLInit.notify_all();
    return;
  }

  if (!vulkan_init(vulkan_get_instance(), m_Connector.GetVulkanSurface())) {
    m_eSDLInit = SDLInitState::SDLInit_Failure;
    m_eSDLInit.notify_all();
    return;
  }

  if (!wlsession_init()) {
    fprintf(stderr, "Failed to initialize Wayland session\n");
    m_eSDLInit = SDLInitState::SDLInit_Failure;
    m_eSDLInit.notify_all();
    return;
  }

  // Update g_nOutputWidthPts.
  {
    int width, height;
    SDL_GetWindowSize(m_Connector.GetSDLWindow(), &width, &height);
    g_nOutputWidthPts = width;
    g_nOutputHeightPts = height;

    SDL_GetWindowSizeInPixels(m_Connector.GetSDLWindow(), &width, &height);
    g_nOutputWidth = width;
    g_nOutputHeight = height;

    sdl_log.infof("Using %dx%d window", width, height);
  }

  if (g_bForceRelativeMouse) {
    SDL_SetWindowRelativeMouseMode(m_Connector.GetSDLWindow(), true);
    m_bApplicationGrabbed = true;
  }

  SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");

  g_nOldNestedRefresh = g_nNestedRefresh;

  m_eSDLInit = SDLInitState::SDLInit_Success;
  m_eSDLInit.notify_all();

  static uint32_t fake_timestamp = 0;

  wlserver.bWaylandServerRunning.wait(false);

  SDL_Event event;
  while (SDL_WaitEvent(&event)) {
    fake_timestamp++;

    if (!m_Connector.IsUiHidden() && (event.key.mod & SDL_KMOD_LGUI) == 0) {
      if (HandleUiEvent(event)) {
        // Grab all events while popup visible
        // TODO: grab only input events (io.WantCaptureMouse/io.WantCaptureKeyboard)
        continue;
      }
    }

    if (HandleInputEvent(event, fake_timestamp)) {
      continue;
    }

    if (event.type >= SDL_EVENT_WINDOW_FIRST && event.type <= SDL_EVENT_WINDOW_LAST) {
      if (HandleWindowEvent(event)) {
        continue;
      }
    }

    if (event.type == GetUserEventIndex(GAMESCOPE_SDL_EVENT_REQ_EXIT)) {
      // Quit
      return;
    }

    if (HandleUserEvent(event)) {
      continue;
    }
  }
}

} // namespace gamescope
