#pragma once

#include "esphome/core/color.h"
#include "esp_color_correction.h"

namespace esphome {
namespace light {

class AddressableLightValues {
 public:
  virtual uint8_t get_red(int32_t index) const = 0;
  virtual uint8_t get_green(int32_t index) const = 0;
  virtual uint8_t get_blue(int32_t index) const = 0;
  virtual uint8_t get_white(int32_t index) const = 0;
  virtual uint8_t get_effect_data(int32_t index) const = 0;
  virtual Color get(int32_t index) const {
    return Color(this->get_red(index), this->get_green(index), this->get_blue(index), this->get_white(index));
  }

  virtual void set_red(int32_t index, uint8_t red) = 0;
  virtual void set_green(int32_t index, uint8_t green) = 0;
  virtual void set_blue(int32_t index, uint8_t blue) = 0;
  virtual void set_white(int32_t index, uint8_t white) = 0;
  virtual void set_effect_data(int32_t index, uint8_t effect_data) = 0;
  virtual void set(int32_t index, const Color& color) {
    this->set_red(index, color.red);
    this->set_green(index, color.green);
    this->set_blue(index, color.blue);
    this->set_white(index, color.white);
  }
};

class ColorCorrectingLightValues : public AddressableLightValues {
 public:
  ColorCorrectingLightValues(AddressableLightValues& parent, const ESPColorCorrection& correction)
      : parent_(parent), correction_(correction) {}

  uint8_t get_red(int32_t index) const override {
    return this->correction_.color_uncorrect_red(this->parent_.get_red(index));
  }
  uint8_t get_green(int32_t index) const override {
    return this->correction_.color_uncorrect_green(this->parent_.get_green(index));
  }
  uint8_t get_blue(int32_t index) const override {
    return this->correction_.color_uncorrect_blue(this->parent_.get_blue(index));
  }
  uint8_t get_white(int32_t index) const override {
    return this->correction_.color_uncorrect_white(this->parent_.get_white(index));
  }
  uint8_t get_effect_data(int32_t index) const override {
    return this->parent_.get_effect_data(index);
  }
  void set_red(int32_t index, uint8_t red) override {
    this->parent_.set_red(index, this->correction_.color_correct_red(red));
  }
  void set_green(int32_t index, uint8_t green) override {
    this->parent_.set_green(index, this->correction_.color_correct_green(green));
  }
  void set_blue(int32_t index, uint8_t blue) override {
    this->parent_.set_blue(index, this->correction_.color_correct_blue(blue));
  }
  void set_white(int32_t index, uint8_t white) override {
    this->parent_.set_white(index, this->correction_.color_correct_white(white));
  }
  void set_effect_data(int32_t index, uint8_t effect_data) override {
    this->parent_.set_effect_data(index, effect_data);
  }

 protected:
  AddressableLightValues& parent_;
  const ESPColorCorrection& correction_;
};

}  // namespace light
}  // namespace esphome