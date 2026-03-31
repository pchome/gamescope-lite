#pragma once

namespace gamescope
{
    // Backend enum.
    enum GamescopeBackend
    {
        Auto,
        SDL,
        Headless,
    };

    // Backend forward declarations.
    class CSDLBackend;
    class CHeadlessBackend;
}
