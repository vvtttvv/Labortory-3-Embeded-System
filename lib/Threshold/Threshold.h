#ifndef THRESHOLD_H
#define THRESHOLD_H

#include <stdint.h>

typedef enum {
    THRESHOLD_NORMAL = 0,
    THRESHOLD_ALERT  = 1
} ThresholdState;

typedef struct {
    float          threshHigh;     // Transition NORMAL -> ALERT above this
    float          threshLow;      // Transition ALERT -> NORMAL below this
    uint8_t        debounceLimit;  // Consecutive readings needed to confirm
    uint8_t        counter;        // Current debounce counter
    ThresholdState state;          // Current state
} ThresholdMonitor;

void           threshold_init(ThresholdMonitor *mon, float high, float low, uint8_t debounce);
void           threshold_update(ThresholdMonitor *mon, float value);
ThresholdState threshold_get_state(const ThresholdMonitor *mon);
uint8_t        threshold_get_counter(const ThresholdMonitor *mon);

#endif
