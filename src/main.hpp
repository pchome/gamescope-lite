#pragma once

#include <getopt.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <vector>

extern const char *gamescope_optstring;
extern std::vector<option> const gamescope_options;

extern std::atomic< bool > g_bRun;

extern int g_nNestedWidth;
extern int g_nNestedHeight;
extern int g_nNestedRefresh; // mHz
extern int g_nNestedUnfocusedRefresh; // mHz
extern int g_nNestedDisplayIndex;

extern uint32_t g_nOutputWidth;
extern uint32_t g_nOutputHeight;

extern int g_nPreferredOutputWidth;
extern int g_nPreferredOutputHeight;

extern bool g_bForceRelativeMouse;
extern int g_nOutputRefresh; // mHz
extern bool g_bOutputHDREnabled;
extern bool g_bForceInternal;

extern bool g_bFullscreen;

extern bool g_bGrabbed;

extern float g_aspectRatio;

extern float g_mouseSensitivity;
// extern const char *g_sOutputName;

enum class GamescopeUpscaleFilter : uint32_t
{
    LINEAR = 0,
    NEAREST,
    FSR,
    NIS,
    PIXEL,

    FROM_VIEW = 0xF, // internal
};

enum class GamescopeDownscaleFilter : uint32_t
{
    LINEAR = 0,
    BICUBIC,
};

struct GamescopeBicubicParams
{
	float b = 1.0f / 3.0f;
	float c = 1.0f / 3.0f;
};

static constexpr bool DoesHardwareSupportUpscaleFilter( GamescopeUpscaleFilter eFilter )
{
    // Could do nearest someday... AMDGPU DC supports custom tap placement to an extent.

    return eFilter == GamescopeUpscaleFilter::LINEAR;
}

enum class GamescopeUpscaleScaler : uint32_t
{
    AUTO,
    INTEGER,
    FIT,
    FILL,
    STRETCH,
    NATIVE,
};

extern GamescopeUpscaleFilter g_upscaleFilter;
extern GamescopeDownscaleFilter g_downscaleFilter;
extern GamescopeUpscaleScaler g_upscaleScaler;
extern GamescopeUpscaleFilter g_wantedUpscaleFilter;
extern GamescopeDownscaleFilter g_wantedDownscaleFilter;
extern GamescopeUpscaleScaler g_wantedUpscaleScaler;
extern int g_upscaleFilterSharpness;
extern bool g_bForcePreemptiveUpscaling;
extern GamescopeBicubicParams g_bicubicParams;

/**
 * GamescopeUpscaleFilter::FSR
 * - Range: (>2.0f -> 0.0f) higher value mean lower sharpness
 * - Step: changes more noticeable between lower values
 * - Prev algorithm: `sharpness / 10.0f` -> [0.0..2.0] step: 0.1
 * - Prev default: `0.2`
 * - Recommended (aka marketing bullshit) default: `0.2`
 * @todo find the real function
 */
constexpr std::array<float, 21> const g_fsrSharpnessMap = {
    { 4.00f, 2.00f, 1.00f, 0.90f, 0.80f, 0.70f, 0.60f, 0.50f, 0.40f, 0.30f, 0.20f,
      0.10f, 0.08f, 0.07f, 0.06f, 0.05f, 0.04f, 0.03f, 0.02f, 0.01f, 0.0f },
};
/**
 * GamescopeUpscaleFilter::NIS
 * - Range: (0.0f -> 1.0f) higher value mean higher sharpness
 * - Step: changes are linear
 * - Prev algorithm: `(20 - sharpness) / 20.0f` -> [1.0..0.0] step: 0.05
 * - Prev default: `0.9`
 * - Recommended (__GL_SHARPEN_VALUE) default: `0.5`
 * @todo confirm it's real function is linear
 */
constexpr std::array<float, 21> const g_nisSharpnessMap = {
    { 0.00f, 0.05f, 0.10f, 0.15f, 0.20f, 0.25f, 0.30f, 0.35f, 0.40f, 0.45f, 0.50f,
      0.55f, 0.60f, 0.65f, 0.70f, 0.75f, 0.80f, 0.85f, 0.90f, 0.95f, 1.0f },
};
constexpr auto const getFsrMappedSharpness = []( int v ) { return g_fsrSharpnessMap.at( std::clamp( v, 0, 20 ) ); };
constexpr auto const getNisMappedSharpness = []( int v ) { return g_nisSharpnessMap.at( std::clamp( v, 0, 20 ) ); };

extern bool g_bBorderlessOutputWindow;

// extern bool g_bExposeWayland;

extern bool g_bRt;

extern int g_nXWaylandCount;

extern uint32_t g_preferVendorID;
extern uint32_t g_preferDeviceID;

extern bool g_bFp16Enabled;
