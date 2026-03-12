#ifndef SENSOR_ANALOG_H
#define SENSOR_ANALOG_H

#include <stdint.h>

namespace SensorAnalog
{
    void     init(uint8_t pin);
    uint16_t readRaw();
    float    rawToCelsius(uint16_t raw);
}

#endif
