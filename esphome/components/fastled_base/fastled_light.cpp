#include "fastled_light.h"
#include "esphome/core/log.h"

namespace esphome {
namespace fastled_base {

static const char *TAG = "fastled";

void FastLEDLightOutput::setup() {
  ESP_LOGCONFIG(TAG, "Setting up FastLED light...");
  this->controller_->init();
  this->controller_->setLeds(this->light_values_->leds_, this->num_leds_);
  if (!this->max_refresh_rate_.has_value()) {
    this->set_max_refresh_rate(this->controller_->getMaxRefreshRate());
  }
}
void FastLEDLightOutput::dump_config() {
  ESP_LOGCONFIG(TAG, "FastLED light:");
  ESP_LOGCONFIG(TAG, "  Num LEDs: %u", this->num_leds_);
  ESP_LOGCONFIG(TAG, "  Max refresh rate: %u", *this->max_refresh_rate_);
}
void FastLEDLightOutput::write_state(light::LightState *state) {
  // protect from refreshing too often
  uint32_t now = micros();
  if (*this->max_refresh_rate_ != 0 && (now - this->last_refresh_) < *this->max_refresh_rate_) {
    return;
  }
  this->last_refresh_ = now;

  ESP_LOGVV(TAG, "Writing RGB values to bus...");
  this->controller_->showLeds();
}

}  // namespace fastled_base
}  // namespace esphome
