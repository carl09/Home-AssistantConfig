#pragma once

#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace countdown {

class CountDown : public PollingComponent, public text_sensor::TextSensor {
 public:
  CountDown() : PollingComponent(1000){};

  void setup() override;
  void update() override;
  void dump_config() override;
};
}  // namespace countdown
}  // namespace esphome