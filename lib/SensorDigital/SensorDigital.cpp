#include "SensorDigital.h"
#include <SimpleDHT.h>

static SimpleDHT11 *_dht = NULL;

void SensorDigital::init(uint8_t pin)
{
    static SimpleDHT11 instance(pin);
    _dht = &instance;
}

bool SensorDigital::read(float *temp, float *humidity)
{
    if (_dht == NULL) return false;

    byte t = 0, h = 0;
    int err = _dht->read(&t, &h, NULL);

    if (err != SimpleDHTErrSuccess) {
        return false;
    }

    *temp     = (float)t;
    *humidity = (float)h;
    return true;
}
