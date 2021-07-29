#pragma once

#include "esphome/core/component.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/light/light_output.h"
#include "esphome/components/light/light_output_util.h"

namespace esphome {
namespace rgb {

class RGBLightOutput : public light::LightOutput {
 public:
  void set_red(output::FloatOutput *red) { red_ = red; }
  void set_green(output::FloatOutput *green) { green_ = green; }
  void set_blue(output::FloatOutput *blue) { blue_ = blue; }
  void set_emulate_color_temperature(bool emulate) { this->emulate_color_temperature_ = emulate; }

  light::LightTraits get_traits() override {
    auto traits = light::LightTraits();
    if (this->emulate_color_temperature_) {
      traits.set_supported_color_modes({light::ColorMode::RGB, light::ColorMode::COLOR_TEMPERATURE});
      traits.set_min_mireds(25);
      traits.set_max_mireds(400);
    } else {
      traits.set_supported_color_modes({light::ColorMode::RGB});
    }
    return traits;
  }
  void write_state(light::LightState *state) override {
    float red, green, blue;
    if (state->current_values.get_color_mode() & light::ColorMode::COLOR_TEMPERATURE) {
      light::LightOutputUtil::color_temp_as_rgb(*state, &red, &green, &blue);
    } else {
      light::LightOutputUtil::as_rgb(*state, &red, &green, &blue);
    }
    this->red_->set_level(red);
    this->green_->set_level(green);
    this->blue_->set_level(blue);
  }

 protected:
  output::FloatOutput *red_;
  output::FloatOutput *green_;
  output::FloatOutput *blue_;
  bool emulate_color_temperature_{false};
};

}  // namespace rgb
}  // namespace esphome
