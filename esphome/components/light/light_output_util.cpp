#include "light_output_util.h"
#include "color_mode.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace light {

static const char *const TAG = "light";

float LightOutputUtil::as_brightness(const LightState &state) {
  auto values = state.current_values;
  return gamma_correct(values.get_state() * values.get_brightness(), state.get_gamma_correct());
}
void LightOutputUtil::as_rgb(const LightState &state, float *red, float *green, float *blue) {
  auto values = state.current_values;
  if (values.get_color_mode() & ColorCapability::RGB) {
    float brightness = values.get_state() * values.get_brightness() * values.get_color_brightness();
    *red = gamma_correct(brightness * values.get_red(), state.get_gamma_correct());
    *green = gamma_correct(brightness * values.get_green(), state.get_gamma_correct());
    *blue = gamma_correct(brightness * values.get_blue(), state.get_gamma_correct());
  } else {
    *red = 0;
    *green = 0;
    *blue = 0;
  }
}
void LightOutputUtil::as_rgbw(const LightState &state, float *red, float *green, float *blue, float *white) {
  LightOutputUtil::as_rgb(state, red, green, blue);
  auto values = state.current_values;
  if (values.get_color_mode() & ColorCapability::WHITE) {
    float brightness = values.get_state() * values.get_brightness() * values.get_color_brightness();
    *white = gamma_correct(brightness, state.get_gamma_correct());
  } else {
    *white = 0;
  }
}
void LightOutputUtil::as_rgbww(const LightState &state, float *red, float *green, float *blue, float *cold_white,
                               float *warm_white, bool constant_brightness) {
  LightOutputUtil::as_rgb(state, red, green, blue);
  LightOutputUtil::as_cwww(state, cold_white, warm_white, constant_brightness);
}
void LightOutputUtil::as_cwww(const LightState &state, float *cold_white, float *warm_white, bool constant_brightness) {
  auto values = state.current_values;
  if (values.get_color_mode() & ColorMode::COLD_WARM_WHITE) {
    const float cw_level = gamma_correct(values.get_cold_white(), state.get_gamma_correct());
    const float ww_level = gamma_correct(values.get_warm_white(), state.get_gamma_correct());
    const float white_level = gamma_correct(values.get_state() * values.get_brightness(), state.get_gamma_correct());
    *cold_white = white_level * cw_level;
    *warm_white = white_level * ww_level;
    if (constant_brightness && (cw_level > 0 || ww_level > 0)) {
      const float sum = cw_level + ww_level;
      *cold_white /= sum;
      *warm_white /= sum;
    }
  } else {
    *cold_white = 0;
    *warm_white = 0;
  }
}

}  // namespace light
}  // namespace esphome
