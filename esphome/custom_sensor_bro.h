#include "esphome.h"
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_MitsubishiHeavy.h>

class SensorBroClimate : public Climate, public Component
{

public:
    SensorBroClimate(uint16_t kIrLed)
    {
        this->_kIrLed = kIrLed;
    }

    void setup() override
    {

        this->sensor_->add_on_state_callback([this](float state) {
            this->current_temperature = state;
            // current temperature changed, publish state
            this->publish_state();
        });

        this->current_temperature = this->sensor_->state;

        ESP_LOGI("custom", "Setup Called");

        this->publish_state();

        // restore set points
        auto restore = this->restore_state_();

        if (restore.has_value())
        {
            restore->apply(this);
        }
        else
        {
            // restore from defaults
            this->mode = climate::CLIMATE_MODE_OFF;
            this->target_temperature = 24;
        }
    }

    void control(const ClimateCall &call) override
    {
        bool transmit = false;

        ESP_LOGI("custom", "Calling Control");
        if (call.get_mode().has_value())
        {
            ESP_LOGI("custom", "Calling Control Mode");
            ClimateMode mode = *call.get_mode();
            this->mode = mode;

            transmit = true;
        }

        if (call.get_target_temperature().has_value())
        {
            ESP_LOGI("custom", "Calling Control Get Temp");
            // User requested target temperature change
            float temp = *call.get_target_temperature();
            this->target_temperature = temp;
            transmit = true;
        }

        if (call.get_fan_mode().has_value())
        {
            ESP_LOGI("custom", "Calling Control Fan Mode");

            if (call.get_fan_mode() == climate::CLIMATE_FAN_ON)
            {
                this->fan_mode = climate::CLIMATE_FAN_AUTO;
            }
            else
            {
                this->fan_mode = *call.get_fan_mode();
            }

            transmit = true;
        }

        if (call.get_swing_mode().has_value())
        {
            this->swing_mode = *call.get_swing_mode();
            transmit = true;
        }

        if (transmit)
        {
            this->transmit_state_();
        }
        this->publish_state();
    }

    climate::ClimateTraits traits() override
    {
        auto traits = climate::ClimateTraits();
        traits.set_supports_current_temperature(true);
        traits.set_supports_auto_mode(true);
        traits.set_supports_cool_mode(true);
        traits.set_supports_heat_mode(true);
        traits.set_supports_two_point_target_temperature(false);
        traits.set_supports_away(false);
        traits.set_visual_min_temperature(18);
        traits.set_visual_max_temperature(30);
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

    void set_sensor(Sensor *sensor) { this->sensor_ = sensor; }
    void set_ir_pin(uint16_t *pin) { this->ir_pin_ = pin; }

private:
    uint8_t bits[5];
    Sensor *sensor_{nullptr};
    uint16_t *ir_pin_;
    uint16_t _kIrLed;

    void transmit_state_()
    {
        uint8_t remote_mode;
        uint8_t remote_fan;
        bool remote_power = true;

        IRMitsubishiHeavy152Ac ac(_kIrLed);
        ac.begin();

        switch (this->mode)
        {
        case climate::CLIMATE_MODE_COOL:
            remote_mode = kMitsubishiHeavyCool;
            ac.setMode(remote_mode);
            break;
        case climate::CLIMATE_MODE_HEAT:
            remote_mode = kMitsubishiHeavyHeat;
            ac.setMode(remote_mode);
            break;
        case climate::CLIMATE_MODE_AUTO:
            remote_mode = kMitsubishiHeavyAuto;
            ac.setMode(remote_mode);
            break;
        case climate::CLIMATE_MODE_FAN_ONLY:
            remote_mode = kMitsubishiHeavyFan;
            ac.setMode(remote_mode);
            break;
        case climate::CLIMATE_MODE_DRY:
            remote_mode = kMitsubishiHeavyDry;
            ac.setMode(remote_mode);
            break;
        case climate::CLIMATE_MODE_OFF:
        default:
            remote_power = false;
            break;
        }

        switch (this->fan_mode)
        {
        case climate::CLIMATE_FAN_AUTO:
            remote_fan = kMitsubishiHeavy152FanAuto;
            ac.setFan(remote_fan);
            break;
        case climate::CLIMATE_FAN_LOW:
            remote_fan = kMitsubishiHeavy152FanLow;
            ac.setFan(remote_fan);
            break;
        case climate::CLIMATE_FAN_MEDIUM:
            remote_fan = kMitsubishiHeavy152FanMed;
            ac.setFan(remote_fan);
            break;
        case climate::CLIMATE_FAN_HIGH:
            remote_fan = kMitsubishiHeavy152FanHigh;
            ac.setFan(remote_fan);
            break;
        case climate::CLIMATE_FAN_MIDDLE:
        case climate::CLIMATE_FAN_ON:
        case climate::CLIMATE_FAN_OFF:
        case climate::CLIMATE_FAN_FOCUS:
        case climate::CLIMATE_FAN_DIFFUSE:
        default:
            ESP_LOGI("custom", "No Fan Mode");
            // remote_power = false;
            break;
        }

        const uint8_t swingVOff = kMitsubishiHeavy152SwingVMiddle;  // kMitsubishiHeavy152SwingVOff; // kMitsubishiHeavy88SwingVOff
        const uint8_t swingHOff = kMitsubishiHeavy152SwingHMiddle;  // kMitsubishiHeavy152SwingHOff; // kMitsubishiHeavy88SwingVOff

        switch (this->swing_mode)
        {
        case climate::CLIMATE_SWING_OFF:
            ac.setSwingVertical(swingVOff);
            ac.setSwingHorizontal(swingHOff);
            break;
        case climate::CLIMATE_SWING_BOTH:
            ac.setSwingVertical(kMitsubishiHeavy152SwingVAuto);
            ac.setSwingHorizontal(kMitsubishiHeavy152SwingHAuto);
            break;
        case climate::CLIMATE_SWING_VERTICAL:
            ac.setSwingVertical(kMitsubishiHeavy152SwingVAuto);
            ac.setSwingHorizontal(swingHOff);
            break;
        case climate::CLIMATE_SWING_HORIZONTAL:
            ac.setSwingVertical(swingVOff);
            ac.setSwingHorizontal(kMitsubishiHeavy152SwingHAuto);
            break;
        }

        ac.setTemp(this->target_temperature);

        ac.setPower(remote_power);

        ac.send();

        auto remote_state = ac.toString();

        ESP_LOGI("custom", "IR State: '%s'", remote_state.c_str());
    }
};