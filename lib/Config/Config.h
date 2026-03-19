#ifndef CONFIG_H
#define CONFIG_H

#define PIN_THERMISTOR    A0
#define PIN_DHT11         7
#define PIN_LED_ALERT     13

#define THERM_R_NOMINAL   10000.0f
#define THERM_T_NOMINAL   25.0f
#define THERM_B_COEFF     3950.0f
#define THERM_R_SERIES    10000.0f

#define THRESH_HIGH       26.0f  // Alert activates above this (C)
#define THRESH_LOW        24.0f  // Alert deactivates below this (C)
#define DEBOUNCE_COUNT    2      // Consecutive readings to confirm state change (<100 ms target)

#define COND_SAT_MIN_C    -20.0f
#define COND_SAT_MAX_C    80.0f
#define COND_WINDOW_SIZE  5
#define COND_WEIGHT_0     1
#define COND_WEIGHT_1     2
#define COND_WEIGHT_2     3
#define COND_WEIGHT_3     4
#define COND_WEIGHT_4     5

#define ACQUISITION_PERIOD_MS   20    // Sensor read rate (20-100 ms)
#define CONDITIONING_PERIOD_MS  20    // Signal conditioning rate
#define THRESHOLD_PERIOD_MS     20    // Threshold check rate
#define DISPLAY_PERIOD_MS       500   // Serial report rate
#define DHT_READ_INTERVAL_MS    2000  // DHT11 min sampling interval

#define SERIAL_BAUD_RATE  9600

#endif
