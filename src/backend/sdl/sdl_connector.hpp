#pragma once

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_render.h>

#include "backend.h"
#include "backend/sdl/sdl_action.hpp"

namespace gamescope {

class CSDLBackend;
class CSDLConnector final
    : public CBaseBackendConnector
    , public INestedHints {
  CSDLBackend* m_pBackend = nullptr;
  CSDLAction* m_pAction = nullptr;
  SDL_Window* m_pWindow = nullptr;
  SDL_Window* m_pPopup = nullptr;
  VkSurfaceKHR m_pVkSurface = VK_NULL_HANDLE;
  SDL_Renderer* m_pUiRenderer = nullptr;
  // BackendConnectorHDRInfo m_HDRInfo{};

public:
  CSDLConnector(CSDLBackend* pBackend);
  ~CSDLConnector() override;

  auto Init() -> bool;

  /////////////////////
  // IBackendConnector
  /////////////////////

  [[nodiscard]] auto GetCurrentOrientation() const -> GamescopePanelOrientation override;
  // [[nodiscard]] auto GetHDRInfo() const -> BackendConnectorHDRInfo const& override;
  [[nodiscard]] auto GetMake() const -> char const* override;
  [[nodiscard]] auto GetModel() const -> char const* override;
  // [[nodiscard]] auto GetModes() const -> std::span<BackendMode const> override;
  [[nodiscard]] auto GetName() const -> char const* override;
  // [[nodiscard]] auto GetRawEDID() const -> std::span<uint8_t const> override;
  [[nodiscard]] auto GetScreenType() const -> GamescopeScreenType override;
  [[nodiscard]] auto GetValidDynamicRefreshRates() const -> std::span<uint32_t const> override;
#if 0
  [[nodiscard]] auto SupportsHDR() const -> bool override;
  [[nodiscard]] auto SupportsVRR() const -> bool override;

  [[nodiscard]] auto IsHDRActive() const -> bool override;
  [[nodiscard]] auto IsVRRActive() const -> bool override;
#endif
  using dc_t = displaycolorimetry_t;
  void GetNativeColorimetry(bool bHDR10,
                            dc_t* displayColorimetry,
                            EOTF* displayEOTF,
                            dc_t* outputEncodingColorimetry,
                            EOTF* outputEncodingEOTF) const override;

  auto GetNestedHints() -> INestedHints* override { return this; }

  auto Present(FrameInfo_t const* pFrameInfo, bool bAsync) -> int override;

  ///////////////////
  // INestedHints
  ///////////////////

  void SetCursorImage(std::shared_ptr<INestedHints::CursorInfo> info) override;
  void SetIcon(std::shared_ptr<std::vector<uint32_t>> uIconPixels) override;
  void SetRelativeMouseMode(bool bRelative) override;
  void SetSelection(std::shared_ptr<std::string> szContents, GamescopeSelection eSelection) override;
  void SetTitle(std::shared_ptr<std::string> szTitle) override;
  void SetVisible(bool bVisible) override;

  //--

  [[nodiscard]] auto GetSDLWindow() const -> SDL_Window* { return m_pWindow; }
  [[nodiscard]] auto GetPopupWindow() const -> SDL_Window* { return m_pPopup; }
  [[nodiscard]] auto GetVulkanSurface() const -> VkSurfaceKHR { return m_pVkSurface; }
  [[nodiscard]] auto GetUiRenderer() const -> SDL_Renderer* { return m_pUiRenderer; }
  [[nodiscard]] auto Action() const -> CSDLAction* { return m_pAction; }

  [[nodiscard]] auto GetTitle() const -> std::shared_ptr<std::string>;

  ////////////////////////
  // Custom UI
  ///////////////////////

  // Initialization
  void UiInit();

  void UiStartFrame();
  void UiLayout();
  void UiRender();
  void UiPresent();

  void UiShow(bool bShow);
  auto IsUiHidden() -> bool;

  void UiShutdown();

  /** Print available SDLRenderer drivers */
  void ListRendererDrivers();

};
} // namespace gamescope
