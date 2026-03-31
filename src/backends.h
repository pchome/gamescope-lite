#pragma once

namespace gamescope
{
    // Backend enum.
    enum GamescopeBackend
    {
        Auto,
        SDL,
        Headless,
        Wayland,
    };

    // Backend forward declarations.
    class CSDLBackend;
    class CHeadlessBackend;
    class CWaylandBackend;
}
