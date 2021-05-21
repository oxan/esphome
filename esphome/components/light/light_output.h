#pragma once

#include "esphome/core/component.h"
#include "light_traits.h"
#include "light_state.h"

namespace esphome {
namespace light {

class LightState;

/// Interface to write LightStates to hardware.
class LightOutput {
 public:
  /// Return the LightTraits of this LightOutput.
  virtual LightTraits get_traits() = 0;

  virtual void setup_state(LightState *state) {}

  /// Called on every update of the current values of the associated LightState,
  /// can optionally be used to do processing on this change.
  virtual void update_state(LightState *state) {}

  /// Called from loop() every time the current value of the light state changed,
  /// should write the new state to hardware. Every call to write_state() is
  /// preceded by (at least) one call to update_state().
  virtual void write_state(LightState *state) = 0;
};

}  // namespace light
}  // namespace esphome
