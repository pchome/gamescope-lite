#include <format>
#include <optional>
#include <print>

#include <SDL3/SDL_clipboard.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>

#include "sdl_connector.hpp"

#include "sdl_backend.hpp"

#include "constants_include.hpp"
#include "gamescope_shared.h"
#include "main.hpp"
#include "refresh_rate.h"
#include "rendervulkan.hpp"
#include "steamcompmgr.hpp"
#include "vblankmanager.hpp"

extern bool g_bForceHDR10OutputDebug;
// extern bool g_bFirstFrame;
extern int g_nPreferredOutputWidth;
extern int g_nPreferredOutputHeight;

namespace gamescope {
//////////////////
// CSDLConnector
//////////////////

CSDLConnector::CSDLConnector(CSDLBackend* pBackend) : m_pBackend{pBackend} {}

CSDLConnector::~CSDLConnector() {
  if (m_pWindow != nullptr) {
    SDL_DestroyWindow(m_pWindow);
  }
}

auto CSDLConnector::GetName() const -> char const* { return "SDLWindow"; }
auto CSDLConnector::GetMake() const -> char const* { return "Gamescope Lite"; }
auto CSDLConnector::GetModel() const -> char const* { return "Virtual Display"; }

auto CSDLConnector::Init() -> bool {
  g_nOutputWidth = g_nPreferredOutputWidth;
  g_nOutputHeight = g_nPreferredOutputHeight;
  g_nOutputRefresh = g_nNestedRefresh;

  if (g_nOutputHeight == 0) {
    if (g_nOutputWidth != 0) {
      std::println(stderr, "Cannot specify -W without -H");
      return false;
    }
    g_nOutputHeight = defaults::nestedH;
  }

  if (g_nOutputWidth == 0) {
    g_nOutputWidth = g_nOutputHeight * g_aspectRatio;
  }

  if (g_nOutputRefresh == 0) {
    g_nOutputRefresh = gamescope::ConvertHztomHz(defaults::refreshHz);
  }

  uint32_t unusedFlags = 0;
  uint32_t uSDLWindowFlags = SDL_WINDOW_VULKAN;

  uSDLWindowFlags |= SDL_WINDOW_RESIZABLE;
  uSDLWindowFlags |= SDL_WINDOW_HIDDEN;
  uSDLWindowFlags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;

  (g_bBorderlessOutputWindow ? uSDLWindowFlags : unusedFlags) |= SDL_WINDOW_BORDERLESS;
  // (g_bFullscreen ? uSDLWindowFlags : unusedFlags) |= SDL_WINDOW_FULLSCREEN_DESKTOP;
  (g_bGrabbed ? uSDLWindowFlags : unusedFlags) |= SDL_WINDOW_KEYBOARD_GRABBED;

  // auto pos_x = SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_nNestedDisplayIndex);
  // auto pos_y = SDL_WINDOWPOS_UNDEFINED_DISPLAY(g_nNestedDisplayIndex);

  m_pWindow = SDL_CreateWindow(GetName(), g_nOutputWidth, g_nOutputHeight, uSDLWindowFlags);
  // SDL_CreateWindowWithProperties()
  if (m_pWindow == nullptr) {
    return false;
  }

  // fullscreen mode borderless
  SDL_SetWindowFullscreenMode(m_pWindow, nullptr);

  if (!SDL_Vulkan_CreateSurface(m_pWindow, vulkan_get_instance(), nullptr, &m_pVkSurface)) {
    std::println(stderr, "SDL_Vulkan_CreateSurface failed: {0}", SDL_GetError());
    return false;
  }

  {
    constexpr auto offset_x = 10;
    constexpr auto offset_y = 10;
    auto popup_w = static_cast<int>(g_nOutputWidth / 2);
    auto popup_h = static_cast<int>(g_nOutputHeight / 2);
    constexpr auto popup_flags = SDL_WINDOW_POPUP_MENU | SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN;

    m_pPopup = SDL_CreatePopupWindow(m_pWindow, offset_x, offset_y, popup_w, popup_h, popup_flags);
    if (m_pPopup == nullptr) {
      std::println(stderr, "SDL_CreatePopupWindow failed: {0}", SDL_GetError());
    }
  }

  return true;
}

auto CSDLConnector::GetCurrentOrientation() const -> GamescopePanelOrientation { return GAMESCOPE_PANEL_ORIENTATION_0; }
auto CSDLConnector::GetHDRInfo() const -> BackendConnectorHDRInfo const& { return m_HDRInfo; }
auto CSDLConnector::GetModes() const -> std::span<BackendMode const> { return std::span<BackendMode const>{}; }
auto CSDLConnector::GetRawEDID() const -> std::span<uint8_t const> { return std::span<uint8_t const>{}; }
auto CSDLConnector::GetScreenType() const -> GamescopeScreenType { return gamescope::GAMESCOPE_SCREEN_TYPE_INTERNAL; }
auto CSDLConnector::GetValidDynamicRefreshRates() const -> std::span<uint32_t const> {
  return std::span<uint32_t const>{};
}
auto CSDLConnector::IsHDRActive() const -> bool { return false; }
auto CSDLConnector::IsVRRActive() const -> bool { return false; }
auto CSDLConnector::SupportsHDR() const -> bool { return GetHDRInfo().IsHDR10(); }
auto CSDLConnector::SupportsVRR() const -> bool { return false; }

void CSDLConnector::GetNativeColorimetry(bool /*bHDR10*/,
                                         displaycolorimetry_t* displayColorimetry,
                                         EOTF* displayEOTF,
                                         displaycolorimetry_t* outputEncodingColorimetry,
                                         EOTF* outputEncodingEOTF) const {
  if (g_bForceHDR10OutputDebug) {
    *displayColorimetry = displaycolorimetry_2020;
    *displayEOTF = EOTF_PQ;
    *outputEncodingColorimetry = displaycolorimetry_2020;
    *outputEncodingEOTF = EOTF_PQ;
  } else {
    *displayColorimetry = displaycolorimetry_709;
    *displayEOTF = EOTF_Gamma22;
    *outputEncodingColorimetry = displaycolorimetry_709;
    *outputEncodingEOTF = EOTF_Gamma22;
  }
}

auto CSDLConnector::Present(FrameInfo_t const* pFrameInfo, bool /*bAsync*/) -> int {
  // TODO: Resolve const crap
  std::optional oCompositeResult = vulkan_composite((FrameInfo_t*)pFrameInfo, nullptr, false);
  if (!oCompositeResult) {
    return -EINVAL;
  }

  vulkan_present_to_window();

  // TODO: Hook up PresentationFeedback.

  // Wait for the composite result on our side *after* we
  // commit the buffer to the compositor to avoid a bubble.
  vulkan_wait(*oCompositeResult, true);

  GetVBlankTimer().UpdateWasCompositing(true);
  GetVBlankTimer().UpdateLastDrawTime(get_time_in_nanos() - g_SteamCompMgrVBlankTime.ulWakeupTime);

  return 0;
}

void CSDLConnector::SetCursorImage(std::shared_ptr<INestedHints::CursorInfo> info) {
  m_pBackend->SetCursorImage(std::move(info));
}

void CSDLConnector::SetRelativeMouseMode(bool bRelative) { m_pBackend->SetRelativeMouseMode(bRelative); }
void CSDLConnector::SetVisible(bool bVisible) { m_pBackend->SetVisible(bVisible); }
void CSDLConnector::SetTitle(std::shared_ptr<std::string> szTitle) { m_pBackend->SetTitle(std::move(szTitle)); }
void CSDLConnector::SetIcon(std::shared_ptr<std::vector<uint32_t>> uIconPixels) {
  m_pBackend->SetIcon(std::move(uIconPixels));
}

void CSDLConnector::SetSelection(std::shared_ptr<std::string> szContents, GamescopeSelection eSelection) {
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
} // namespace gamescope
