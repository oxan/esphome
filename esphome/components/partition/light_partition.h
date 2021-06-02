#pragma once

#include <utility>

#include "esphome/core/component.h"
#include "esphome/components/light/addressable_light.h"

namespace esphome {
namespace partition {

class AddressableSegment {
 public:
  AddressableSegment(light::LightState *src, int32_t src_offset, int32_t size, bool reversed)
      : src_(static_cast<light::AddressableLight *>(src->get_output())),
        src_offset_(src_offset),
        size_(size),
        reversed_(reversed) {}

  light::AddressableLight *get_src() const { return this->src_; }
  int32_t get_src_offset() const { return this->src_offset_; }
  int32_t get_size() const { return this->size_; }
  int32_t get_dst_offset() const { return this->dst_offset_; }
  void set_dst_offset(int32_t dst_offset) { this->dst_offset_ = dst_offset; }
  bool is_reversed() const { return this->reversed_; }

 protected:
  light::AddressableLight *src_;
  int32_t src_offset_;
  int32_t size_;
  int32_t dst_offset_;
  bool reversed_;
};

class PartitionedLightValues : public light::AddressableLightValues {
 public:
  PartitionedLightValues(const std::vector<AddressableSegment>& segments) : segments_(segments) {}

  uint8_t get_red(int32_t index) const override { return this->find_pixel(index).get_red(); }
  uint8_t get_green(int32_t index) const override { return this->find_pixel(index).get_green(); }
  uint8_t get_blue(int32_t index) const override { return this->find_pixel(index).get_blue(); }
  uint8_t get_white(int32_t index) const override { return this->find_pixel(index).get_white(); }
  uint8_t get_effect_data(int32_t index) const override { return this->find_pixel(index).get_effect_data(); }
  void set_red(int32_t index, uint8_t red) override { this->find_pixel(index).set_red(red); }
  void set_green(int32_t index, uint8_t green) override { this->find_pixel(index).set_green(green); }
  void set_blue(int32_t index, uint8_t blue) override { this->find_pixel(index).set_blue(blue); }
  void set_white(int32_t index, uint8_t white) override { this->find_pixel(index).set_white(white); }
  void set_effect_data(int32_t index, uint8_t effect_data) override { this->find_pixel(index).set_effect_data(effect_data); }

 protected:
  light::ESPRangeView find_pixel(int32_t index) const;

  const std::vector<AddressableSegment>& segments_;
};

class PartitionLightOutput : public light::AddressableLight {
 public:
  explicit PartitionLightOutput(std::vector<AddressableSegment> segments) : segments_(std::move(segments)), light_values_(segments_) {
    int32_t off = 0;
    for (auto &seg : this->segments_) {
      seg.set_dst_offset(off);
      off += seg.get_size();
    }
  }
  int32_t size() const override {
    auto &last_seg = this->segments_[this->segments_.size() - 1];
    return last_seg.get_dst_offset() + last_seg.get_size();
  }
  light::LightTraits get_traits() override { return this->segments_[0].get_src()->get_traits(); }
  void write_state(light::LightState *state) override {
    for (auto seg : this->segments_) {
      seg.get_src()->schedule_show();
    }
  }

 protected:
  light::AddressableLightValues& get_light_values() override { return this->light_values_; }

  std::vector<AddressableSegment> segments_;
  PartitionedLightValues light_values_;
};

}  // namespace partition
}  // namespace esphome
