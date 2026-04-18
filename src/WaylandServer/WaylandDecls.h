#pragma once

namespace gamescope::WaylandServer
{
    class CWaylandResource;

    template <typename... Types>
	class CWaylandProtocol;

    class CLinuxDrmSyncobjManager;
    class CLinuxDrmSyncobjSurface;
    class CLinuxDrmSyncobjTimeline;
    using CLinuxDrmSyncobj = CWaylandProtocol<CLinuxDrmSyncobjManager>;

#if HAVE_RESHADE
    class CReshadeManager;
    using CReshade = CWaylandProtocol<CReshadeManager>;
#endif

    class CGamescopeActionBindingManager;
    using CGamescopeActionBindingProtocol = CWaylandProtocol<CGamescopeActionBindingManager>;

}
