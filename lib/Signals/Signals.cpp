#include "Signals.h"

SemaphoreHandle_t dataMutex = NULL;

SensorData sensorData = { 0, 0.0f, 0.0f, 0.0f, false };
AlertData  alertData  = { THRESHOLD_NORMAL, 0, THRESHOLD_NORMAL, 0 };

void Signals_init()
{
    dataMutex = xSemaphoreCreateMutex();
}

bool Signals_isReady()
{
    return (dataMutex != NULL);
}
