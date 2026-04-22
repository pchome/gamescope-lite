#pragma once

#include <SDL3/SDL_video.h>

#include "backend.h"

namespace gamescope
{
  class CSDLBackend;
  class CSDLConnector final : public CBaseBackendConnector, public INestedHints
  {
    CSDLBackend *m_pBackend = nullptr;
    SDL_Window *m_pWindow = nullptr;
    VkSurfaceKHR m_pVkSurface = VK_NULL_HANDLE;
    BackendConnectorHDRInfo m_HDRInfo{};

  public:
    CSDLConnector( CSDLBackend *pBackend );
    ~CSDLConnector() override;

    auto Init() -> bool;

    /////////////////////
    // IBackendConnector
    /////////////////////

    [[nodiscard]] auto GetCurrentOrientation() const -> GamescopePanelOrientation override;
    [[nodiscard]] auto GetHDRInfo() const -> const BackendConnectorHDRInfo & override;
    [[nodiscard]] auto GetMake() const -> const char * override;
    [[nodiscard]] auto GetModel() const -> const char * override;
    [[nodiscard]] auto GetModes() const -> std::span<const BackendMode> override;
    [[nodiscard]] auto GetName() const -> const char * override;
    [[nodiscard]] auto GetRawEDID() const -> std::span<const uint8_t> override;
    [[nodiscard]] auto GetScreenType() const -> GamescopeScreenType override;
    [[nodiscard]] auto GetValidDynamicRefreshRates() const -> std::span<const uint32_t> override;

    [[nodiscard]] auto SupportsHDR() const -> bool override;
    [[nodiscard]] auto SupportsVRR() const -> bool override;

    [[nodiscard]] auto IsHDRActive() const -> bool override;
    [[nodiscard]] auto IsVRRActive() const -> bool override;

    using dc_t = displaycolorimetry_t;
    void GetNativeColorimetry(bool bHDR10, dc_t *displayColorimetry, EOTF *displayEOTF,  dc_t *outputEncodingColorimetry, EOTF *outputEncodingEOTF ) const override;

    auto GetNestedHints() -> INestedHints * override { return this; }

    auto Present( const FrameInfo_t *pFrameInfo, bool bAsync ) -> int override;

    ///////////////////
    // INestedHints
    ///////////////////

    void SetCursorImage( std::shared_ptr<INestedHints::CursorInfo> info ) override;
    void SetIcon( std::shared_ptr<std::vector<uint32_t>> uIconPixels ) override;
    void SetRelativeMouseMode( bool bRelative ) override;
    void SetSelection(std::shared_ptr<std::string> szContents, GamescopeSelection eSelection) override;
    void SetTitle( std::shared_ptr<std::string> szTitle ) override;
    void SetVisible( bool bVisible ) override;

    //--

    [[nodiscard]] auto GetSDLWindow() const -> SDL_Window * { return m_pWindow; }
    [[nodiscard]] auto GetVulkanSurface() const -> VkSurfaceKHR { return m_pVkSurface; }
  };
} //namespace
