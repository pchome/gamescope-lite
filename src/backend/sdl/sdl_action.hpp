#pragma once

#include "main.hpp"

#include <string>

namespace gamescope {

class CSDLConnector;
class CSDLAction final {
  CSDLConnector* m_pConnector = nullptr;

public:
  CSDLAction(CSDLConnector* pConnector);
  ~CSDLAction() = default;

  static auto Init() -> bool { return true; }

  void TogglePopup();
  void ToggleFullscreen();

  void SetWindowTitle(std::string const& title);

  static void SetUpscaleFilter(GamescopeUpscaleFilter filter);
  static void SetDownscaleFilter(GamescopeDownscaleFilter filter);
  static void SetUpscaleScaler(GamescopeUpscaleScaler scaler);

  static void SetUpscaleFilterSharpness(int sharpness);
  static void IncUpscaleFilterSharpness();
  static void DecUpscaleFilterSharpness();

  void ToggleKeyboardGrab();
  void ToggleMouseGrab();

  static void ToggleUpscaleFilter();
  static void ToggleDownscaleFilter();

  static void ToggleUpscaleFilterFSR();
  static void ToggleUpscaleFilterNIS();
};

} // namespace gamescope
