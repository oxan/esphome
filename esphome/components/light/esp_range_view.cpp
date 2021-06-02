#include "esp_range_view.h"
#include "addressable_light.h"

namespace esphome {
namespace light {

int32_t HOT interpret_index(int32_t index, int32_t size) {
  if (index < 0)
    return size + index;
  return index;
}

ESPColorView ESPRangeView::operator[](int32_t index) const {
  index = interpret_index(index, this->size()) + this->begin_;
  return {this->parent_, index, &this->correction_};
}
ESPRangeIterator ESPRangeView::begin() { return {*this, this->begin_}; }
ESPRangeIterator ESPRangeView::end() { return {*this, this->end_}; }

void ESPRangeView::set(const Color &color) {
  for (auto c : *this)
    c.set(color);
}
void ESPRangeView::set_red(uint8_t red) {
  for (auto c : *this)
    c.set_red(red);
}
void ESPRangeView::set_green(uint8_t green) {
  for (auto c : *this)
    c.set_green(green);
}
void ESPRangeView::set_blue(uint8_t blue) {
  for (auto c : *this)
    c.set_blue(blue);
}
void ESPRangeView::set_white(uint8_t white) {
  for (auto c : *this)
    c.set_white(white);
}
void ESPRangeView::set_effect_data(uint8_t effect_data) {
  for (auto c : *this)
    c.set_effect_data(effect_data);
}

void ESPRangeView::fade_to_white(uint8_t amnt) {
  for (auto c : *this)
    c.fade_to_white(amnt);
}
void ESPRangeView::fade_to_black(uint8_t amnt) {
  for (auto c : *this)
    c.fade_to_black(amnt);
}
void ESPRangeView::lighten(uint8_t delta) {
  for (auto c : *this)
    c.lighten(delta);
}
void ESPRangeView::darken(uint8_t delta) {
  for (auto c : *this)
    c.darken(delta);
}

ESPRangeView ESPRangeView::range(int32_t from, int32_t to) const {
  from = interpret_index(from, this->size());
  to = interpret_index(to, this->size());
  return {this->parent_, this->correction_, this->begin_ + from, this->begin_ + to};
}

void ESPRangeView::shift_left(int32_t amnt) {
  if (amnt < 0) {
    this->shift_right(-amnt);
    return;
  }
  if (amnt > this->size())
    amnt = this->size();
  this->range(0, -amnt) = this->range(amnt, this->size());
}
void ESPRangeView::shift_right(int32_t amnt) {
  if (amnt < 0) {
    this->shift_left(-amnt);
    return;
  }
  if (amnt > this->size())
    amnt = this->size();
  this->range(amnt, this->size()) = this->range(0, -amnt);
}

ESPRangeView &ESPRangeView::operator=(const ESPRangeView &rhs) {  // NOLINT
  // If size doesn't match, error (todo warning)
  if (rhs.size() != this->size())
    return *this;

  if (&this->parent_ != &rhs.parent_) {
    for (int32_t i = 0; i < this->size(); i++)
      (*this)[i].set(rhs[i].get());
    return *this;
  }

  // If both equal, already done
  if (rhs.begin_ == this->begin_)
    return *this;

  if (rhs.begin_ > this->begin_) {
    // Copy from left
    for (int32_t i = 0; i < this->size(); i++) {
      (*this)[i].set(rhs[i].get());
    }
  } else {
    // Copy from right
    for (int32_t i = this->size() - 1; i >= 0; i--) {
      (*this)[i].set(rhs[i].get());
    }
  }

  return *this;
}

}  // namespace light
}  // namespace esphome
