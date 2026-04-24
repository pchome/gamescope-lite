#pragma once

#include <thread>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_surface.h>

#include "backend.h"
#include "sdl_connector.hpp"

namespace gamescope {

enum class SDLInitState : std::uint8_t {
  SDLInit_Waiting,
  SDLInit_Success,
  SDLInit_Failure,
};

enum SDLCustomEvents : std::uint8_t {
  GAMESCOPE_SDL_EVENT_TITLE,
  GAMESCOPE_SDL_EVENT_ICON,
  GAMESCOPE_SDL_EVENT_VISIBLE,
  GAMESCOPE_SDL_EVENT_GRAB,
  GAMESCOPE_SDL_EVENT_CURSOR,

  GAMESCOPE_SDL_EVENT_REQ_EXIT,

  GAMESCOPE_SDL_EVENT_COUNT,
};

class CSDLBackend final : public CBaseBackend {
  bool m_bShown = false;
  CSDLConnector m_Connector; // Window.
  uint32_t m_uUserEventIdBase = 0u;
  std::vector<char const*> m_pszInstanceExtensions;
  char const* const* m_ppEnabledExtensionNames;
  uint32_t m_enabledExtensionCount = 0u;

  std::thread m_SDLThread;
  std::atomic<SDLInitState> m_eSDLInit = {SDLInitState::SDLInit_Waiting};

  std::atomic<bool> m_bApplicationGrabbed = {false};
  std::atomic<bool> m_bApplicationVisible = {false};
  std::atomic<std::shared_ptr<INestedHints::CursorInfo>> m_pApplicationCursor;
  std::atomic<std::shared_ptr<std::string>> m_pApplicationTitle;
  std::atomic<std::shared_ptr<std::vector<uint32_t>>> m_pApplicationIcon;
  SDL_Surface* m_pIconSurface = nullptr;
  SDL_Surface* m_pCursorSurface = nullptr;
  SDL_Cursor* m_pCursor = nullptr;

  void SDLThreadFunc();

  auto GetUserEventIndex(SDLCustomEvents eEvent) const -> uint32_t;
  void PushUserEvent(SDLCustomEvents eEvent);

  /** SDL event handlers */

  void HandleEvent(SDL_Event eEvent);

  /** Input event handlers */

  /** Return `true` if handled, otherwise `false` */
  auto HandleInputEvent(SDL_Event eEvent, uint32_t fake_timestamp) -> bool;

  /** Window event handlers */

  /** Return `true` if handled, otherwise `false` */
  auto HandleWindowEvent(SDL_Event eEvent) -> bool;

  /** Custom event handlers */

  void SwitchKeyboardGrabIndicator();
  void SwitchMainWindowVisibility();
  void UseApplicationIcon();
  void UseApplicationCursor();
  /** Return `true` if handled, otherwise `false` */
  auto HandleUserEvent(SDL_Event eEvent) -> bool;

protected:
  void OnBackendBlobDestroyed(BackendBlob* pBlob) override;

public:
  CSDLBackend();
  ~CSDLBackend() override;

  /////////////
  // IBackend
  /////////////
  auto Init() -> bool override;
  auto PostInit() -> bool override;

  void DirtyState(bool bForce = false, bool bForceModeset = false) override;
  auto PollState() -> bool override;

  auto CreateBackendBlob(std::type_info const& type, std::span<uint8_t const> data)
      -> std::shared_ptr<BackendBlob> override;
  auto CursorSurfaceSize(glm::uvec2 uvecSize) const -> glm::uvec2 override;

  auto GetConnector(GamescopeScreenType eScreenType) -> IBackendConnector* override;
  auto GetCurrentConnector() -> IBackendConnector* override;
  auto GetDeviceExtensions(VkPhysicalDevice pVkPhysicalDevice) const -> std::span<char const* const> override;
  auto GetInstanceExtensions() const -> std::span<char const* const> override;
  auto GetInstanceExtensionsNames() const -> char const* const* override;
  auto GetInstanceExtensionsCount() const -> uint32_t const override;
  void GetPreferredOutputFormat(uint32_t* pPrimaryPlaneFormat, uint32_t* pOverlayPlaneFormat) const override;
  auto GetPresentLayout() const -> VkImageLayout override;
  auto GetSupportedModifiers(uint32_t uDrmFormat) const -> std::span<uint64_t const> override;

  auto ImportDmabufToBackend(wlr_dmabuf_attributes* pDmaBuf) -> OwningRc<IBackendFb> override;

  auto IsPaused() const -> bool override;
  auto IsSessionBased() const -> bool override;
  auto IsVisible() const -> bool override;

  auto SupportsExplicitSync() const -> bool override;
  auto SupportsPlaneHardwareCursor() const -> bool override;
  auto SupportsTearing() const -> bool override;

  auto UsesModifiers() const -> bool override;
  auto UsesVulkanSwapchain() const -> bool override;

  auto ValidPhysicalDevice(VkPhysicalDevice pVkPhysicalDevice) const -> bool override;

  ////////////////////////
  // INestedHints Compat
  ///////////////////////

  void SetCursorImage(std::shared_ptr<INestedHints::CursorInfo> info);
  void SetIcon(std::shared_ptr<std::vector<uint32_t>> uIconPixels);
  void SetRelativeMouseMode(bool bRelative);
  static void SetSelection(std::shared_ptr<std::string> const& szContents, GamescopeSelection eSelection);
  void SetTitle(std::shared_ptr<std::string> szTitle);
  void SetVisible(bool bVisible);
  void ShowPopup(bool bShow);
};

} // namespace gamescope
