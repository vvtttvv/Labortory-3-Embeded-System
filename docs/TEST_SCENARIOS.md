# Lab 6 Test Scenarios and Validation Matrix

## Requirement -> Test -> Validation

| ID | Requirement | Test Scenario | Expected Result | Evidence |
|----|-------------|---------------|-----------------|----------|
| R1 | Periodic acquisition 20-100 ms | Set acquisition period to 50 ms and log timestamps | Sample interval remains around 50 ms | Serial log snapshots |
| R2 | Raw sensor interface | Inspect output containing ADC raw values | Raw values are reported each display cycle | `[THERM] ADC:` line |
| R3 | Hysteresis threshold | Apply temperature ramp around HIGH/LOW | No rapid toggling near threshold band | Alert state transitions |
| R4 | Debounce persistence | Inject short spikes above HIGH | No alert if spikes shorter than debounce window | Counter output + stable state |
| R5 | Display/reporting at slower period | Verify 500 ms report interval | Structured output every about 500 ms | Timestamped serial output |
| R6 | Saturation stage | Force out-of-range values in simulation | Saturated value stays inside configured bounds | `Sat:` field |
| R7 | Median filter stage | Inject salt-and-pepper spike | Median output rejects isolated spike | `Med:` field trend |
| R8 | Weighted average stage | Apply step signal | `WMA` smooths response vs raw/median | `Raw/Med/WMA` comparison |
| R9 | Task synchronization | Stress with concurrent updates | No corrupted/inconsistent report fields | Stable serial output |
| R10 | Alert reaction latency < 100 ms | Apply threshold crossing and measure time to LED state change | Delay under 100 ms | Scope/video/time logs |

## Practical Test Procedure

1. Start serial monitor at 9600.
2. Record at least 30 seconds of output for baseline.
3. Heat/cool sensor slowly around threshold limits.
4. Create short disturbances (touch/jump noise) and observe debounce behavior.
5. Capture evidence for each requirement (log snippet, screenshot, measured timing).

## Pass Criteria

- All requirements R1-R10 must have evidence.
- No oscillation around threshold band.
- No missed task activity under nominal load.
- Latency target under 100 ms is measured and documented.
