#pragma once
#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/light/light_state.h"
#include "esphome/components/output/binary_output.h"

namespace esphome {
namespace gdimmer {

class GestureDimmer : public Component {
 public:
  void set_sensor(sensor::Sensor *s) { sensor_ = s; }
  void set_light(light::LightState *l) { light_ = l; }
  void set_led(output::BinaryOutput *o) { led_ = o; }

  void set_thresholds(float toggle_high, float toggle_low) {
    toggle_high_ = toggle_high; toggle_low_ = toggle_low;
  }
  void set_timing(uint32_t cooldown_ms, uint32_t hold_time_ms, uint32_t step_interval_ms, uint32_t blink_period_ms) {
    cooldown_ms_ = cooldown_ms; hold_time_ms_ = hold_time_ms;
    step_interval_ms_ = step_interval_ms; blink_period_ms_ = blink_period_ms;
  }
  void set_dimming_params(float dim_step, float b_min, float b_max, bool alternate_direction) {
    dim_step_ = dim_step; b_min_ = b_min; b_max_ = b_max; alternate_direction_ = alternate_direction;
  }
  void set_rise_grace_ms(uint32_t rise_grace_ms) { rise_grace_ms_ = rise_grace_ms; }

  void setup() override;
  void loop() override;
  void dump_config() override;

 protected:
  void on_prox_(float p);
  void pulse_led_();
  void set_led_(bool on);

  float read_brightness_() const;

  sensor::Sensor *sensor_{nullptr};
  light::LightState *light_{nullptr};
  output::BinaryOutput *led_{nullptr};

  // config
  float toggle_high_{180.0f};
  float toggle_low_{80.0f};
  uint32_t cooldown_ms_{600};
  uint32_t hold_time_ms_{500};
  uint32_t step_interval_ms_{60};
  uint32_t blink_period_ms_{300};
  float dim_step_{0.03f};
  float b_min_{0.05f};
  float b_max_{1.0f};
  bool alternate_direction_{true};

  // state
  float last_p_{0.0f};
  uint32_t last_event_ms_{0};
  bool tap_pending_{false};
  uint32_t tap_start_ms_{0};

  bool dimming_{false};
  bool dim_up_{true};
  uint32_t last_step_ms_{0};
  uint32_t last_blink_ms_{0};

  // led pulse (non-blocking)
  bool led_pulsing_{false};
  uint32_t led_pulse_off_ms_{0};

  // telemetry
  bool was_in_cooldown_{false};

  // gesture detection aid for slower approaches
  uint32_t last_below_low_ms_{0};
  uint32_t rise_grace_ms_{900};  // allow up to ~0.9s between being below low and crossing high
};

}  // namespace gesture_dimmer
}  // namespace esphome
