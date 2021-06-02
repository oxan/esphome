#pragma once

#include "addressable_light_values.h"
#include "esp_hsv_color.h"

namespace esphome {
namespace light {

int32_t interpret_index(int32_t index, int32_t size);

class ESPRangeIterator;

class ESPRangeView {
 public:
  ESPRangeView(AddressableLightValues &parent, int32_t begin, int32_t end)
    : parent_(parent), begin_(begin), end_(end < begin ? begin : end) {}

  int32_t size() const { return this->end_ - this->begin_; }
  ESPRangeView operator[](int32_t index) const;
  ESPRangeIterator begin();
  ESPRangeIterator end();

  // All get* methods only make sense on ranges of size 1.
  uint8_t get_red() const { return this->parent_.get_red(this->begin_); }
  uint8_t get_green() const { return this->parent_.get_green(this->begin_); }
  uint8_t get_blue() const { return this->parent_.get_blue(this->begin_); }
  uint8_t get_white() const { return this->parent_.get_white(this->begin_); }
  uint8_t get_effect_data() const { return this->parent_.get_effect_data(this->begin_); }
  Color get() const { return this->parent_.get(this->begin_); }

  void set(const Color &color);
  void set(const ESPHSVColor &color) { this->set(color.to_rgb()); }
  void set_red(uint8_t red);
  void set_green(uint8_t green);
  void set_blue(uint8_t blue);
  void set_white(uint8_t white);
  void set_effect_data(uint8_t effect_data);

  void fade_to_white(uint8_t amnt);
  void fade_to_black(uint8_t amnt);
  void lighten(uint8_t delta);
  void darken(uint8_t delta);

  ESPRangeView all() const { return *this; }
  ESPRangeView range(int32_t from, int32_t to) const;

  void shift_left(int32_t amnt);
  void shift_right(int32_t amnt);

  ESPRangeView &operator=(const Color &rhs) {
    this->set(rhs);
    return *this;
  }
  ESPRangeView &operator=(const ESPHSVColor &rhs) {
    this->set(rhs);
    return *this;
  }
  ESPRangeView &operator=(const ESPRangeView &rhs);

 protected:
  friend ESPRangeIterator;

  AddressableLightValues &parent_;
  int32_t begin_;
  int32_t end_;
};

class ESPRangeIterator {
 public:
  ESPRangeIterator(const ESPRangeView &range, int32_t i) : range_(range), i_(i) {}
  ESPRangeIterator operator++() {
    this->i_++;
    return *this;
  }
  bool operator!=(const ESPRangeIterator &other) const { return this->i_ != other.i_; }
  ESPRangeView operator*() const { return this->range_[this->i_]; }

 protected:
  const ESPRangeView &range_;
  int32_t i_;
};

}  // namespace light
}  // namespace esphome
