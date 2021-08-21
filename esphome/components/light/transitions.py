import esphome.codegen as cg
import esphome.config_validation as cv

from esphome.util import Registry
from esphome.const import CONF_NAME, CONF_LAMBDA, CONF_UPDATE_INTERVAL
from .types import (
    AddressableLight,
    LightColorValues,
    FadeTransition,
    LambdaTransition,
    AddressableFadeTransition,
    AddressableLambdaTransition,
)

OUTPUT_TRANSITIONS = {}

TRANSITIONS_REGISTRY = Registry()


def register_generic_transition(
    name, transition_type, default_name, schema, *extra_validators
):
    schema = cv.Schema(schema).extend(
        {
            cv.Optional(CONF_NAME, default=default_name): cv.string_strict,
        }
    )
    validator = cv.All(schema, *extra_validators)
    return TRANSITIONS_REGISTRY.register(name, transition_type, validator)


def register_output_transition(
    output, name, transition_type, default_name, schema, *extra_validators
):
    OUTPUT_TRANSITIONS[name] = output
    return register_generic_transition(
        name, transition_type, default_name, schema, *extra_validators
    )


@register_generic_transition("fade", FadeTransition, "Fade", {})
async def fade_transition_to_code(config, transition_id):
    return cg.new_Pvariable(transition_id, config[CONF_NAME])


@register_generic_transition(
    "lambda",
    LambdaTransition,
    "Lambda",
    {
        cv.Required(CONF_LAMBDA): cv.lambda_,
        cv.Optional(CONF_UPDATE_INTERVAL, default="0ms"): cv.update_interval,
    },
)
async def lambda_transition_to_code(config, transition_id):
    lambda_ = await cg.process_lambda(
        config[CONF_LAMBDA],
        [
            (LightColorValues.operator("constref"), "start"),
            (LightColorValues.operator("constref"), "target"),
            (float, "x"),
        ],
        return_type=cg.optional.template(LightColorValues),
    )
    return cg.new_Pvariable(
        transition_id, config[CONF_NAME], config[CONF_UPDATE_INTERVAL], lambda_
    )


@register_output_transition(
    AddressableLight, "addressable_fade", AddressableFadeTransition, "Fade", {}
)
async def addressable_fade_transition_to_code(config, transition_id):
    return cg.new_Pvariable(transition_id, config[CONF_NAME])


@register_output_transition(
    AddressableLight,
    "addressable_lambda",
    AddressableLambdaTransition,
    "Lambda",
    {
        cv.Required(CONF_LAMBDA): cv.lambda_,
        cv.Optional(CONF_UPDATE_INTERVAL, default="0ms"): cv.update_interval,
    },
)
async def addressable_lambda_transition_to_code(config, transition_id):
    lambda_ = await cg.process_lambda(
        config[CONF_LAMBDA],
        [
            (AddressableLight.operator("ref"), "output"),
            (LightColorValues.operator("constref"), "start"),
            (LightColorValues.operator("constref"), "target"),
            (float, "x"),
        ],
        return_type=cg.optional.template(LightColorValues),
    )
    return cg.new_Pvariable(
        transition_id, config[CONF_NAME], config[CONF_UPDATE_INTERVAL], lambda_
    )


def validate_transitions(light_type=None):
    def validator(value):
        value = cv.validate_registry("transition", TRANSITIONS_REGISTRY)(value)
        errors = []
        names = set()
        for i, x in enumerate(value):
            key = next(it for it in x.keys())
            if key in OUTPUT_TRANSITIONS and (
                light_type is None
                or not light_type.inherits_from(OUTPUT_TRANSITIONS[key])
            ):
                errors.append(
                    cv.Invalid(
                        "The transition '{}' is not allowed for this light type".format(
                            key
                        ),
                        [i],
                    )
                )
                continue

            name = x[key][CONF_NAME]
            if name in names:
                errors.append(
                    cv.Invalid(
                        "Found the transition name '{}' twice. All transitions must have unique names".format(
                            name
                        ),
                        [i],
                    )
                )
                continue

            names.add(name)

        if errors:
            raise cv.MultipleInvalid(errors)
        return value

    return validator
