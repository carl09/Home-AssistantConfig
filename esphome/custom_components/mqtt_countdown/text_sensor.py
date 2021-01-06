import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor, mqtt
from esphome.const import CONF_ID, CONF_QOS, CONF_TOPIC, CONF_TRIGGER_ID
from esphome import automation

DEPENDENCIES = ['mqtt']

CONF_MQTT_PARENT_ID = 'mqtt_parent_id'
CONF_ON_START = 'on_start'
CONF_ON_END = 'on_end'
# CONF_ON_SUNRISE = 'on_end'

mqtt_countdown_ns = cg.esphome_ns.namespace('mqtt_countdown')
MqttCountDownSensor = mqtt_countdown_ns.class_('MqttCountDown', text_sensor.TextSensor, cg.Component)
MqttCountDownTrigger = mqtt_countdown_ns.class_('MqttCountDownTrigger', cg.PollingComponent, automation.Trigger.template())

CONFIG_SCHEMA = text_sensor.TEXT_SENSOR_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(MqttCountDownSensor),
    cv.GenerateID(CONF_MQTT_PARENT_ID): cv.use_id(mqtt.MQTTClientComponent),
    cv.Required(CONF_TOPIC): cv.subscribe_topic,
    cv.Optional(CONF_QOS, default=0): cv.mqtt_qos,

    cv.Optional(CONF_ON_END): automation.validate_automation({
        cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(MqttCountDownTrigger),
    }),
    cv.Optional(CONF_ON_START): automation.validate_automation({
        cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(MqttCountDownTrigger),
    }),
})


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add_library('CountUpDownTimer', None)
    yield cg.register_component(var, config)
    yield text_sensor.register_text_sensor(var, config)

    parent = yield cg.get_variable(config[CONF_MQTT_PARENT_ID])
    cg.add(var.set_parent(parent))
    cg.add(var.set_topic(config[CONF_TOPIC]))

    if CONF_QOS in config:
        cg.add(var.set_qos(config[CONF_QOS]))

    for conf in config.get(CONF_ON_END, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID])
        yield cg.register_component(trigger, conf)
        yield cg.register_parented(trigger, var)
        cg.add(trigger.set_testState(True))
        yield automation.build_automation(trigger, [], conf)

    for conf in config.get(CONF_ON_START, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID])
        yield cg.register_component(trigger, conf)
        yield cg.register_parented(trigger, var)
        cg.add(trigger.set_testState(False))
        yield automation.build_automation(trigger, [], conf)        
