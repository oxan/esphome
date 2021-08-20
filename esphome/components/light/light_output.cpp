#include "light_output.h"
#include "transitions.h"

namespace esphome {
namespace light {

std::unique_ptr<LightTransition> LightOutput::create_default_transition() { return make_unique<FadeTransition>(); }

}  // namespace light
}  // namespace esphome
