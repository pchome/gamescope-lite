#pragma once
#include <cstdint>
#include <atomic>
#include <memory>
#include <vector>

#include "wlr_begin.hpp"
#include <wlr/types/wlr_compositor.h>
#include "wlr_end.hpp"

#include "gamescope_shared.h"
#include "vblankmanager.hpp"

extern uint32_t currentOutputWidth;
extern uint32_t currentOutputHeight;
#if 0
extern bool g_bHDRItmEnable;
extern bool g_bForceHDRSupportDebug;
extern bool g_bForceHDR10OutputDebug;
#endif
extern std::vector< wlr_surface * > wayland_surfaces_deleted;

extern bool hasFocusWindow;

// These are used for touch scaling, so it's really the window that's focused for touch
extern float focusedWindowScaleX;
extern float focusedWindowScaleY;

extern float focusedWindowOffsetX;
extern float focusedWindowOffsetY;

extern bool g_bFirstFrame;
extern bool g_bFSRActive;
extern bool g_bBicubicActive;

extern uint32_t inputCounter;
extern uint64_t g_lastWinSeq;

extern gamescope::VBlankTime g_SteamCompMgrVBlankTime;
// extern gamescope::VBlankTime g_SteamCompMgrVBlankTime;
extern pid_t focusWindow_pid;
extern std::atomic<std::shared_ptr<std::string>> focusWindow_engine;

extern void steamcompmgr_set_app_refresh_cycle_override( gamescope::GamescopeScreenType type, int override_fps, bool change_refresh, bool change_fps_cap );
