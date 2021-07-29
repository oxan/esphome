#pragma once

#include "esphome/core/component.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/light/light_output.h"
#include "esphome/components/light/light_output_util.h"

namespace esphome {
namespace monochromatic {

class MonochromaticLightOutput : public light::LightOutput {
 public:
  void set_output(output::FloatOutput *output) { output_ = output; }
  light::LightTraits get_traits() override {
    auto traits = light::LightTraits();
    traits.set_supported_color_modes({light::ColorMode::BRIGHTNESS});
    return traits;
  }
  void write_state(light::LightState *state) override {
    float bright = light::LightOutputUtil::as_brightness(state);
    this->output_->set_level(bright);
  }

 protected:
  output::FloatOutput *output_;
};

}  // namespace monochromatic
}  // namespace esphome
