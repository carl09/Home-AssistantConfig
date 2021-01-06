#include "mqtt_countdown.h"
#include "esphome/core/log.h"
#include <CountUpDownTimer.h>

namespace esphome {
namespace mqtt_countdown {

static const char *TAG = "mqtt_countdown";

CountUpDownTimer T(DOWN, false);

void MqttCountDown::dump_config() {
  ESP_LOGCONFIG(TAG, "MQTT CountDown:");
  ESP_LOGCONFIG(TAG, "  Topic: %s", this->topic_.c_str());
}

void MqttCountDown::setup() {
  this->parent_->subscribe(
      this->topic_, [this](const std::string &topic, std::string payload) { this->run(payload); }, this->qos_);

  this->publish_state("");
}

void MqttCountDown::loop() {
  T.Timer();

  if (T.TimeHasChanged()) {
    char data[5];
    snprintf(data, sizeof(data), "%ld:%ld", T.ShowMinutes(), T.ShowSeconds());
    this->publish_state(data);
  }

  if (!T.isTimerRunning() && !hasCompleated) {
    hasCompleated = true;
    ESP_LOGI(TAG, "Timmer Stopped");
    this->publish_state("");
  }
}

void MqttCountDown::run(std::string payload) {
  ESP_LOGI(TAG, "subscribe payload: %s", payload.c_str());

  long mins = atol(payload.c_str());

  hasCompleated = false;

  T.ResetTimer();
  T.SetTimer(0, mins, 0);
  T.StartTimer();
}

float MqttCountDown::get_setup_priority() const { return setup_priority::AFTER_CONNECTION; }

}  // namespace mqtt_countdown
}  // namespace esphome