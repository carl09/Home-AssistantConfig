#include "countdown.h"
#include "esphome/core/log.h"
#include <CountUpDownTimer.h>

namespace esphome {
namespace countdown {

static const char *TAG = "countdown";

CountUpDownTimer T(DOWN);

void CountDown::dump_config() {
  ESP_LOGCONFIG(TAG, "CountDown:");
  LOG_UPDATE_INTERVAL(this);
}

void CountDown::setup() {
  T.SetTimer(0, 1, 0);
  T.StartTimer();
}

void CountDown::update() {
  T.Timer();

  if (T.TimeHasChanged()) {
    char data[100];

    snprintf(data, sizeof(data), "%ld:%ld", T.ShowMinutes(), T.ShowSeconds());

    ESP_LOGI(TAG, "Current: %s", data);

    this->publish_state(data);
  }
}

}  // namespace countdown
}  // namespace esphome