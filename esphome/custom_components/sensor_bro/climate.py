import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_PIN, CONF_SENSOR
from esphome.components import climate, sensor
from esphome import pins

import esphome.codegen as cg
from esphome.cpp_helpers import gpio_pin_expression

# AUTO_LOAD = ['sensor']

sensor_bro_ns = cg.esphome_ns.namespace('sensor_bro')
SensorBroClimate = sensor_bro_ns.class_('SensorBroClimate', climate.Climate, cg.Component)

CONFIG_SCHEMA = climate.CLIMATE_SCHEMA.extend({
    cv.Required(CONF_PIN): pins.gpio_input_pin_schema,
    cv.Optional(CONF_SENSOR): cv.use_id(sensor.Sensor),
    cv.GenerateID(): cv.declare_id(SensorBroClimate),
}).extend(cv.COMPONENT_SCHEMA)


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add_library('IRRemoteESP8266', None) # cg.add_library('IRRemoteESP8266', '2.7.7')
    yield cg.register_component(var, config)
    yield climate.register_climate(var, config)

    # cg.add(var.set_pin(config[CONF_PIN]))
    pin = yield gpio_pin_expression(config[CONF_PIN])
    cg.add(var.set_pin(pin))

    if CONF_SENSOR in config:
        sens = yield cg.get_variable(config[CONF_SENSOR])
        cg.add(var.set_sensor(sens))

