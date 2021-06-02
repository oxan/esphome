#pragma once

#include "esphome/core/color.h"
#include "addressable_light_values.h"
#include "esp_hsv_color.h"
#include "esp_color_correction.h"

namespace esphome {
namespace light {

class ESPColorSettable {
 public:
  virtual void set(const Color &color) = 0;
  virtual void set_red(uint8_t red) = 0;
  virtual void set_green(uint8_t green) = 0;
  virtual void set_blue(uint8_t blue) = 0;
  virtual void set_white(uint8_t white) = 0;
  virtual void set_effect_data(uint8_t effect_data) = 0;
  virtual void fade_to_white(uint8_t amnt) = 0;
  virtual void fade_to_black(uint8_t amnt) = 0;
  virtual void lighten(uint8_t delta) = 0;
  virtual void darken(uint8_t delta) = 0;
  void set(const ESPHSVColor &color) { this->set_hsv(color); }
  void set_hsv(const ESPHSVColor &color) {
    Color rgb = color.to_rgb();
    this->set_rgb(rgb.r, rgb.g, rgb.b);
  }
  void set_rgb(uint8_t red, uint8_t green, uint8_t blue) {
    this->set_red(red);
    this->set_green(green);
    this->set_blue(blue);
  }
  void set_rgbw(uint8_t red, uint8_t green, uint8_t blue, uint8_t white) {
    this->set_rgb(red, green, blue);
    this->set_white(white);
  }
};

class ESPColorView : public ESPColorSettable {
 public:
  ESPColorView(AddressableLightValues& parent, int32_t index)
      : parent_(parent), index_(index) {}
  ESPColorView &operator=(const Color &rhs) {
    this->set(rhs);
    return *this;
  }
  ESPColorView &operator=(const ESPHSVColor &rhs) {
    this->set_hsv(rhs);
    return *this;
  }
  void set(const Color &color) override { this->set_rgbw(color.r, color.g, color.b, color.w); }
  void set_red(uint8_t red) override { this->parent_.set_red(this->index_, red); }
  void set_green(uint8_t green) override { this->parent_.set_green(this->index_, green); }
  void set_blue(uint8_t blue) override { this->parent_.set_blue(this->index_, blue); }
  void set_white(uint8_t white) override { this->parent_.set_white(this->index_, white); }
  void set_effect_data(uint8_t effect_data) override { this->parent_.set_effect_data(this->index_, effect_data); }
  void fade_to_white(uint8_t amnt) override { this->set(this->get().fade_to_white(amnt)); }
  void fade_to_black(uint8_t amnt) override { this->set(this->get().fade_to_black(amnt)); }
  void lighten(uint8_t delta) override { this->set(this->get().lighten(delta)); }
  void darken(uint8_t delta) override { this->set(this->get().darken(delta)); }
  Color get() const { return Color(this->get_red(), this->get_green(), this->get_blue(), this->get_white()); }
  uint8_t get_red() const { return this->parent_.get_red(this->index_); }
  uint8_t get_green() const { return this->parent_.get_green(this->index_); }
  uint8_t get_blue() const { return this->parent_.get_blue(this->index_); }
  uint8_t get_white() const { return this->parent_.get_white(this->index_); }
  uint8_t get_effect_data() const { return this->parent_.get_effect_data(this->index_); }

 protected:
  AddressableLightValues& parent_;
  const int32_t index_;
};

}  // namespace light
}  // namespace esphome
