import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ENTITY_ID, CONF_ID, CONF_STATE_TOPIC

countdown_ns = cg.esphome_ns.namespace('countdown')
CountDownTextSensor = countdown_ns.class_('CountDown', text_sensor.TextSensor, cg.Component)

CONFIG_SCHEMA = text_sensor.TEXT_SENSOR_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(CountDownTextSensor),
})


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add_library('CountUpDownTimer', None)
    yield cg.register_component(var, config)
    yield text_sensor.register_text_sensor(var, config)

