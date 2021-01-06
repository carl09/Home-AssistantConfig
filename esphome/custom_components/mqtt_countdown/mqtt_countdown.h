#pragma once

#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/mqtt/mqtt_client.h"

namespace esphome {
namespace mqtt_countdown {

class MqttCountDown : public text_sensor::TextSensor, public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_parent(mqtt::MQTTClientComponent *parent) { parent_ = parent; }
  void set_topic(const std::string &topic) { topic_ = topic; }
  float get_setup_priority() const override;
  void set_qos(uint8_t qos) { qos_ = qos; };

  bool hasCompleated{true};

 protected:
  void run(std::string payload);

  mqtt::MQTTClientComponent *parent_;
  std::string topic_;
  uint8_t qos_{};
};

class MqttCountDownTrigger : public Trigger<>, public PollingComponent, public Parented<MqttCountDown> {
 public:
  MqttCountDownTrigger() : PollingComponent(1000) {}

  void setup() override { lastState_ = this->parent_->hasCompleated; }

  void set_testState(bool testState) { testState_ = testState; }

  void update() override {
    bool current = this->parent_->hasCompleated;

    if (lastState_ != current) {
      if (current == testState_) {
        this->trigger();
      }
      lastState_ = current;
    }
  }

 protected:
  bool lastState_;
  bool testState_;
};

}  // namespace mqtt_countdown
}  // namespace esphome