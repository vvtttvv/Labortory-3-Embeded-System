#ifndef TASKS_H
#define TASKS_H

#include <Arduino_FreeRTOS.h>

namespace Tasks
{
    void initHardware();
    bool createAll();

    void acquisitionTask(void *pvParameters);
    void conditioningTask(void *pvParameters);  // Saturation + median + weighted average
    void thresholdTask(void *pvParameters);  
    void displayTask(void *pvParameters);      
}

#endif
