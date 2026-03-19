# Lab 6 Checklist (Parts 1 + 2)

Use this as a completion tracker for the full assignment.

## Current status after code review (Part 1)

- [x] Separate modules exist for acquisition, threshold/conditioning, reporting.
- [x] Separate FreeRTOS tasks exist: acquisition (50 ms), threshold (50 ms), display (500 ms).
- [x] Raw acquisition API exists for analog (`SensorAnalog::readRaw()`).
- [x] Hysteresis is implemented with configurable HIGH/LOW thresholds.
- [x] Debounce is implemented with configurable persistence count (`DEBOUNCE_COUNT`).
- [x] Shared data protection is implemented with mutex.
- [x] Structured serial reporting includes intermediate values and alert states.
- [x] Build succeeds for `mega2560` in PlatformIO.
- [ ] End-to-end latency (<100 ms) is not yet measured and documented.
- [ ] Formal test evidence matrix (requirement -> test -> result) is not yet documented.
- [ ] Required architecture/behavior diagrams and HW schematic are not yet attached.

## 1) Part 1 - Binary Threshold Acquisition

### Architecture and modularity
- [ ] Keep modules separated: acquisition, conditioning/threshold, reporting.
- [ ] Keep reusable sensor drivers in dedicated modules (analog + digital).
- [ ] Keep shared interfaces and data contracts in one place.
- [ ] Keep alert logic (hysteresis + debounce) isolated from task code.

### Functional requirements
- [ ] Read sensor data periodically (20-100 ms configurable).
- [ ] Expose raw sensor read API (`readRaw()` or equivalent).
- [ ] Implement threshold with hysteresis (`HIGH` / `LOW` thresholds).
- [ ] Implement anti-bounce / persistence confirmation before state switch.
- [ ] Make persistence interval configurable (time- or count-based).
- [ ] Report intermediate values (raw, processed, counters, state).
- [ ] Report current alert state through STDIO serial output.
- [ ] Ensure end-to-end reaction latency is < 100 ms for alert path.

### FreeRTOS and synchronization
- [ ] Use dedicated tasks (Acquisition, Threshold, Display).
- [ ] Define task periods and priorities explicitly.
- [ ] Protect shared data with mutex/semaphore.
- [ ] Avoid blocking sections that can cause missed deadlines.

### Validation for Part 1
- [ ] Test NORMAL -> ALERT transition above HIGH threshold.
- [ ] Test ALERT -> NORMAL transition below LOW threshold.
- [ ] Test values around threshold band (no oscillation/chatter).
- [ ] Inject noise and confirm debounce blocks false transitions.
- [ ] Validate periodic reporting cadence (about 500 ms).
- [ ] Validate behavior when digital sensor read fails temporarily.

## 2) Part 2 - Analog Signal Conditioning

### Conditioning pipeline
- [ ] Acquire analog raw sample at configurable period.
- [ ] Apply saturation/clamping to valid ADC/temperature range.
- [ ] Apply salt-and-pepper suppression (median filter).
- [ ] Apply weighted moving average after median stage.
- [ ] Keep each intermediate stage available for reporting.

### Suggested data path
- [ ] `raw -> saturated -> median -> weighted_average -> threshold(optional)`.
- [ ] Keep filter window sizes configurable constants.
- [ ] Keep weights configurable and normalized/documented.

### Validation for Part 2
- [ ] Test step response (fast value jump) and measure settling time.
- [ ] Test impulse noise (single-sample spikes) and verify rejection.
- [ ] Test sustained noise and compare pre/post filter variance.
- [ ] Test saturation at low and high bounds.
- [ ] Verify output update period and CPU load remain acceptable.

## 3) Optional Variant C (max score path)
- [ ] Monitor two sensors simultaneously (one analog, one digital).
- [ ] Display both sensor values and both alert states.
- [ ] Keep both processing paths independent and synchronized.

## 4) Documentation and deliverables (grading-critical)

### Engineering process artifacts
- [ ] Technical requirements list (functional + non-functional).
- [ ] Minimum one test per requirement with validation criterion.
- [ ] Component list with roles.
- [ ] Structural architecture diagram (components and interactions).
- [ ] Layered HW/SW interface architecture.
- [ ] Behavior diagrams (flow/state/sequence as needed).
- [ ] Electrical schematic (simulation environment).

### Test evidence and optimization
- [ ] Test logs/screenshots from serial monitor and/or plotter.
- [ ] Observations and detected issues.
- [ ] Optimizations applied after validation.
- [ ] Final conclusions and recommendations.

## 5) Practical "done" definition before submission
- [ ] Project builds and runs on target board/simulator.
- [ ] Serial output shows intermediate values + states + alerts.
- [ ] No race conditions observed under stress.
- [ ] Latency target (<100 ms) measured and documented.
- [ ] Report contains architecture, diagrams, tests, and results.
