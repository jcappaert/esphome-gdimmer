# esphome-gdimmer

ESPHome external component that turns a proximity sensor into a wave-to-toggle / hold-to-dim control for a light.

## Features
- Wave to toggle, hold to start dimming (auto direction flip optional).
- Heartbeat blink while dimming (optional LED output).
- Cooldown, hysteresis thresholds, step size, min/max brightness, blink period are configurable.
- Grace window for slower hand approaches (tunable).

## Installation

### As a git source
```yaml
external_components:
  - source:
      type: git
      url: https://github.com/jcappaert/esphome-gdimmer
    components: [gdimmer]
```

### As a local source
```yaml
external_components:
  - source:
      type: local
      path: /path/to/esphome-gdimmer/components/
    components: [gdimmer]
```

## YAML usage
You need a proximity sensor (any ESPHome sensor id that publishes a float), a light entity to control, and optionally an LED output for feedback.

```yaml
sensor:
  - platform: vcnl4040        # any proximity sensor; example only
    proximity:
      id: prox

light:
  - platform: monochromatic
    id: bar_light
    output: led_pwm

output:
  - platform: gpio
    id: prox_led
    pin: GPIO1

gdimmer:
  sensor: prox          # required: proximity sensor id
  light: bar_light      # required: light id
  led: prox_led         # optional LED for pulse/heartbeat
  # Optional tuning (defaults shown)
  toggle_high: 180
  toggle_low: 80
  cooldown_ms: 600
  hold_time_ms: 500
  step_interval_ms: 60
  dim_step: 0.03
  b_min: 0.05
  b_max: 1.0
  blink_period_ms: 300
  alternate_direction: true
  rise_grace_ms: 900
```

## Behavior
- Rising through hysteresis arms a gesture (`p >= toggle_high` after being below `toggle_low` or within `rise_grace_ms` of being below it).
- If the signal drops back to `<= toggle_low` before `hold_time_ms`, it toggles the light.
- If it stays high for `hold_time_ms`, dimming starts; brightness steps every `step_interval_ms` by `dim_step`, reversing at `b_min`/`b_max`. `alternate_direction` flips the starting direction each time dimming begins.
- Dimming stops when the signal falls below `toggle_low` and enters cooldown for `cooldown_ms`.
- LED (if provided) pulses on wave and heartbeats while dimming.

## Tuning tips
- Slow approach not detected: increase `rise_grace_ms`, reduce proximity smoothing latency, or widen hysteresis (`toggle_low` lower).
- Too sensitive / false toggles: raise `toggle_high`, increase `cooldown_ms`, or reduce `rise_grace_ms`.
- Dimming too fast/slow: adjust `dim_step` and/or `step_interval_ms`.
- Brightness bounds: set `b_min`/`b_max` to your desired floor/ceiling.

## Logging
Enable `logger` at `DEBUG` (and `VERBOSE` for per-step dim logs) to see gesture detection, cooldown, toggles, and dimming direction/steps.
