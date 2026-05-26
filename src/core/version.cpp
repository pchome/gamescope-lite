#include <print>

#include "version.hpp"

namespace gamescope {
void PrintVersion() {
  std::println("[{}][version]  {} ({})", build::project_name, gamescopeVersion, build::buildtype);
}
} // namespace gamescope
