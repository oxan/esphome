#pragma once

#include "esphome/core/color.h"

namespace esphome {
namespace light {

class AddressableLightBuffer {
 public:
  virtual uint32_t size() const = 0;

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
  virtual void set(int32_t index, const Color &color) {
    this->set_red(index, color.red);
    this->set_green(index, color.green);
    this->set_blue(index, color.blue);
    this->set_white(index, color.white);
  }
};

}  // namespace light
}  // namespace esphome
