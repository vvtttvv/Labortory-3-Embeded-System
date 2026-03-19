#include <Arduino.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include "UartStdio.h"
#include "Signals.h"
#include "Tasks.h"
#include "Config.h"
#include <task.h>

void setup()
{
    UartStdio::init(SERIAL_BAUD_RATE);

    printf_P(PSTR("[INIT] Lab6 - Sensor Monitoring System\n"));
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

extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    printf_P(PSTR("[FATAL] Stack overflow in task: %s\n"), pcTaskName ? pcTaskName : "unknown");
    pinMode(PIN_LED_ALERT, OUTPUT);
    for (;;) {
        digitalWrite(PIN_LED_ALERT, HIGH);
        delay(120);
        digitalWrite(PIN_LED_ALERT, LOW);
        delay(120);
    }
}

extern "C" void vApplicationMallocFailedHook(void)
{
    printf_P(PSTR("[FATAL] Malloc failed\n"));
    pinMode(PIN_LED_ALERT, OUTPUT);
    for (;;) {
        digitalWrite(PIN_LED_ALERT, HIGH);
        delay(350);
        digitalWrite(PIN_LED_ALERT, LOW);
        delay(350);
    }
}