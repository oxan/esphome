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
};

}  // namespace light
}  // namespace esphome
