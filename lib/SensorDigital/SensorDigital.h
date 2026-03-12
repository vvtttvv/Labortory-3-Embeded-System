#ifndef SENSOR_DIGITAL_H
#define SENSOR_DIGITAL_H

#include <stdint.h>
#include <stdbool.h>

namespace SensorDigital
{
    void init(uint8_t pin);
    bool read(float *temp, float *humidity);
}

#endif
