#include "esphome/core/color.h"

namespace esphome {

Color Color::from_color_temperature(float color_temperature) {
  // Based on the HA source code
  // https://github.com/home-assistant/core/blob/2222a121f/homeassistant/util/color.py#L491

  float hectokelvin = clamp(10000 / color_temperature, 10, 400);
  uint8_t red = clamp(329.698727446 * powf(hectokelvin - 60, -0.1332047592), 0, 255);
  uint8_t green = clamp(hectokelvin <= 66 ? 99.4708025861 * logf(hectokelvin) - 161.1195681661
                                          : 288.1221695283 * powf(hectokelvin - 60, -0.0755148492), 0, 255);
  uint8_t blue = clamp(138.5177312231 * logf(hectokelvin - 10) - 305.0447927307, 0, 255);
  return Color(red, green, blue);
}

};  // namespace esphome
