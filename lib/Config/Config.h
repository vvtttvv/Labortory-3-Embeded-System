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
#define DEBOUNCE_COUNT    5      // Consecutive readings to confirm state change

#define ACQUISITION_PERIOD_MS   50    // Sensor read rate (20-100 ms)
#define THRESHOLD_PERIOD_MS     50    // Threshold check rate
#define DISPLAY_PERIOD_MS       500   // Serial report rate
#define DHT_READ_INTERVAL_MS    2000  // DHT11 min sampling interval

#define SERIAL_BAUD_RATE  9600

#endif
