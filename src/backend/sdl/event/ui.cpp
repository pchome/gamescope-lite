#include "imgui_impl_sdl3.h"

#include "./ui.hpp"
#include "backend/sdl/sdl_backend.hpp"

namespace gamescope {

auto CSDLBackend::HandleUiEvent(SDL_Event event) -> bool { return ImGui_ImplSDL3_ProcessEvent(&event); }

} // namespace gamescope
