#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
//  Pin Configuration
// ============================================================
#define PIN_THERMISTOR    A0    // NTC thermistor analog input
#define PIN_DHT11         7    // DHT11 digital data pin
#define PIN_LED_ALERT     13    // External LED for alert (with 220 Ohm resistor)

// ============================================================
//  NTC Thermistor Parameters (10 kOhm NTC, B=3950)
// ============================================================
#define THERM_R_NOMINAL   10000.0f   // Resistance at T_NOMINAL (Ohm)
#define THERM_T_NOMINAL   25.0f      // Temperature for R_NOMINAL (C)
#define THERM_B_COEFF     3950.0f    // Beta coefficient
#define THERM_R_SERIES    10000.0f   // Series resistor in voltage divider (Ohm)

// ============================================================
//  Threshold / Hysteresis Configuration
// ============================================================
#define THRESH_HIGH       26.0f  // Alert activates above this (C)
#define THRESH_LOW        24.0f  // Alert deactivates below this (C)
#define DEBOUNCE_COUNT    5      // Consecutive readings to confirm state change

// ============================================================
//  Timing Configuration (ms)
// ============================================================
#define ACQUISITION_PERIOD_MS   50    // Sensor read rate (20-100 ms)
#define THRESHOLD_PERIOD_MS     50    // Threshold check rate
#define DISPLAY_PERIOD_MS       500   // Serial report rate
#define DHT_READ_INTERVAL_MS    2000  // DHT11 min sampling interval

// ============================================================
//  Serial Configuration
// ============================================================
#define SERIAL_BAUD_RATE  9600

#endif
