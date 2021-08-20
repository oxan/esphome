#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "light_color_values.h"
#include "light_state.h"
#include "light_transition.h"

namespace esphome {
namespace light {

class FadeTransition : public LightTransition {
 public:
  void start() override {
    // When turning light on from off state, use target state and only increase brightness from zero.
    if (!this->start_values_.is_on() && this->target_values_.is_on()) {
      this->start_values_ = LightColorValues(this->target_values_);
      this->start_values_.set_brightness(0.0f);
    }

    // When turning light off from on state, use source state and only decrease brightness to zero.
    if (this->start_values_.is_on() && !this->target_values_.is_on()) {
      this->target_values_ = LightColorValues(this->start_values_);
      this->target_values_.set_brightness(0.0f);
    }

    // When changing color mode, go through off state, as color modes are orthogonal and there can't be two active.
    if (this->start_values_.get_color_mode() != this->target_values_.get_color_mode()) {
      this->changing_color_mode_ = true;
      this->intermediate_values_ = this->start_values_;
      this->intermediate_values_.set_state(false);
    }
  }

  optional<LightColorValues> apply() override {
    float p = this->get_progress_();

    // Halfway through, when intermediate state (off) is reached, flip it to the target, but remain off.
    if (this->changing_color_mode_ && p > 0.5f &&
        this->intermediate_values_.get_color_mode() != this->target_values_.get_color_mode()) {
      this->intermediate_values_ = this->target_values_;
      this->intermediate_values_.set_state(false);
    }

    LightColorValues &start = this->changing_color_mode_ && p > 0.5f ? this->intermediate_values_ : this->start_values_;
    LightColorValues &end = this->changing_color_mode_ && p < 0.5f ? this->intermediate_values_ : this->target_values_;
    if (this->changing_color_mode_)
      p = p < 0.5f ? p * 2 : (p - 0.5) * 2;

    float v = FadeTransition::smoothed_progress(p);
    return LightColorValues::lerp(start, end, v);
  }

 protected:
  // This looks crazy, but it reduces to 6x^5 - 15x^4 + 10x^3 which is just a smooth sigmoid-like
  // transition from 0 to 1 on x = [0, 1]
  static float smoothed_progress(float x) { return x * x * x * (x * (x * 6.0f - 15.0f) + 10.0f); }

  bool changing_color_mode_{false};
  LightColorValues intermediate_values_{};
};

}  // namespace light
}  // namespace esphome
