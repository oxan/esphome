#pragma once

#include "esphome/core/component.h"
#include "light_state.h"
#include "light_traits.h"

namespace esphome {
namespace light {

class LightOutputUtil {
 public:
  static float as_brightness(const LightState &state);
  static void as_rgb(const LightState &state, float *red, float *green, float *blue);
  static void as_rgbw(const LightState &state, float *red, float *green, float *blue, float *white);
  static void as_rgbww(const LightState &state, float *red, float *green, float *blue, float *cold_white,
                       float *warm_white, bool constant_brightness = false);
  static void as_cwww(const LightState &state, float *cold_white, float *warm_white, bool constant_brightness = false);

  /// Get the color temperature as a value in the range 0-1, where 0 is the lowest color temperature supported by the
  /// output, and 1 the highest.
  static float as_relative_color_temp(const LightState &state);
};

}  // namespace light
}  // namespace esphome
