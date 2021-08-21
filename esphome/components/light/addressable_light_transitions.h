#pragma once

#include "addressable_light.h"
#include "light_transition.h"

namespace esphome {
namespace light {

class AddressableFadeTransition : public LightTransition {
 public:
  AddressableFadeTransition(AddressableLight &light) : light_(light) {}

  void start() override {
    // our transition will handle brightness, disable brightness in correction.
    this->light_.correction_.set_local_brightness(255);
    this->target_color_ = esp_color_from_light_color_values(this->target_values_);
    this->target_color_ *= this->target_values_.is_on() ? to_uint8_scale(this->target_values_.get_brightness()) : 0;
  }

  optional<LightColorValues> apply() override {
    // Don't try to transition over running effects, instead immediately use the target values. write_state() and the
    // effects pick up the change from current_values.
    if (this->light_.is_effect_active())
      return this->target_values_;

    // Use a specialized transition for addressable lights: instead of using a unified transition for
    // all LEDs, we use the current state of each LED as the start.

    // We can't use a direct lerp smoothing here though - that would require creating a copy of the original
    // state of each LED at the start of the transition.
    // Instead, we "fake" the look of the LERP by using an exponential average over time and using
    // dynamically-calculated alpha values to match the look.
    float smoothed_progress = AddressableFadeTransition::smoothed_progress(this->get_progress_());
    float denom = (1.0f - smoothed_progress);
    float alpha = denom == 0.0f ? 0.0f : (smoothed_progress - this->last_transition_progress_) / denom;

    // We need to use a low-resolution alpha here which makes the transition set in only after ~half of the length
    // We solve this by accumulating the fractional part of the alpha over time.
    float alpha255;
    this->accumulated_alpha_ = modff(alpha * 255.0f + this->accumulated_alpha_, &alpha255);
    auto alpha8 = static_cast<uint8_t>(alpha255);

    if (alpha8 != 0) {
      uint8_t inv_alpha8 = 255 - alpha8;
      Color add = this->target_color_ * alpha8;

      for (auto led : this->light_)
        led.set(add + led.get() * inv_alpha8);
    }

    this->last_transition_progress_ = smoothed_progress;
    this->light_.schedule_show();

    return {};
  }

 protected:
  static float smoothed_progress(float x) { return x * x * x * (x * (x * 6.0f - 15.0f) + 10.0f); }

  AddressableLight &light_;
  Color target_color_{};
  float last_transition_progress_{0.0f};
  float accumulated_alpha_{0.0f};
};

}  // namespace light
}  // namespace esphome
