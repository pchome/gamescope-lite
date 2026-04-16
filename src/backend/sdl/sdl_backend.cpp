// For the nested case, reads input from the SDL window and send to wayland

// #include <X11/Xlib.h>
#include <thread>
// #include <mutex>
#include <string>
#include <string_view>
#include <format>
#include <print>
// #include <optional>
#include <utility>

#include <csignal>
#include <linux/input-event-codes.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <SDL2/SDL_clipboard.h>
#include <SDL2/SDL_events.h>

#include "sdl_backend.hpp"
#include "sdl_connector.hpp"

#include "gamescope_shared.h"
#include "main.hpp"
#include "rendervulkan.hpp"
#include "steamcompmgr.hpp"
#include "wlserver.hpp"

// #include "Utils/Defer.h"
#include "refresh_rate.h"

#include "sdlscancodetable.hpp"

static int g_nOldNestedRefresh = 0;
static bool g_bWindowFocused = true;

static int g_nOutputWidthPts = 0;
static int g_nOutputHeightPts = 0;

extern bool g_bForceHDR10OutputDebug;
extern bool steamMode;
extern bool g_bFirstFrame;
extern int g_nPreferredOutputWidth;
extern int g_nPreferredOutputHeight;

namespace gamescope {

static LogScope sdl_log("backend");

////////////////
// CSDLBackend
////////////////

CSDLBackend::CSDLBackend()
: m_Connector{this}
, m_SDLThread{[this]() -> void { this->SDLThreadFunc(); }} {}

auto CSDLBackend::Init() -> bool {
  m_eSDLInit.wait(SDLInitState::SDLInit_Waiting);
  return m_eSDLInit == SDLInitState::SDLInit_Success;
}

auto CSDLBackend::PostInit() -> bool { return true; }

auto CSDLBackend::GetInstanceExtensions() const -> std::span<const char *const> {
  return std::span<const char *const>{m_pszInstanceExtensions.begin(), m_pszInstanceExtensions.end()};
}

auto CSDLBackend::GetDeviceExtensions(VkPhysicalDevice  /*pVkPhysicalDevice*/) const -> std::span<const char *const> {
  return std::span<const char *const>{};
}

auto CSDLBackend::GetPresentLayout() const -> VkImageLayout { return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; }

void CSDLBackend::GetPreferredOutputFormat(uint32_t *pPrimaryPlaneFormat, uint32_t *pOverlayPlaneFormat) const {
  *pPrimaryPlaneFormat = VulkanFormatToDRM(VK_FORMAT_A2B10G10R10_UNORM_PACK32);
  *pOverlayPlaneFormat = VulkanFormatToDRM(VK_FORMAT_B8G8R8A8_UNORM);
}

auto CSDLBackend::ValidPhysicalDevice(VkPhysicalDevice  /*pVkPhysicalDevice*/) const -> bool { return true; }

void CSDLBackend::DirtyState(bool bForce, bool bForceModeset) {}
auto CSDLBackend::PollState() -> bool { return false; }

auto CSDLBackend::CreateBackendBlob(const std::type_info & /*type*/, std::span<const uint8_t> data) -> std::shared_ptr<BackendBlob> {
  return std::make_shared<BackendBlob>(data);
}

auto CSDLBackend::ImportDmabufToBackend(wlr_dmabuf_attributes * /*pDmaBuf*/) -> OwningRc<IBackendFb> { return new CBaseBackendFb(); }

auto CSDLBackend::UsesModifiers() const -> bool { return false; }
auto CSDLBackend::GetSupportedModifiers(uint32_t  /*uDrmFormat*/) const -> std::span<const uint64_t> {
  return std::span<const uint64_t>{};
}

auto CSDLBackend::GetCurrentConnector() -> IBackendConnector * { return &m_Connector; }
auto CSDLBackend::GetConnector(GamescopeScreenType eScreenType) -> IBackendConnector * {
  if (eScreenType == GAMESCOPE_SCREEN_TYPE_INTERNAL) {
    return &m_Connector;
  }
  return nullptr;
}

/** We use the nested hints cursor stuff.
  * Not our own plane. */
auto CSDLBackend::SupportsPlaneHardwareCursor() const -> bool { return false; }
auto CSDLBackend::SupportsTearing() const -> bool { return false; }
auto CSDLBackend::UsesVulkanSwapchain() const -> bool { return true; }
auto CSDLBackend::IsSessionBased() const -> bool { return false; }
/** We use a Vulkan swapchain, so yes. */
auto CSDLBackend::SupportsExplicitSync() const -> bool { return true; }
auto CSDLBackend::IsPaused() const -> bool { return false; }
auto CSDLBackend::IsVisible() const -> bool { return true; }
auto CSDLBackend::CursorSurfaceSize(glm::uvec2 uvecSize) const -> glm::uvec2 { return uvecSize; }

///////////////////
// INestedHints
///////////////////

void CSDLBackend::SetCursorImage(std::shared_ptr<INestedHints::CursorInfo> info) {
  m_pApplicationCursor = std::move(info);
  PushUserEvent(GAMESCOPE_SDL_EVENT_CURSOR);
}

void CSDLBackend::SetRelativeMouseMode(bool bRelative) {
  // Do not spam SDL events
  if (bRelative == m_bApplicationGrabbed) {
    return;
  }
  m_bApplicationGrabbed = bRelative;
  PushUserEvent(GAMESCOPE_SDL_EVENT_GRAB);
}

void CSDLBackend::SetVisible(bool bVisible) {
  // Do not spam SDL events
  if (m_bApplicationVisible == bVisible) {
    return;
  }
  m_bApplicationVisible = bVisible;
  PushUserEvent(GAMESCOPE_SDL_EVENT_VISIBLE);
}

void CSDLBackend::SetTitle(std::shared_ptr<std::string> szTitle) {
  m_pApplicationTitle = std::move(szTitle);
  PushUserEvent(GAMESCOPE_SDL_EVENT_TITLE);
}

void CSDLBackend::SetIcon(std::shared_ptr<std::vector<uint32_t>> uIconPixels) {
  m_pApplicationIcon = std::move(uIconPixels);
  PushUserEvent(GAMESCOPE_SDL_EVENT_ICON);
}

void CSDLBackend::SetSelection(const std::shared_ptr<std::string> &szContents, GamescopeSelection eSelection) {
  switch (eSelection) {
    case GAMESCOPE_SELECTION_CLIPBOARD:
      SDL_SetClipboardText(szContents->c_str());
      break;
    case GAMESCOPE_SELECTION_PRIMARY:
      SDL_SetPrimarySelectionText(szContents->c_str());
      break;
    case GAMESCOPE_SELECTION_COUNT:
    default:
      break;
  }
}

void CSDLBackend::OnBackendBlobDestroyed(BackendBlob *pBlob) { /* Do nothing. */ }


CSDLBackend::~CSDLBackend() {
  PushUserEvent(GAMESCOPE_SDL_EVENT_REQ_EXIT);
  if (m_SDLThread.joinable()) {
    m_SDLThread.join();
  }
}

void CSDLBackend::SDLThreadFunc() {
  pthread_setname_np(pthread_self(), "gamescope-sdl");

  m_uUserEventIdBase = SDL_RegisterEvents(GAMESCOPE_SDL_EVENT_COUNT);

  SDL_SetHint(SDL_HINT_APP_NAME, "Gamescope");
  SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, "1");

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
    m_eSDLInit = SDLInitState::SDLInit_Failure;
    m_eSDLInit.notify_all();
    return;
  }

  sdl_log.infof("Using SDL backend");

  if (SDL_Vulkan_LoadLibrary(nullptr) != 0) {
    fprintf(stderr, "SDL_Vulkan_LoadLibrary failed: %s\n", SDL_GetError());
    m_eSDLInit = SDLInitState::SDLInit_Failure;
    m_eSDLInit.notify_all();
    return;
  }

  unsigned int uExtCount = 0;
  SDL_Vulkan_GetInstanceExtensions(nullptr, &uExtCount, nullptr);
  m_pszInstanceExtensions.resize(uExtCount);
  SDL_Vulkan_GetInstanceExtensions(nullptr, &uExtCount, m_pszInstanceExtensions.data());

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

