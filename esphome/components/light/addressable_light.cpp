#include "addressable_light.h"
#include "esphome/core/log.h"

namespace esphome {
namespace light {

static const char *TAG = "light.addressable";

void AddressableLight::call_setup() {
  this->setup();
  this->light_values_ = this->setup_light_values();

#ifdef ESPHOME_LOG_HAS_VERY_VERBOSE
  this->set_interval(5000, [this]() {
    const char *name = this->state_parent_ == nullptr ? "" : this->state_parent_->get_name().c_str();
    ESP_LOGVV(TAG, "Addressable Light '%s'", name);
    for (int i = 0; i < this->size(); i++) {
      auto color = this->light_values_->get(i);
      ESP_LOGVV(TAG, "  [%2d] Color: R=%3u G=%3u B=%3u W=%3u", i, color.red, color.green, color.blue, color.white);
    }
    ESP_LOGVV(TAG, "");
  });
#endif
}

void AddressableLight::setup_state(LightState *state) {
  this->corrected_values_ = new ColorCorrectingLightValues(*this->light_values_, state->get_color_correction(), 255);
  this->state_parent_ = state;
}

void AddressableLight::update_state(LightState *state) {
  this->last_transition_progress_ = 0.0f;
  this->accumulated_alpha_ = 0.0f;

  // Note: both the RangeView from get_pixels() and the LightColorValues.as_* helpers apply color+gamma
  // correction, so use exactly one of them.

  // Set the brightness used by color correction for future writes through get_pixels().
  auto val = state->current_values;
  this->corrected_values_->set_brightness(to_uint8(state->current_values.get_state() * state->current_values.get_brightness()));

  if (state->transformer_ == nullptr || !state->transformer_->is_transition()) {
    // no transformer active or non-transition one
    this->get_raw_pixels() = val.as_rgbw(state->get_color_correction(), false);
  } else {
    // transition transformer active, activate specialized transition for addressable effects
    // instead of using a unified transition for all LEDs, we use the current state each LED as the
    // start. Warning: ugly

    // We can't use a direct lerp smoothing here though - that would require creating a copy of the original
    // state of each LED at the start of the transition
    // Instead, we "fake" the look of the LERP by using an exponential average over time and using
    // dynamically-calculated alpha values to match the look of the

    float new_progress = state->transformer_->get_progress();
    float prev_smoothed = LightTransitionTransformer::smoothed_progress(last_transition_progress_);
    float new_smoothed = LightTransitionTransformer::smoothed_progress(new_progress);
    this->last_transition_progress_ = new_progress;

    auto end_values = state->transformer_->get_end_values();
    Color target_color = end_values.as_rgbw(ESPColorCorrection(), false);

    // our transition will handle brightness, disable brightness in correction.
    this->corrected_values_->set_brightness(255);
    uint8_t orig_w = target_color.w;
    target_color *= to_uint8(end_values.get_brightness() * end_values.get_state());
    // w is not scaled by brightness
    target_color.w = orig_w;

    float denom = (1.0f - new_smoothed);
    float alpha = denom == 0.0f ? 0.0f : (new_smoothed - prev_smoothed) / denom;

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
      Color add = target_color * alpha8;

      for (auto led : this->get_pixels())
        led = add + led.get() * inv_alpha8;
    }
  }
}

}  // namespace light
}  // namespace esphome
