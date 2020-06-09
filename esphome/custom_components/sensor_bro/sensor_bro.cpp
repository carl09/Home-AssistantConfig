#include "sensor_bro.h"
#include "esphome/core/log.h"

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_MitsubishiHeavy.h>

namespace esphome {
namespace sensor_bro {

static const char *TAG = "sensor_bro.climate";

const uint8_t DEFAULT_TEMP_MIN = 16;         // Celsius
const uint8_t DEFAULT_TEMP_MAX = 31;         // Celsius
const uint8_t DEFAULT_TARGET_TEMP_MAX = 22;  // Celsius

void SensorBroClimate::setup() {
  if (this->sensor_) {
    this->sensor_->add_on_state_callback([this](float state) {
      this->current_temperature = state;
      this->publish_state();
    });
    this->current_temperature = this->sensor_->state;
  } else {
    this->current_temperature = NAN;
  }

  // restore set points
  auto restore = this->restore_state_();

  if (restore.has_value()) {
    restore->apply(this);
  } else {
    // restore from defaults
    this->mode = climate::CLIMATE_MODE_OFF;
    this->target_temperature = DEFAULT_TARGET_TEMP_MAX;
    this->fan_mode = climate::CLIMATE_FAN_AUTO;
    this->swing_mode = climate::CLIMATE_SWING_OFF;
  }

  if (isnan(this->target_temperature))
    this->target_temperature = DEFAULT_TARGET_TEMP_MAX;

  this->ac_ = new IRMitsubishiHeavy152Ac(this->pin_->get_pin());
  this->ac_->begin();

  // Syncing Remote settings
  this->setMode(false);
  this->setFanMode(false);
  this->setSwingMode(false);
  this->setTargetTemperature(false);
}

climate::ClimateTraits SensorBroClimate::traits() {
  auto traits = climate::ClimateTraits();
  traits.set_supports_current_temperature(this->sensor_ != nullptr);
  traits.set_supports_auto_mode(true);
  traits.set_supports_cool_mode(true);
  traits.set_supports_heat_mode(true);
  traits.set_supports_two_point_target_temperature(false);
  traits.set_supports_away(false);
  traits.set_visual_min_temperature(DEFAULT_TEMP_MIN);
  traits.set_visual_max_temperature(DEFAULT_TEMP_MAX);
  traits.set_visual_temperature_step(1);

  traits.set_supports_fan_only_mode(true);
  traits.set_supports_dry_mode(true);

  traits.set_supports_fan_mode_on(false);
  traits.set_supports_fan_mode_off(false);
  traits.set_supports_fan_mode_auto(true);
  traits.set_supports_fan_mode_low(true);
  traits.set_supports_fan_mode_medium(true);
  traits.set_supports_fan_mode_high(true);
  traits.set_supports_fan_mode_middle(false);
  traits.set_supports_fan_mode_focus(false);
  traits.set_supports_fan_mode_diffuse(false);

  traits.set_supports_swing_mode_off(true);
  traits.set_supports_swing_mode_both(true);
  traits.set_supports_swing_mode_vertical(true);
  traits.set_supports_swing_mode_horizontal(true);

  return traits;
}

void SensorBroClimate::control(const climate::ClimateCall &call) {
  if (call.get_mode().has_value()) {
    ESP_LOGI(TAG, "Calling Control Mode");
    this->mode = *call.get_mode();
    this->setMode(true);
  }

  if (call.get_target_temperature().has_value()) {
    ESP_LOGI(TAG, "Calling Control Set Target Temp");
    this->target_temperature = *call.get_target_temperature();
    this->setTargetTemperature(true);
  }

  if (call.get_fan_mode().has_value()) {
    ESP_LOGI(TAG, "Calling Control Fan Mode");
    this->fan_mode = *call.get_fan_mode();
    this->setFanMode(true);
  }

  if (call.get_swing_mode().has_value()) {
    ESP_LOGI(TAG, "Calling Control Swing Mode");
    this->swing_mode = *call.get_swing_mode();
    this->setSwingMode(true);
  }

  auto remote_state = this->ac_->toString();

  ESP_LOGI(TAG, "IR State: '%s'", remote_state.c_str());

  this->publish_state();
}

void SensorBroClimate::dump_config() {
  ESP_LOGCONFIG(TAG, "SensorBro:");
  LOG_CLIMATE("", "IR Climate", this);
  LOG_PIN("  Step Pin: ", this->pin_);

  ESP_LOGCONFIG(TAG, "  mode %u", this->mode);
  ESP_LOGCONFIG(TAG, "  target_temperature %f", this->target_temperature);
  ESP_LOGCONFIG(TAG, "  fan_mode %u", this->fan_mode);
  ESP_LOGCONFIG(TAG, "  swing_mode %u", this->swing_mode);
}

void SensorBroClimate::setMode(bool send) {
  if (this->mode == climate::CLIMATE_MODE_OFF) {
    this->ac_->setPower(false);
  } else {
    switch (this->mode) {
      case climate::CLIMATE_MODE_COOL:
        this->ac_->setMode(kMitsubishiHeavyCool);
        break;
      case climate::CLIMATE_MODE_HEAT:
        this->ac_->setMode(kMitsubishiHeavyHeat);
        break;
      case climate::CLIMATE_MODE_AUTO:
        this->ac_->setMode(kMitsubishiHeavyAuto);
        break;
      case climate::CLIMATE_MODE_FAN_ONLY:
        this->ac_->setMode(kMitsubishiHeavyFan);
        break;
      case climate::CLIMATE_MODE_DRY:
        this->ac_->setMode(kMitsubishiHeavyDry);
        break;
      case climate::CLIMATE_MODE_OFF:
      default:
        ESP_LOGW(TAG, "No Mode: %u", this->mode);
    }
    this->ac_->setPower(true);
  }
  if (send) {
    this->ac_->send();
  }
}

void SensorBroClimate::setTargetTemperature(bool send) {
  this->ac_->setTemp(this->target_temperature);
  if (send) {
    this->ac_->send();
  }
}

void SensorBroClimate::setFanMode(bool send) {
  //   if (call.get_fan_mode() == climate::CLIMATE_FAN_ON) {
  //     this->fan_mode = climate::CLIMATE_FAN_AUTO;
  //   } else {
  //     this->fan_mode = *call.get_fan_mode();
  //   }

  switch (this->fan_mode) {
    case climate::CLIMATE_FAN_AUTO:
      this->ac_->setFan(kMitsubishiHeavy152FanAuto);
      break;
    case climate::CLIMATE_FAN_LOW:
      this->ac_->setFan(kMitsubishiHeavy152FanLow);
      break;
    case climate::CLIMATE_FAN_MEDIUM:
      this->ac_->setFan(kMitsubishiHeavy152FanMed);
      break;
    case climate::CLIMATE_FAN_HIGH:
      this->ac_->setFan(kMitsubishiHeavy152FanHigh);
      break;
    case climate::CLIMATE_FAN_MIDDLE:
    case climate::CLIMATE_FAN_ON:
    case climate::CLIMATE_FAN_OFF:
    case climate::CLIMATE_FAN_FOCUS:
    case climate::CLIMATE_FAN_DIFFUSE:
    default:
      ESP_LOGW(TAG, "No Fan Mode: %u", this->fan_mode);
      break;
  }

  if (send) {
    this->ac_->send();
  }
}

void SensorBroClimate::setSwingMode(bool send) {
  const uint8_t swingVOff =
      kMitsubishiHeavy152SwingVMiddle;  // kMitsubishiHeavy152SwingVOff; // kMitsubishiHeavy88SwingVOff
  const uint8_t swingHOff =
      kMitsubishiHeavy152SwingHMiddle;  // kMitsubishiHeavy152SwingHOff; // kMitsubishiHeavy88SwingVOff

  switch (this->swing_mode) {
    case climate::CLIMATE_SWING_OFF:
      this->ac_->setSwingVertical(swingVOff);
      this->ac_->setSwingHorizontal(swingHOff);
      break;
    case climate::CLIMATE_SWING_BOTH:
      this->ac_->setSwingVertical(kMitsubishiHeavy152SwingVAuto);
      this->ac_->setSwingHorizontal(kMitsubishiHeavy152SwingHAuto);
      break;
    case climate::CLIMATE_SWING_VERTICAL:
      this->ac_->setSwingVertical(kMitsubishiHeavy152SwingVAuto);
      this->ac_->setSwingHorizontal(swingHOff);
      break;
    case climate::CLIMATE_SWING_HORIZONTAL:
      this->ac_->setSwingVertical(swingVOff);
      this->ac_->setSwingHorizontal(kMitsubishiHeavy152SwingHAuto);
      break;
  }

  if (send) {
    this->ac_->send();
  }
}

}  // namespace sensor_bro
}  // namespace esphome