#if SDL_VERSION_ATLEAST(2, 26, 0)
    SDL_GetWindowSizeInPixels(m_Connector.GetSDLWindow(), &width, &height);
#endif
    g_nOutputWidth = width;
    g_nOutputHeight = height;

    sdl_log.infof("Using %dx%d window", width, height);
  }

  if (g_bForceRelativeMouse) {
    SDL_SetRelativeMouseMode(SDL_TRUE);
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

    switch (event.type) {
    case SDL_CLIPBOARDUPDATE: {
      char *pClipBoard = SDL_GetClipboardText();
      char *pPrimarySelection = SDL_GetPrimarySelectionText();

      gamescope_set_selection(pClipBoard, GAMESCOPE_SELECTION_CLIPBOARD);
      gamescope_set_selection(pPrimarySelection, GAMESCOPE_SELECTION_PRIMARY);

      SDL_free(pClipBoard);
      SDL_free(pPrimarySelection);
    } break;

    case SDL_MOUSEMOTION: {
      if (m_bApplicationGrabbed) {
        if (g_bWindowFocused) {
          wlserver_lock();
          wlserver_mousemotion(event.motion.xrel, event.motion.yrel, fake_timestamp);
          wlserver_unlock();
        }
      } else {
        wlserver_lock();
        wlserver_touchmotion(event.motion.x / float(g_nOutputWidthPts), event.motion.y / float(g_nOutputHeightPts), 0,
                             fake_timestamp);
        wlserver_unlock();
      }
    } break;

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP: {
      wlserver_lock();
      wlserver_mousebutton(SDLButtonToLinuxButton(event.button.button), event.button.state == SDL_PRESSED,
                           fake_timestamp);
      wlserver_unlock();
    } break;

    case SDL_MOUSEWHEEL: {
      wlserver_lock();
      wlserver_mousewheel(-event.wheel.x, -event.wheel.y, fake_timestamp);
      wlserver_unlock();
    } break;

    case SDL_FINGERMOTION: {
      wlserver_lock();
      wlserver_touchmotion(event.tfinger.x, event.tfinger.y, event.tfinger.fingerId, fake_timestamp);
      wlserver_unlock();
    } break;

    case SDL_FINGERDOWN: {
      wlserver_lock();
      wlserver_touchdown(event.tfinger.x, event.tfinger.y, event.tfinger.fingerId, fake_timestamp);
      wlserver_unlock();
    } break;

    case SDL_FINGERUP: {
      wlserver_lock();
      wlserver_touchup(event.tfinger.fingerId, fake_timestamp);
      wlserver_unlock();
    } break;

    case SDL_KEYDOWN: {
      // If this keydown event is super + one of the shortcut keys, consume the keydown event, since the corresponding
      // keyup event will be consumed by the next case statement when the user releases the key
      if (event.key.keysym.mod & KMOD_LGUI) {
        uint32_t key = SDLScancodeToLinuxKey(event.key.keysym.scancode);
        const uint32_t shortcutKeys[] = {KEY_F, KEY_N, KEY_B, KEY_K, KEY_U, KEY_Y, KEY_I, KEY_O, KEY_S, KEY_G};
        const bool isShortcutKey =
            std::find(std::begin(shortcutKeys), std::end(shortcutKeys), key) != std::end(shortcutKeys);
        if (isShortcutKey) {
          break;
        }
      }
    }
      [[fallthrough]];
    case SDL_KEYUP: {
      uint32_t key = SDLScancodeToLinuxKey(event.key.keysym.scancode);

      if (event.type == SDL_KEYUP && (event.key.keysym.mod & KMOD_LGUI)) {
        bool handled = true;
        switch (key) {
        case KEY_F:
          g_bFullscreen = !g_bFullscreen;
          SDL_SetWindowFullscreen(m_Connector.GetSDLWindow(), g_bFullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
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
          g_wantedUpscaleFilter = (g_wantedUpscaleFilter == GamescopeUpscaleFilter::FSR)
                                      ? GamescopeUpscaleFilter::LINEAR
                                      : GamescopeUpscaleFilter::FSR;
          break;
        case KEY_Y:
          g_wantedUpscaleFilter = (g_wantedUpscaleFilter == GamescopeUpscaleFilter::NIS)
                                      ? GamescopeUpscaleFilter::LINEAR
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
          SDL_SetWindowKeyboardGrab(m_Connector.GetSDLWindow(), g_bGrabbed ? SDL_TRUE : SDL_FALSE);

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
      if (event.key.repeat)
        break;

      wlserver_lock();
      wlserver_key(key, event.type == SDL_KEYDOWN, fake_timestamp);
      wlserver_unlock();
    } break;

    case SDL_WINDOWEVENT: {
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
        SDL_DisplayMode mode = {SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, 0};

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
    } break;

    default: {
      if (event.type == GetUserEventIndex(GAMESCOPE_SDL_EVENT_VISIBLE)) {
        bool bVisible = m_bApplicationVisible;

        // If we are Steam Mode in nested, show the window
        // whenever we have had a first frame to match
        // what we do in embedded with Steam for testing
        // held commits, etc.
        if (steamMode)
          bVisible |= !g_bFirstFrame;

        if (m_bShown != bVisible) {
          m_bShown = bVisible;

          if (m_bShown) {
            SDL_ShowWindow(m_Connector.GetSDLWindow());
          } else {
            SDL_HideWindow(m_Connector.GetSDLWindow());
          }
        }
      } else if (event.type == GetUserEventIndex(GAMESCOPE_SDL_EVENT_TITLE)) {
        std::shared_ptr<std::string> pAppTitle = m_pApplicationTitle;

        std::string szTitle = pAppTitle ? *pAppTitle : "gamescope";
        if (g_bGrabbed)
          szTitle += " (grabbed)";
        SDL_SetWindowTitle(m_Connector.GetSDLWindow(), szTitle.c_str());

        szTitle = "Title: " + szTitle;
        SDL_SetHint(SDL_HINT_SCREENSAVER_INHIBIT_ACTIVITY_NAME, szTitle.c_str());
        SDL_DisableScreenSaver();
      } else if (event.type == GetUserEventIndex(GAMESCOPE_SDL_EVENT_ICON)) {
        std::shared_ptr<std::vector<uint32_t>> pIcon = m_pApplicationIcon;

        if (m_pIconSurface) {
          SDL_FreeSurface(m_pIconSurface);
          m_pIconSurface = nullptr;
        }

        if (pIcon && pIcon->size() >= 3) {
          const uint32_t uWidth = (*pIcon)[0];
          const uint32_t uHeight = (*pIcon)[1];

          m_pIconSurface = SDL_CreateRGBSurfaceFrom(&(*pIcon)[2], uWidth, uHeight, 32, uWidth * sizeof(uint32_t),
                                                    0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
        }

        SDL_SetWindowIcon(m_Connector.GetSDLWindow(), m_pIconSurface);
      } else if (event.type == GetUserEventIndex(GAMESCOPE_SDL_EVENT_GRAB)) {
        SDL_SetRelativeMouseMode(m_bApplicationGrabbed ? SDL_TRUE : SDL_FALSE);
      } else if (event.type == GetUserEventIndex(GAMESCOPE_SDL_EVENT_CURSOR)) {
        std::shared_ptr<INestedHints::CursorInfo> pCursorInfo = m_pApplicationCursor;

        if (m_pCursorSurface) {
          SDL_FreeSurface(m_pCursorSurface);
          m_pCursorSurface = nullptr;
        }

        if (m_pCursor) {
          SDL_FreeCursor(m_pCursor);
          m_pCursor = nullptr;
        }

        if (pCursorInfo) {
          m_pCursorSurface = SDL_CreateRGBSurfaceFrom(pCursorInfo->pPixels.data(), pCursorInfo->uWidth,
                                                      pCursorInfo->uHeight, 32, pCursorInfo->uWidth * sizeof(uint32_t),
                                                      0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

          m_pCursor = SDL_CreateColorCursor(m_pCursorSurface, pCursorInfo->uXHotspot, pCursorInfo->uYHotspot);
        }

        SDL_SetCursor(m_pCursor);
      } else if (event.type == GetUserEventIndex(GAMESCOPE_SDL_EVENT_REQ_EXIT)) {
        return;
      }
    } break;
    }
  }
}

auto CSDLBackend::GetUserEventIndex(SDLCustomEvents eEvent) const -> uint32_t { return m_uUserEventIdBase + uint32_t(eEvent); }

void CSDLBackend::PushUserEvent(SDLCustomEvents eEvent) {
  SDL_Event event = {
      .user =
          {
              .type = GetUserEventIndex(eEvent),
          },
  };
  SDL_PushEvent(&event);
}

/////////////////////////
// Backend Instantiator
/////////////////////////

template <> auto IBackend::Set<CSDLBackend>() -> bool { return Set(new CSDLBackend{}); }
} // namespace gamescope
