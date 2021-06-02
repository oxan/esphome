#include "light_partition.h"
#include "esphome/core/log.h"

namespace esphome {
namespace partition {

static const char *const TAG = "partition.light";

light::ESPColorView PartitionLightBuffer::find_pixel_(int32_t index) const {
  uint32_t lo = 0;
  uint32_t hi = this->segments_.size() - 1;
  while (lo < hi) {
    uint32_t mid = (lo + hi) / 2;
    int32_t begin = this->segments_[mid].get_dst_offset();
    int32_t end = begin + this->segments_[mid].get_size();
    if (index < begin) {
      hi = mid - 1;
    } else if (index >= end) {
      lo = mid + 1;
    } else {
      lo = hi = mid;
    }
  }
  auto &seg = this->segments_[lo];
  // offset within the segment
  int32_t seg_off = index - seg.get_dst_offset();
  // offset within the src
  int32_t src_off;
  if (seg.is_reversed())
    src_off = seg.get_src_offset() + seg.get_size() - seg_off - 1;
  else
    src_off = seg.get_src_offset() + seg_off;

  return (*seg.get_src()).pixels()[src_off];
}

}  // namespace partition
}  // namespace esphome
