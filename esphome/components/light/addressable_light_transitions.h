#pragma once

#include <functional>
#include "addressable_light.h"
#include "light_transition.h"

namespace esphome {
namespace light {

class AddressableLightTransition : public LightTransition {
 public:
  explicit AddressableLightTransition(const std::string &name) : LightTransition(name) {}

 protected:
  AddressableLight &get_addressable_() const { return *(AddressableLight *) this->state_->get_output(); }
};

class AddressableCascadeTransition : public AddressableLightTransition {
 public:
  AddressableCascadeTransition(const std::string &name, uint32_t fade_length, uint16_t group_size, bool reverse)
      : AddressableLightTransition(name), fade_length_{fade_length}, group_size_{group_size}, reverse_{reverse} {}

  void start() override {
    // our transition will handle brightness, disable brightness in correction.
    this->get_addressable_().correction_.set_local_brightness(255);

    this->group_count_ = this->get_addressable_().size() / this->group_size_;
    this->fade_duration_ =
        this->fade_length_ > 0.0f ? clamp((float) this->fade_length_ / this->length_, 0.0f, 1.0f) : 0.25f;
    this->start_ = esp_color_from_light_color_values(this->get_start_values());
    this->start_ *= this->start_values_.is_on() ? to_uint8_scale(this->target_values_.get_brightness()) : 0;
    this->target_ = esp_color_from_light_color_values(this->get_target_values());
    this->target_ *= this->target_values_.is_on() ? to_uint8_scale(this->target_values_.get_brightness()) : 0;
  }

  optional<LightColorValues> apply() override {
    AddressableLight &it = this->get_addressable_();
    float progress = this->get_progress_();

    for (uint16_t group = 0; group < this->group_count_; group++) {
      float group_start = (float) group / (this->group_count_ - 1) * (1.0f - this->fade_duration_);
      float group_progress = clamp((progress - group_start) / this->fade_duration_, 0.0f, 1.0f);
      Color color = this->lerp_(group_progress);
      if (!this->reverse_)
        it.range(group * this->group_size_, (group + 1) * this->group_size_) = color;
      else
        it.range(it.size() - (group + 1) * this->group_size_, it.size() - group * this->group_size_) = color;
    }

    it.schedule_show();
    return {};
  }

 protected:
  Color lerp_(float progress) {
    return Color(esphome::lerp(progress, this->start_.red, this->target_.red),
                 esphome::lerp(progress, this->start_.green, this->target_.green),
                 esphome::lerp(progress, this->start_.blue, this->target_.blue),
                 esphome::lerp(progress, this->start_.white, this->target_.white));
  }

  uint32_t fade_length_{0};
  uint16_t group_size_{1};
  bool reverse_{false};

  uint16_t group_count_;
  float fade_duration_;
  Color start_;
  Color target_;
};

class AddressableFadeTransition : public AddressableLightTransition {
 public:
  explicit AddressableFadeTransition(const std::string &name) : AddressableLightTransition(name) {}

  void start() override {
    // our transition will handle brightness, disable brightness in correction.
    this->get_addressable_().correction_.set_local_brightness(255);
    this->target_color_ = esp_color_from_light_color_values(this->target_values_);
    this->target_color_ *= this->target_values_.is_on() ? to_uint8_scale(this->target_values_.get_brightness()) : 0;
  }

  optional<LightColorValues> apply() override {
    // Don't try to transition over running effects, instead immediately use the target values. write_state() and the
    // effects pick up the change from current_values.
    if (this->get_addressable_().is_effect_active())
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

      for (auto led : this->get_addressable_().all())
        led.set(add + led.get() * inv_alpha8);
    }

    this->last_transition_progress_ = smoothed_progress;
    this->get_addressable_().schedule_show();

    return {};
  }

 protected:
  static float smoothed_progress(float x) { return x * x * x * (x * (x * 6.0f - 15.0f) + 10.0f); }

  Color target_color_{};
  float last_transition_progress_{0.0f};
  float accumulated_alpha_{0.0f};
};

class AddressableLambdaTransition : public AddressableLightTransition {
 public:
  using lambda_t = std::function<optional<LightColorValues>(AddressableLight &, const LightColorValues &,
                                                            const LightColorValues &, float)>;

  AddressableLambdaTransition(const std::string &name, uint32_t update_interval, lambda_t f)
      : AddressableLightTransition(name), f_(std::move(f)), update_interval_(update_interval) {}

  optional<LightColorValues> apply() override {
    const uint32_t now = millis();
    if (now - this->last_run_ >= this->update_interval_) {
      this->last_run_ = now;
      return this->f_(this->get_addressable_(), this->get_start_values(), this->get_target_values(),
                      this->get_progress_());
    }
    return {};
  }

 protected:
  lambda_t f_;
  uint32_t update_interval_;
  uint32_t last_run_{0};
};

}  // namespace light
}  // namespace esphome
