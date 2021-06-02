#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/core/color.h"
#include "addressable_light_values.h"
#include "esp_color_correction.h"
#include "esp_color_view.h"
#include "esp_range_view.h"
#include "light_output.h"
#include "light_state.h"

namespace esphome {
namespace light {

using ESPColor ESPDEPRECATED("ESPColor is deprecated, use Color instead.") = Color;

class AddressableLight : public LightOutput, public Component {
 public:
  virtual int32_t size() const = 0;
  ESPColorView operator[](int32_t index) {
    return ESPColorView{*this->corrected_values_, interpret_index(index, this->size())};
  }
  ESPColorView get(int32_t index) {
    return ESPColorView{*this->corrected_values_, interpret_index(index, this->size())};
  }
  ESPRangeView range(int32_t from, int32_t to) {
    from = interpret_index(from, this->size());
    to = interpret_index(to, this->size());
    return ESPRangeView(this, from, to);
  }
  ESPRangeView all() { return ESPRangeView(this, 0, this->size()); }
  ESPRangeIterator begin() { return this->all().begin(); }
  ESPRangeIterator end() { return this->all().end(); }
  void shift_left(int32_t amnt) {
    if (amnt < 0) {
      this->shift_right(-amnt);
      return;
    }
    if (amnt > this->size())
      amnt = this->size();
    this->range(0, -amnt) = this->range(amnt, this->size());
  }
  void shift_right(int32_t amnt) {
    if (amnt < 0) {
      this->shift_left(-amnt);
      return;
    }
    if (amnt > this->size())
      amnt = this->size();
    this->range(amnt, this->size()) = this->range(0, -amnt);
  }
  void set_correction(float red, float green, float blue, float white = 1.0f) {
    this->correction_.set_max_brightness(Color(uint8_t(roundf(red * 255.0f)), uint8_t(roundf(green * 255.0f)),
                                               uint8_t(roundf(blue * 255.0f)), uint8_t(roundf(white * 255.0f))));
  }
  void setup_state(LightState *state) override {
    this->correction_.calculate_gamma_table(state->get_gamma_correct());
    this->corrected_values_ = new ColorCorrectingLightValues(this->get_light_values(), this->correction_);
    this->state_parent_ = state;
  }
  void update_state(LightState *state) override;
  void schedule_show() { this->state_parent_->next_write_ = true; }

  void call_setup() override;

 protected:
  virtual AddressableLightValues& get_light_values() = 0;

  ESPColorCorrection correction_{};
  ColorCorrectingLightValues *corrected_values_{nullptr};
  LightState *state_parent_{nullptr};
  float last_transition_progress_{0.0f};
  float accumulated_alpha_{0.0f};
};

}  // namespace light
}  // namespace esphome
