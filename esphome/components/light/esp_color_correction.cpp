#include "esp_color_correction.h"
#include "esphome/core/log.h"

namespace esphome {
namespace light {

void ESPColorCorrection::set_gamma_correction(float gamma) {
  this->gamma_correction_ = gamma;

  for (uint16_t i = 0; i < 256; i++) {
    // corrected = val ^ gamma
    auto corrected = to_uint8(gamma_correct(i / 255.0f, gamma));
    this->gamma_table_[i] = corrected;
  }
  if (gamma == 0.0f) {
    for (uint16_t i = 0; i < 256; i++)
      this->gamma_reverse_table_[i] = i;
    return;
  }
  for (uint16_t i = 0; i < 256; i++) {
    // val = corrected ^ (1/gamma)
    auto uncorrected = to_uint8(powf(i / 255.0f, 1.0f / gamma));
    this->gamma_reverse_table_[i] = uncorrected;
  }
}

}  // namespace light
}  // namespace esphome
