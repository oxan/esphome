#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/core/color.h"
#include "addressable_light_values.h"
#include "esp_range_view.h"
#include "light_output.h"
#include "light_state.h"

namespace esphome {
namespace light {

class AddressableLight : public LightOutput, public Component {
 public:
  void call_setup() override;

  void setup_state(LightState *state) override;
  void update_state(LightState *state) override;

  void schedule_show() { this->state_parent_->next_write_ = true; }

  ESPRangeView get_raw_pixels() { return ESPRangeView{*this->light_values_, 0, this->size()}; }
  ESPRangeView get_pixels() { return ESPRangeView{*this->corrected_values_, 0, this->size()}; }

  // Compatibility methods
  ESPDEPRECATED("AddressableLight[] is deprecated, use AddressableLight.get_pixels()[] instead.")
  ESPRangeView operator[](int32_t index) { return this->get_pixels()[index]; }
  ESPDEPRECATED("AddressableLight.get() is deprecated, use AddressableLight.get_pixels().get() instead.")
  ESPRangeView get(int32_t index) { return this->get_pixels()[index]; }
  ESPDEPRECATED("AddressableLight.range() is deprecated, use AddressableLight.get_pixels().range() instead.")
  ESPRangeView range(int32_t from, int32_t to) { return this->get_pixels().range(from, to); }
  ESPDEPRECATED("AddressableLight.all() is deprecated, use AddressableLight.get_pixels() instead.")
  ESPRangeView all() { return this->get_pixels(); }
  ESPDEPRECATED("Iteration over AddressableLight is deprecated, iterate over AddressableLight.get_pixels() instead.")
  ESPRangeIterator begin() { return this->get_pixels().begin(); }
  ESPRangeIterator end() { return this->get_pixels().end(); }
  ESPDEPRECATED("AddressableLight.shift_left() is deprecated, use AddressableLight.get_pixels().shift_left() instead.")
  void shift_left(int32_t amnt) { return this->get_pixels().shift_left(amnt); }
  ESPDEPRECATED("AddressableLight.shift_right() is deprecated, use AddressableLight.get_pixels().shift_right() instead.")
  void shift_right(int32_t amnt) { return this->get_pixels().shift_right(amnt); }

 protected:
  virtual int32_t size() const = 0;
  virtual AddressableLightValues* setup_light_values() = 0;

  AddressableLightValues *light_values_{nullptr};
  ColorCorrectingLightValues *corrected_values_{nullptr};
  LightState *state_parent_{nullptr};
  float last_transition_progress_{0.0f};
  float accumulated_alpha_{0.0f};
};

// Compatibility types
using ESPColor ESPDEPRECATED("ESPColor is deprecated, use Color instead.") = Color;
using ESPColorView ESPDEPRECATED("ESPColorView is deprecated, use a (one-item) ESPRangeView instead.") = ESPRangeView;

}  // namespace light
}  // namespace esphome
