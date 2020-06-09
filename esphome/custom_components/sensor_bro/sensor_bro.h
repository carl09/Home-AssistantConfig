#pragma once

#include "esphome/components/climate/climate.h"
#include "esphome/components/sensor/sensor.h"

#include <ir_MitsubishiHeavy.h>

namespace esphome {
namespace sensor_bro {

class SensorBroClimate : public climate::Climate, public Component {
 public:
  void setup() override;

  void set_sensor(sensor::Sensor *sensor) { this->sensor_ = sensor; }

  void set_pin(GPIOPin *pin) { pin_ = pin; }

  void dump_config() override;

 protected:
  void control(const climate::ClimateCall &call) override;
  climate::ClimateTraits traits() override;

  sensor::Sensor *sensor_{nullptr};
  IRMitsubishiHeavy152Ac *ac_{nullptr};
  GPIOPin *pin_;

  void setMode(bool send);
  void setTargetTemperature(bool send);
  void setFanMode(bool send);
  void setSwingMode(bool send);
};

}  // namespace sensor_bro
}  // namespace esphome
