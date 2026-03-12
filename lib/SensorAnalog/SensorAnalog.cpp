#include "SensorAnalog.h"
#include "Config.h"
#include <Arduino.h>
#include <math.h>

static uint8_t _pin;

void SensorAnalog::init(uint8_t pin)
{
    _pin = pin;
    pinMode(pin, INPUT);
}

uint16_t SensorAnalog::readRaw()
{
    return analogRead(_pin);
}

float SensorAnalog::rawToCelsius(uint16_t raw)
{
    // Prevent division by zero at ADC boundaries
    if (raw == 0 || raw >= 1023) return -999.0f;

    // Voltage divider: Vcc --[R_series]-- ADC --[NTC]-- GND   R_therm = R_series * ADC / (1023 - ADC)
    float resistance = THERM_R_SERIES * (float)raw / (1023.0f - (float)raw);

    // Steinhart-Hart B-parameter equation:   1/T = 1/T0 + (1/B) * ln(R / R0)
    float steinhart = log(resistance / THERM_R_NOMINAL) / THERM_B_COEFF;
    steinhart += 1.0f / (THERM_T_NOMINAL + 273.15f);
    float tempC = (1.0f / steinhart) - 273.15f;

    return tempC;
}
