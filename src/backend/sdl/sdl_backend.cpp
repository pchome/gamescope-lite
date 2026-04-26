// For the nested case, reads input from the SDL window and send to wayland

#include <SDL3/SDL_clipboard.h>
#include <SDL3/SDL_video.h>
#include <cstdint>

#include "sdl_backend.hpp"
#include "sdl_connector.hpp"

namespace gamescope {

////////////////
// CSDLBackend
////////////////

CSDLBackend::CSDLBackend()
    : m_Connector{this}
    , m_ppEnabledExtensionNames{nullptr}
    , m_SDLThread{[this]() -> void { this->SDLThreadFunc(); }} {}

auto CSDLBackend::Init() -> bool {
  m_eSDLInit.wait(SDLInitState::SDLInit_Waiting);
  return m_eSDLInit == SDLInitState::SDLInit_Success;
}

auto CSDLBackend::PostInit() -> bool { return true; }

auto CSDLBackend::GetInstanceExtensions() const -> std::span<char const* const> {
  return std::span<char const* const>{m_pszInstanceExtensions.begin(), m_pszInstanceExtensions.end()};
}
auto CSDLBackend::GetInstanceExtensionsNames() const -> char const* const* { return m_ppEnabledExtensionNames; };
auto CSDLBackend::GetInstanceExtensionsCount() const -> uint32_t const { return m_enabledExtensionCount; };

auto CSDLBackend::GetDeviceExtensions(VkPhysicalDevice /*pVkPhysicalDevice*/) const -> std::span<char const* const> {
  return std::span<char const* const>{};
}

auto CSDLBackend::GetPresentLayout() const -> VkImageLayout { return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; }

void CSDLBackend::GetPreferredOutputFormat(uint32_t* pPrimaryPlaneFormat, uint32_t* pOverlayPlaneFormat) const {
  *pPrimaryPlaneFormat = DRM_FORMAT_INVALID; // VulkanFormatToDRM(VK_FORMAT_A2B10G10R10_UNORM_PACK32);
  *pOverlayPlaneFormat = DRM_FORMAT_INVALID; // VulkanFormatToDRM(VK_FORMAT_B8G8R8A8_UNORM);
}

auto CSDLBackend::ValidPhysicalDevice(VkPhysicalDevice /*pVkPhysicalDevice*/) const -> bool { return true; }

void CSDLBackend::DirtyState(bool bForce, bool bForceModeset) {}
auto CSDLBackend::PollState() -> bool { return false; }

auto CSDLBackend::CreateBackendBlob(std::type_info const& /*type*/, std::span<uint8_t const> data)
    -> std::shared_ptr<BackendBlob> {
  return std::make_shared<BackendBlob>(data);
}

auto CSDLBackend::ImportDmabufToBackend(wlr_dmabuf_attributes* /*pDmaBuf*/) -> OwningRc<IBackendFb> {
  return new CBaseBackendFb();
}

auto CSDLBackend::UsesModifiers() const -> bool { return false; }
auto CSDLBackend::GetSupportedModifiers(uint32_t /*uDrmFormat*/) const -> std::span<uint64_t const> {
  return std::span<uint64_t const>{};
}

auto CSDLBackend::GetCurrentConnector() -> IBackendConnector* { return &m_Connector; }
auto CSDLBackend::GetConnector(GamescopeScreenType eScreenType) -> IBackendConnector* {
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

void CSDLBackend::SetSelection(std::shared_ptr<std::string> const& szContents, GamescopeSelection eSelection) {
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

void CSDLBackend::OnBackendBlobDestroyed(BackendBlob* pBlob) { /* Do nothing. */ }

CSDLBackend::~CSDLBackend() {
  PushUserEvent(GAMESCOPE_SDL_EVENT_REQ_EXIT);
  if (m_SDLThread.joinable()) {
    m_SDLThread.join();
  }
}

auto CSDLBackend::GetUserEventIndex(SDLCustomEvents eEvent) const -> uint32_t {
  return m_uUserEventIdBase + uint32_t(eEvent);
}

void CSDLBackend::PushUserEvent(SDLCustomEvents eEvent) {
  SDL_Event event = {
      .user =
          {
              .type = GetUserEventIndex(eEvent),
          },
  };
  // SDL3:
  // > You should set the event.common.timestamp field before passing an event to SDL_PushEvent().
  // > If the timestamp is 0 it will be filled in with SDL_GetTicksNS().
  event.common.timestamp = 0;
  SDL_PushEvent(&event);
}

/////////////////////////
// Backend Instantiator
/////////////////////////

template <> auto IBackend::Set<CSDLBackend>() -> bool { return Set(new CSDLBackend{}); }
} // namespace gamescope
