#include <Arduino.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include "UartStdio.h"
#include "Signals.h"
#include "Tasks.h"
#include "Config.h"

void setup()
{
    UartStdio::init(SERIAL_BAUD_RATE);

    printf_P(PSTR("[INIT] Lab5 - Sensor Monitoring System\n"));
    printf_P(PSTR("[INIT] Sensors: NTC Thermistor (A0) + DHT11 (D7)\n"));

    Tasks::initHardware();
    Signals_init();

    if (!Signals_isReady())
    {
        printf_P(PSTR("[ERR] Sync objects failed\n"));
        for (;;) { ; }
    }

    printf_P(PSTR("[INIT] Creating tasks...\n"));

    if (!Tasks::createAll())
    {
        printf_P(PSTR("[ERR] Task creation failed\n"));
        for (;;) { ; }
    }

    printf_P(PSTR("[INIT] System ready, scheduler starting\n"));
}

void loop() { }