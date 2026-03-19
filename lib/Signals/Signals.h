#ifndef SIGNALS_H
#define SIGNALS_H

#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include "Threshold.h"

struct SensorData {
    // Analog sensor (NTC thermistor)
    uint16_t analogRaw;
    float    analogTempRaw;
    bool     analogValid;

    // Digital sensor (DHT11)
    float    digitalTemp;
    float    digitalHumidity;
    bool     digitalValid;
};

struct ConditioningData {
    float raw;
    float saturated;
    float median;
    float weighted;
    bool  valid;
};

struct AlertData {
    ThresholdState analogState;
    uint8_t        analogCounter;
    ThresholdState digitalState;
    uint8_t        digitalCounter;
};

// Mutex protecting shared data
extern SemaphoreHandle_t dataMutex;

// Shared sensor readings and alert states
extern SensorData sensorData;
extern ConditioningData conditioningData;
extern AlertData  alertData;

void Signals_init();
bool Signals_isReady();

#endif
