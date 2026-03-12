#include "Threshold.h"

void threshold_init(ThresholdMonitor *mon, float high, float low, uint8_t debounce)
{
    mon->threshHigh    = high;
    mon->threshLow     = low;
    mon->debounceLimit = debounce;
    mon->counter       = 0;
    mon->state         = THRESHOLD_NORMAL;
}

void threshold_update(ThresholdMonitor *mon, float value)
{
    switch (mon->state)
    {
    case THRESHOLD_NORMAL:
        // Check if value exceeds upper threshold
        if (value > mon->threshHigh) {
            mon->counter++;
            if (mon->counter >= mon->debounceLimit) {
                mon->state   = THRESHOLD_ALERT;
                mon->counter = 0;
            }
        } else {
            mon->counter = 0;  // Reset counter if condition not met
        }
        break;

    case THRESHOLD_ALERT:
        // Check if value drops below lower threshold
        if (value < mon->threshLow) {
            mon->counter++;
            if (mon->counter >= mon->debounceLimit) {
                mon->state   = THRESHOLD_NORMAL;
                mon->counter = 0;
            }
        } else {
            mon->counter = 0;
        }
        break;
    }
}

ThresholdState threshold_get_state(const ThresholdMonitor *mon)
{
    return mon->state;
}

uint8_t threshold_get_counter(const ThresholdMonitor *mon)
{
    return mon->counter;
}
