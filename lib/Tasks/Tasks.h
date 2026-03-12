#ifndef TASKS_H
#define TASKS_H

#include <Arduino_FreeRTOS.h>

namespace Tasks
{
    void initHardware();
    bool createAll();

    // FreeRTOS task functions
    void acquisitionTask(void *pvParameters);   // Sensor reading
    void thresholdTask(void *pvParameters);      // Hysteresis + debounce
    void displayTask(void *pvParameters);        // Serial reporting
}

#endif
