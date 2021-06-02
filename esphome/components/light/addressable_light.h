#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/core/color.h"
#include "esp_color_correction.h"
#include "esp_color_view.h"
#include "esp_range_view.h"
#include "light_output.h"
#include "light_state.h"
#include "transformers.h"

namespace esphome {
namespace light {

using ESPColor ESPDEPRECATED("esphome::light::ESPColor is deprecated, use esphome::Color instead.", "v1.21") = Color;

class AddressableLight : public LightOutput, public Component {
 public:
  /// Not a public API, will be made protected in the future. Use pixels().size() instead.
  virtual int32_t size() const = 0;

  ESPRangeView pixels() { return ESPRangeView{this, 0, this->size()}; }

  // Indicates whether an effect that directly updates the output buffer is active to prevent overwriting
  bool is_effect_active() const { return this->effect_active_; }
  void set_effect_active(bool effect_active) { this->effect_active_ = effect_active; }
  std::unique_ptr<LightTransformer> create_default_transition() override;
  void set_correction(float red, float green, float blue, float white = 1.0f) {
    this->correction_.set_max_brightness(
        Color(to_uint8_scale(red), to_uint8_scale(green), to_uint8_scale(blue), to_uint8_scale(white)));
  }
  void setup_state(LightState *state) override {
    this->correction_.calculate_gamma_table(state->get_gamma_correct());
    this->state_parent_ = state;
  }
  void update_state(LightState *state) override;
  void schedule_show() { this->state_parent_->next_write_ = true; }

  void call_setup() override;

  // Legacy methods
  ESPDEPRECATED("AddressableLight[] is deprecated, use pixels()[] instead", "2021.9")
  ESPColorView operator[](int32_t index) { return this->pixels()[index]; }
  ESPDEPRECATED("AddressableLight.get() is deprecated, use pixels().get() instead", "2021.9")
  ESPColorView get(int32_t index) { return this->pixels()[index]; }
  ESPDEPRECATED("AddressableLight.range() is deprecated, use pixels().range() instead", "2021.9")
  ESPRangeView range(int32_t from, int32_t to) { return this->pixels().range(from, to); }
  ESPDEPRECATED("AddressableLight.all() is deprecated, use pixels() instead", "2021.9")
  ESPRangeView all() { return this->pixels(); }
  ESPDEPRECATED("Iteration over AddressableLight is deprecated, iterate over pixels() instead", "2021.9")
  ESPRangeIterator begin() { return this->pixels().begin(); }
  ESPRangeIterator end() { return this->pixels().end(); }
  ESPDEPRECATED("AddressableLight.shift_left() is deprecated, use pixels().shift_left() instead.", "2021.9")
  void shift_left(int32_t amnt) { return this->pixels().shift_left(amnt); }
  ESPDEPRECATED("AddressableLight.shift_right() is deprecated, use pixels().shift_right() instead.", "2021.9")
  void shift_right(int32_t amnt) { return this->pixels().shift_right(amnt); }

 protected:
  friend class AddressableLightTransformer;
  friend class ESPRangeView;

  virtual ESPColorView get_view_internal(int32_t index) const = 0;

  bool effect_active_{false};
  ESPColorCorrection correction_{};
  LightState *state_parent_{nullptr};
};

class AddressableLightTransformer : public LightTransitionTransformer {
 public:
  AddressableLightTransformer(AddressableLight &light) : light_(light) {}

  void start() override;
  optional<LightColorValues> apply() override;

 protected:
  AddressableLight &light_;
  Color target_color_{};
  float last_transition_progress_{0.0f};
  float accumulated_alpha_{0.0f};
};

}  // namespace light
}  // namespace esphome
