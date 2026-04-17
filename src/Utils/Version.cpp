#include <print>

#include "Process.h"
#include "Version.h"

#include "GamescopeVersion.h"

namespace gamescope {
void PrintVersion() {
    std::println("[{0}] [version]  {1} ({2})",
                 Process::GetProcessName(),
                 gamescope::k_szGamescopeVersion,
                 gamescope::build::buildtype);
}
} // namespace gamescope
