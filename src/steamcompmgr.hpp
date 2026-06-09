#pragma once
#include <cstdint>

#include <string>

#include "wlr_begin.hpp"
// #include <wlr/types/wlr_buffer.h>
// #include <wlr/types/wlr_compositor.h>
// #include <wlr/render/wlr_texture.h>
// #include <wlr/render/dmabuf.h>
#include "wlr_end.hpp"

#include "gamescope_shared.h"
#include "rc.h"

auto get_time_in_milliseconds() -> uint32_t;
auto get_time_in_nanos() -> uint64_t;
void sleep_for_nanos( uint64_t nanos );
void sleep_until_nanos( uint64_t nanos );
auto nanos_to_timespec( uint64_t ulNanos ) -> timespec;

void steamcompmgr_main(int argc, char **argv);

// #include "rendervulkan.hpp"
// #include "wlserver.hpp"
// #include "vblankmanager.hpp"

#include <X11/extensions/Xfixes.h>

struct _XDisplay;
struct CursorBarrierInfo
{
	int x1 = 0;
	int y1 = 0;
	int x2 = 0;
	int y2 = 0;
};
struct CursorBarrier
{
	PointerBarrier obj = None;
	CursorBarrierInfo info = {};
};

struct FrameInfo_t;
class CVulkanTexture;

struct steamcompmgr_win_t;
struct xwayland_ctx_t;
class gamescope_xwayland_server_t;

class MouseCursor
{
public:
	explicit MouseCursor(xwayland_ctx_t *ctx);

	int x() const;
	int y() const;

	void paint(steamcompmgr_win_t *window, steamcompmgr_win_t *fit, FrameInfo_t *frameInfo);
	void setDirty();

	// Will take ownership of data.
	bool setCursorImage(char *data, int w, int h, int hx, int hy);
	bool setCursorImageByName(const char *name);

	void hide();

	void UpdatePosition();

	bool isHidden();
	bool imageEmpty() const { return m_imageEmpty; }

	void undirty() { getTexture(); }

	xwayland_ctx_t *getCtx() const { return m_ctx; }

	bool needs_server_flush() const { return m_needs_server_flush; }
	void inform_flush() { m_needs_server_flush = false; }

	void GetDesiredSize( int& nWidth, int &nHeight );

	void checkSuspension();

	bool IsConstrained() const { return m_bConstrained; }
private:

	bool getTexture();

	void updateCursorFeedback( bool bForce = false );

	int m_x = 0, m_y = 0;
	bool m_bConstrained = false;
	int m_hotspotX = 0, m_hotspotY = 0;

	gamescope::OwningRc<CVulkanTexture> m_texture;
	bool m_dirty;
	uint64_t m_ulLastConnectorId = 0;
	bool m_imageEmpty;

	xwayland_ctx_t *m_ctx;

	bool m_bCursorVisibleFeedback = false;
	bool m_needs_server_flush = false;
};

void nudge_steamcompmgr();
void force_repaint();

struct wlr_surface *steamcompmgr_get_server_input_surface( size_t idx );
struct wlserver_vk_swapchain_feedback* steamcompmgr_get_base_layer_swapchain_feedback();

struct wlserver_x11_surface_info *lookup_x11_surface_info_from_xid( gamescope_xwayland_server_t *xwayland_server, uint32_t xid );

void init_xwayland_ctx(uint32_t serverId, gamescope_xwayland_server_t *xwayland_server);
void gamescope_set_selection(std::string const& contents, GamescopeSelection eSelection);
#if HAVE_RESHADE
void gamescope_set_reshade_effect(std::string effect_path);
void gamescope_clear_reshade_effect();
#endif
MouseCursor *steamcompmgr_get_current_cursor();
MouseCursor *steamcompmgr_get_server_cursor(uint32_t serverId);


