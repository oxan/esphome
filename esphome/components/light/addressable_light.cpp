#include "addressable_light.h"
#include "esphome/core/log.h"

namespace esphome {
namespace light {

static const char *const TAG = "light.addressable";

void AddressableLight::call_setup() {
  this->setup();
  this->buffer_ = this->create_buffer();

#ifdef ESPHOME_LOG_HAS_VERY_VERBOSE
  this->set_interval(5000, [this]() {
    const char *name = this->state_parent_ == nullptr ? "" : this->state_parent_->get_name().c_str();
    ESP_LOGVV(TAG, "Addressable Light '%s' (effect_active=%s)", name, YESNO(this->effect_active_));
    for (int i = 0; i < this->buffer_->size(); i++) {
      ESP_LOGVV(TAG, "  [%2d] Color: R=%3u G=%3u B=%3u W=%3u", i, this->buffer_->get_red(i),
                this->buffer_->get_green(i), this->buffer_->get_blue(i), this->buffer_->get_white(i));
    }
    ESP_LOGVV(TAG, " ");
  });
#endif
}

std::unique_ptr<LightTransformer> AddressableLight::create_default_transition() {
  return make_unique<AddressableLightTransformer>(*this);
}

Color esp_color_from_light_color_values(LightColorValues val) {
  auto r = to_uint8_scale(val.get_color_brightness() * val.get_red());
  auto g = to_uint8_scale(val.get_color_brightness() * val.get_green());
  auto b = to_uint8_scale(val.get_color_brightness() * val.get_blue());
  auto w = to_uint8_scale(val.get_white());
  return Color(r, g, b, w);
}

void AddressableLight::update_state(LightState *state) {
  const LightColorValues &val = this->state_parent_->current_values;
  this->brightness_ = val.is_on() ? to_uint8_scale(val.get_brightness()) : 0;

  if (this->is_effect_active())
    return;

  // don't use LightState helper, gamma correction+brightness is handled by ESPRangeView
  this->pixels() = esp_color_from_light_color_values(state->current_values);
  this->schedule_show();
}

void AddressableLightTransformer::start() {
  // don't try to transition over running effects.
  if (this->light_.is_effect_active())
    return;

  auto end_values = this->target_values_;
  this->target_color_ = esp_color_from_light_color_values(end_values);
  this->target_color_ *= to_uint8_scale(end_values.get_brightness() * end_values.get_state());
}

optional<LightColorValues> AddressableLightTransformer::apply() {
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

  float smoothed_progress = LightTransitionTransformer::smoothed_progress(this->get_progress_());

  float denom = (1.0f - smoothed_progress);
  float alpha = denom == 0.0f ? 0.0f : (smoothed_progress - this->last_transition_progress_) / denom;

  // We need to use a low-resolution alpha here which makes the transition set in only after ~half of the length
  // We solve this by accumulating the fractional part of the alpha over time.
  float alpha255 = alpha * 255.0f;
  float alpha255int = floorf(alpha255);
  float alpha255remainder = alpha255 - alpha255int;

  this->accumulated_alpha_ += alpha255remainder;
  float alpha_add = floorf(this->accumulated_alpha_);
  this->accumulated_alpha_ -= alpha_add;

  alpha255 += alpha_add;
  alpha255 = clamp(alpha255, 0.0f, 255.0f);
  auto alpha8 = static_cast<uint8_t>(alpha255);

  if (alpha8 != 0) {
    uint8_t inv_alpha8 = 255 - alpha8;
    Color add = this->target_color_ * alpha8;

    for (auto led : this->pixels_view_)
      led.set(add + led.get() * inv_alpha8);
  }

  this->last_transition_progress_ = smoothed_progress;
  this->light_.schedule_show();

  return {};
}

}  // namespace light
}  // namespace esphome
