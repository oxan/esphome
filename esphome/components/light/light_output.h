#pragma once

#include "esphome/core/component.h"
#include "light_traits.h"
#include "light_state.h"
#include "light_transition.h"

namespace esphome {
namespace light {

/// Interface to write LightStates to hardware.
class LightOutput {
 public:
  /// Return the LightTraits of this LightOutput.
  virtual LightTraits get_traits() = 0;

  /// Return the default transition used.
  virtual std::unique_ptr<LightTransition> create_default_transition();

  virtual void setup_state(LightState *state) {}

  virtual void write_state(LightState *state) = 0;
};

}  // namespace light
}  // namespace esphome
