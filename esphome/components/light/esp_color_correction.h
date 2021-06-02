#pragma once

#include "esphome/core/color.h"

namespace esphome {
namespace light {

class ESPColorCorrection {
 public:
  ESPColorCorrection() : max_brightness_(255, 255, 255, 255), gamma_correction_(1) {
    for (uint16_t i = 0; i < 256; i++)
      this->gamma_table_[i] = this->gamma_reverse_table_[i] = i;
  }

  void set_max_brightness(const Color &max_brightness) { this->max_brightness_ = max_brightness; }
  void set_gamma_correction(float gamma);

  const Color& get_max_brightness() const { return this->max_brightness_; }
  float get_gamma_correction() const { return this->gamma_correction_; }

  inline Color correct(Color color, uint8_t brightness) const ALWAYS_INLINE {
    // corrected = (uncorrected * max_brightness * local_brightness) ^ gamma
    return Color(this->correct_red(color.red, brightness), this->correct_green(color.green, brightness),
                 this->correct_blue(color.blue, brightness), this->correct_white(color.white, brightness));
  }
  inline uint8_t correct_red(uint8_t red, uint8_t brightness) const ALWAYS_INLINE {
    uint8_t res = esp_scale8(esp_scale8(red, this->max_brightness_.red), brightness);
    return this->gamma_table_[res];
  }
  inline uint8_t correct_green(uint8_t green, uint8_t brightness) const ALWAYS_INLINE {
    uint8_t res = esp_scale8(esp_scale8(green, this->max_brightness_.green), brightness);
    return this->gamma_table_[res];
  }
  inline uint8_t correct_blue(uint8_t blue, uint8_t brightness) const ALWAYS_INLINE {
    uint8_t res = esp_scale8(esp_scale8(blue, this->max_brightness_.blue), brightness);
    return this->gamma_table_[res];
  }
  inline uint8_t correct_white(uint8_t white, uint8_t brightness) const ALWAYS_INLINE {
    // do not scale white value with brightness
    uint8_t res = esp_scale8(white, this->max_brightness_.white);
    return this->gamma_table_[res];
  }
  inline Color uncorrect(Color color, uint8_t brightness) const ALWAYS_INLINE {
    // uncorrected = corrected^(1/gamma) / (max_brightness * local_brightness)
    return Color(this->uncorrect_red(color.red, brightness), this->uncorrect_green(color.green, brightness),
                 this->uncorrect_blue(color.blue, brightness), this->uncorrect_white(color.white, brightness));
  }
  inline uint8_t uncorrect_red(uint8_t red, uint8_t brightness) const ALWAYS_INLINE {
    if (this->max_brightness_.red == 0 || brightness == 0)
      return 0;
    uint16_t uncorrected = this->gamma_reverse_table_[red] * 255UL;
    uint8_t res = ((uncorrected / this->max_brightness_.red) * 255UL) / brightness;
    return res;
  }
  inline uint8_t uncorrect_green(uint8_t green, uint8_t brightness) const ALWAYS_INLINE {
    if (this->max_brightness_.green == 0 || brightness == 0)
      return 0;
    uint16_t uncorrected = this->gamma_reverse_table_[green] * 255UL;
    uint8_t res = ((uncorrected / this->max_brightness_.green) * 255UL) / brightness;
    return res;
  }
  inline uint8_t uncorrect_blue(uint8_t blue, uint8_t brightness) const ALWAYS_INLINE {
    if (this->max_brightness_.blue == 0 || brightness == 0)
      return 0;
    uint16_t uncorrected = this->gamma_reverse_table_[blue] * 255UL;
    uint8_t res = ((uncorrected / this->max_brightness_.blue) * 255UL) / brightness;
    return res;
  }
  inline uint8_t uncorrect_white(uint8_t white, uint8_t brightness) const ALWAYS_INLINE {
    if (this->max_brightness_.white == 0)
      return 0;
    uint16_t uncorrected = this->gamma_reverse_table_[white] * 255UL;
    uint8_t res = uncorrected / this->max_brightness_.white;
    return res;
  }

 protected:
  Color max_brightness_;
  float gamma_correction_;
  uint8_t gamma_table_[256];
  uint8_t gamma_reverse_table_[256];
};

}  // namespace light
}  // namespace esphome
