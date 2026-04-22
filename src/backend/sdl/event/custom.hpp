#pragma once

#include <cstdint>

namespace sdl::surface {

constexpr int32_t  depth = 32;
constexpr uint32_t Rmask = 0x00FF0000;
constexpr uint32_t Gmask = 0x0000FF00;
constexpr uint32_t Bmask = 0x000000FF;
constexpr uint32_t Amask = 0xFF000000;

} // namespace sdl::surface
