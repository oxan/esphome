#include "addressable_light.h"
#include "esphome/core/log.h"
#include "addressable_light_transitions.h"

namespace esphome {
namespace light {

static const char *const TAG = "light.addressable";

void AddressableLight::call_setup() {
  this->setup();

#ifdef ESPHOME_LOG_HAS_VERY_VERBOSE
  this->set_interval(5000, [this]() {
    const char *name = this->state_parent_ == nullptr ? "" : this->state_parent_->get_name().c_str();
    ESP_LOGVV(TAG, "Addressable Light '%s' (effect_active=%s next_show=%s)", name, YESNO(this->effect_active_),
              YESNO(this->next_show_));
    for (int i = 0; i < this->size(); i++) {
      auto color = this->get(i);
      ESP_LOGVV(TAG, "  [%2d] Color: R=%3u G=%3u B=%3u W=%3u", i, color.get_red_raw(), color.get_green_raw(),
                color.get_blue_raw(), color.get_white_raw());
    }
    ESP_LOGVV(TAG, " ");
  });
#endif
}

std::unique_ptr<LightTransition> AddressableLight::create_default_transition() {
  return make_unique<AddressableFadeTransition>(*this);
}

Color esp_color_from_light_color_values(LightColorValues val) {
  auto r = to_uint8_scale(val.get_color_brightness() * val.get_red());
  auto g = to_uint8_scale(val.get_color_brightness() * val.get_green());
  auto b = to_uint8_scale(val.get_color_brightness() * val.get_blue());
  auto w = to_uint8_scale(val.get_white());
  return Color(r, g, b, w);
}

void AddressableLight::write_state(LightState *state) {
  auto val = state->current_values;
  auto max_brightness = val.is_on() ? to_uint8_scale(val.get_brightness()) : 0;
  this->correction_.set_local_brightness(max_brightness);

  if (this->is_effect_active())
    return;

  // don't use LightState helper, gamma correction+brightness is handled by ESPColorView
  this->all() = esp_color_from_light_color_values(val);
}

}  // namespace light
}  // namespace esphome
