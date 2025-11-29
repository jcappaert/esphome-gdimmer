#include "gdimmer.h"
#include "esphome/core/log.h"

namespace esphome {
namespace gdimmer {

static const char *const TAG = "gdimmer";

void GestureDimmer::setup() {
  if (this->sensor_ != nullptr) {
    this->sensor_->add_on_state_callback([this](float p) { this->on_prox_(p); });
  }
}

void GestureDimmer::dump_config() {
  ESP_LOGCONFIG(TAG, "Gesture Dimmer:");
  ESP_LOGCONFIG(TAG, "  Thresholds: high=%.1f low=%.1f", toggle_high_, toggle_low_);
  ESP_LOGCONFIG(TAG, "  Timing: cooldown=%ums hold=%ums step=%ums blink=%ums",
                cooldown_ms_, hold_time_ms_, step_interval_ms_, blink_period_ms_);
  ESP_LOGCONFIG(TAG, "  Dimming: step=%.3f min=%.2f max=%.2f alt_dir=%s",
                dim_step_, b_min_, b_max_, alternate_direction_ ? "true" : "false");
  ESP_LOGCONFIG(TAG, "  Rise grace: %ums", rise_grace_ms_);
}

float GestureDimmer::read_brightness_() const {
  if (this->light_ == nullptr) return 0.6f;
  float b = this->light_->current_values.get_brightness();
  if (isnan(b) || b <= 0.0f) b = 0.6f;
  return b;
}

void GestureDimmer::set_led_(bool on) {
  if (this->led_ == nullptr) return;
  this->led_->set_state(on);
}

void GestureDimmer::pulse_led_() {
  if (this->led_ == nullptr) return;
  this->set_led_(true);
  this->led_pulsing_ = true;
  this->led_pulse_off_ms_ = millis() + 90;  // ~90ms flash
}

void GestureDimmer::loop() {
  const uint32_t now = millis();

  // handle LED pulse timeout
  if (this->led_pulsing_ && now >= this->led_pulse_off_ms_) {
    this->set_led_(false);
    this->led_pulsing_ = false;
  }

  if (!this->dimming_) return;

  // heartbeat blink while dimming
  if (this->led_ && (now - this->last_blink_ms_) >= this->blink_period_ms_) {
    this->pulse_led_();
    this->last_blink_ms_ = now;
  }

  // dimming step timing handled here; proximity stream just toggles state
  if ((now - this->last_step_ms_) >= this->step_interval_ms_) {
    float b = this->read_brightness_();
    const bool stepping_up = this->dim_up_;
    b += this->dim_up_ ? this->dim_step_ : -this->dim_step_;
    if (b >= this->b_max_) { b = this->b_max_; this->dim_up_ = false; }
    if (b <= this->b_min_) { b = this->b_min_; this->dim_up_ = true;  }

    if (this->light_) {
      auto call = this->light_->turn_on();
      call.set_brightness(b);
      call.set_transition_length(0);
      call.perform();
    }
    ESP_LOGV(TAG, "Dimming step %s -> %.3f", stepping_up ? "up" : "down", b);
    this->last_step_ms_ = now;
  }
}

void GestureDimmer::on_prox_(float p) {
  const uint32_t now = millis();
  const uint32_t elapsed_since_event = now - this->last_event_ms_;
  const bool cooldown_ok = elapsed_since_event > this->cooldown_ms_;
  if (cooldown_ok && this->was_in_cooldown_) {
    ESP_LOGD(TAG, "Cooldown over; gestures enabled");
    this->was_in_cooldown_ = false;
  } else if (!cooldown_ok) {
    this->was_in_cooldown_ = true;
  }

  // track most recent time we were clearly below the low threshold
  if (p <= this->toggle_low_) {
    this->last_below_low_ms_ = now;
  }

  // If we are dimming, check for release (drop below low threshold)
  if (this->dimming_) {
    if (p <= this->toggle_low_) {
      this->dimming_ = false;
      this->last_event_ms_ = now;
    }
    this->last_p_ = p;
    return;
  }

  // Allow a slower approach: crossing high is valid if we were below low recently
  const bool recent_below_low = (now - this->last_below_low_ms_) <= this->rise_grace_ms_;
  const bool rising = (p >= this->toggle_high_) && (recent_below_low || this->last_p_ <= this->toggle_low_);

  // Not dimming: detect potential tap (rising through hysteresis window)
  if (!this->tap_pending_ && cooldown_ok) {
    if (rising) {
      this->tap_pending_ = true;
      this->tap_start_ms_ = now;
      ESP_LOGD(TAG, "Gesture rise detected (p=%.1f); waiting for release/hold%s",
               p, recent_below_low ? "" : " (via grace window)");
    }
  } else if (!this->tap_pending_ && !cooldown_ok && rising) {
    const uint32_t remaining = (this->cooldown_ms_ > elapsed_since_event) ? (this->cooldown_ms_ - elapsed_since_event) : 0;
    ESP_LOGD(TAG, "Gesture ignored during cooldown (%ums remaining)", remaining);
  }

  if (this->tap_pending_) {
    if (p <= this->toggle_low_) {
      // Quick release -> TOGGLE

      this->pulse_led_();

      if (this->light_) {
        const bool is_on = this->light_->current_values.is_on();
        ESP_LOGD(TAG, "Wave detected -> toggle light %s", is_on ? "off" : "on");
        if (is_on) {
          this->light_->turn_off().perform();
        } else {
          this->light_->turn_on().perform();
        }
      }

      this->tap_pending_ = false;
      this->last_event_ms_ = now;
    } else if ((now - this->tap_start_ms_) >= this->hold_time_ms_) {
      // Hold -> enter DIMMING
      this->dimming_ = true;
      this->last_step_ms_ = now;
      this->last_blink_ms_ = now;
      this->pulse_led_();
      if (this->alternate_direction_) this->dim_up_ = !this->dim_up_;
      ESP_LOGD(TAG, "Hold detected -> start dimming (%s)", this->dim_up_ ? "up" : "down");
      this->tap_pending_ = false;
    }
  }

  this->last_p_ = p;
}

}  // namespace gesture_dimmer
}  // namespace esphome
