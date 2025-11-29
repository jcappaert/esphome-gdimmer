import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.components import sensor, light, output

gesture_ns = cg.esphome_ns.namespace("gdimmer")
GestureDimmer = gesture_ns.class_("GestureDimmer", cg.Component)

CONF_SENSOR = "sensor"
CONF_LIGHT  = "light"
CONF_LED    = "led"

CONF_TOGGLE_HIGH     = "toggle_high"
CONF_TOGGLE_LOW      = "toggle_low"
CONF_COOLDOWN_MS     = "cooldown_ms"
CONF_HOLD_TIME_MS    = "hold_time_ms"
CONF_STEP_INTERVAL   = "step_interval_ms"
CONF_DIM_STEP        = "dim_step"
CONF_B_MIN           = "b_min"
CONF_B_MAX           = "b_max"
CONF_BLINK_PERIOD_MS = "blink_period_ms"
CONF_ALT_DIRECTION   = "alternate_direction"
CONF_RISE_GRACE_MS   = "rise_grace_ms"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(GestureDimmer),

    cv.Required(CONF_SENSOR): cv.use_id(sensor.Sensor),
    cv.Required(CONF_LIGHT):  cv.use_id(light.LightState),
    cv.Optional(CONF_LED):    cv.use_id(output.BinaryOutput),

    cv.Optional(CONF_TOGGLE_HIGH, default=180.0): cv.float_,
    cv.Optional(CONF_TOGGLE_LOW,  default=80.0):  cv.float_,
    cv.Optional(CONF_COOLDOWN_MS,  default=600):  cv.positive_int,
    cv.Optional(CONF_HOLD_TIME_MS, default=500):  cv.positive_int,
    cv.Optional(CONF_STEP_INTERVAL, default=60):  cv.positive_int,
    cv.Optional(CONF_DIM_STEP, default=0.03):     cv.float_,
    cv.Optional(CONF_B_MIN, default=0.05):        cv.float_,
    cv.Optional(CONF_B_MAX, default=1.00):        cv.float_,
    cv.Optional(CONF_BLINK_PERIOD_MS, default=300): cv.positive_int,
    cv.Optional(CONF_ALT_DIRECTION, default=True): cv.boolean,
    cv.Optional(CONF_RISE_GRACE_MS, default=900): cv.positive_int,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])

    await cg.register_component(var, config)

    sens = await cg.get_variable(config[CONF_SENSOR])
    cg.add(var.set_sensor(sens))

    lig = await cg.get_variable(config[CONF_LIGHT])
    cg.add(var.set_light(lig))

    if CONF_LED in config:
        led = await cg.get_variable(config[CONF_LED])
        cg.add(var.set_led(led))

    cg.add(var.set_thresholds(
        config[CONF_TOGGLE_HIGH],
        config[CONF_TOGGLE_LOW]
    ))
    cg.add(var.set_timing(
        config[CONF_COOLDOWN_MS],
        config[CONF_HOLD_TIME_MS],
        config[CONF_STEP_INTERVAL],
        config[CONF_BLINK_PERIOD_MS]
    ))
    cg.add(var.set_dimming_params(
        config[CONF_DIM_STEP],
        config[CONF_B_MIN],
        config[CONF_B_MAX],
        config[CONF_ALT_DIRECTION]
    ))
    cg.add(var.set_rise_grace_ms(config[CONF_RISE_GRACE_MS]))
