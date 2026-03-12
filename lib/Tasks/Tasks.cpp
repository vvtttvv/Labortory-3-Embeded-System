#include "Tasks.h"
#include "Config.h"
#include "Signals.h"
#include "SensorAnalog.h"
#include "SensorDigital.h"
#include "Threshold.h"
#include "UartStdio.h"
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <stdlib.h>

// Threshold monitors (local to threshold task)
static ThresholdMonitor analogMon;
static ThresholdMonitor digitalMon;

void Tasks::initHardware()
{
    SensorAnalog::init(PIN_THERMISTOR);
    SensorDigital::init(PIN_DHT11);
    pinMode(PIN_LED_ALERT, OUTPUT);
    digitalWrite(PIN_LED_ALERT, LOW);
}

bool Tasks::createAll()
{
    BaseType_t r1 = xTaskCreate(acquisitionTask,
                                "Acq", 160, NULL, 3, NULL);

    BaseType_t r2 = xTaskCreate(thresholdTask,
                                "Thresh", 128, NULL, 2, NULL);

    BaseType_t r3 = xTaskCreate(displayTask,
                                "Disp", 200, NULL, 1, NULL);

    return (r1 == pdPASS && r2 == pdPASS && r3 == pdPASS);
}

// ============================================================
//  Task 1: Sensor Acquisition (period: 50 ms)
//  Reads NTC thermistor (every cycle) and DHT11 (every 2 s)
// ============================================================
void Tasks::acquisitionTask(void *pvParameters)
{
    (void)pvParameters;
    printf_P(PSTR("[ACQ] Acquisition task started\n"));

    TickType_t xLastWake = xTaskGetTickCount();
    uint16_t dhtCounter  = 0;
    const uint16_t dhtInterval = DHT_READ_INTERVAL_MS / ACQUISITION_PERIOD_MS;

    for (;;)
    {
        // --- Analog sensor (NTC thermistor) - every cycle ---
        uint16_t raw  = SensorAnalog::readRaw();
        float    tempC = SensorAnalog::rawToCelsius(raw);

        // --- Digital sensor (DHT11) - every ~2 s ---
        float dhtTemp = 0.0f, dhtHum = 0.0f;
        bool  dhtOk   = false;

        if (dhtCounter >= dhtInterval) {
            dhtOk = SensorDigital::read(&dhtTemp, &dhtHum);
            dhtCounter = 0;
        }
        dhtCounter++;

        // --- Update shared data (mutex-protected) ---
        if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            sensorData.analogRaw  = raw;
            sensorData.analogTemp = tempC;

            if (dhtOk) {
                sensorData.digitalTemp     = dhtTemp;
                sensorData.digitalHumidity = dhtHum;
                sensorData.digitalValid    = true;
            }

            xSemaphoreGive(dataMutex);
        }

        vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(ACQUISITION_PERIOD_MS));
    }
}

// ============================================================
//  Task 2: Threshold Alerting (period: 50 ms)
//  Applies hysteresis + counter-based debounce to both sensors
// ============================================================
void Tasks::thresholdTask(void *pvParameters)
{
    (void)pvParameters;
    printf_P(PSTR("[THR] Threshold task started\n"));

    threshold_init(&analogMon,  THRESH_HIGH, THRESH_LOW, DEBOUNCE_COUNT);
    threshold_init(&digitalMon, THRESH_HIGH, THRESH_LOW, DEBOUNCE_COUNT);

    TickType_t xLastWake = xTaskGetTickCount();

    for (;;)
    {
        float aTemp  = 0.0f;
        float dTemp  = 0.0f;
        bool  dValid = false;

        // Read latest sensor values (mutex-protected)
        if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            aTemp  = sensorData.analogTemp;
            dTemp  = sensorData.digitalTemp;
            dValid = sensorData.digitalValid;
            xSemaphoreGive(dataMutex);
        }

        // Run threshold state machines
        threshold_update(&analogMon, aTemp);
        if (dValid) {
            threshold_update(&digitalMon, dTemp);
        }

        // Write alert results (mutex-protected)
        if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            alertData.analogState    = threshold_get_state(&analogMon);
            alertData.analogCounter  = threshold_get_counter(&analogMon);
            alertData.digitalState   = threshold_get_state(&digitalMon);
            alertData.digitalCounter = threshold_get_counter(&digitalMon);
            xSemaphoreGive(dataMutex);
        }

        // LED: ON if either sensor is in ALERT state
        bool anyAlert = (analogMon.state == THRESHOLD_ALERT ||
                         digitalMon.state == THRESHOLD_ALERT);
        digitalWrite(PIN_LED_ALERT, anyAlert ? HIGH : LOW);

        vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(THRESHOLD_PERIOD_MS));
    }
}

// ============================================================
//  Task 3: Display & Reporting (period: 500 ms)
//  Prints structured report with sensor values and alerts
// ============================================================
void Tasks::displayTask(void *pvParameters)
{
    (void)pvParameters;
    printf_P(PSTR("[DSP] Display task started\n"));

    TickType_t xLastWake = xTaskGetTickCount();

    for (;;)
    {
        vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(DISPLAY_PERIOD_MS));

        // Snapshot shared data (mutex-protected)
        SensorData sd;
        AlertData  ad;

        if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50)) == pdTRUE)
        {
            sd = sensorData;
            ad = alertData;
            xSemaphoreGive(dataMutex);
        }
        else {
            printf_P(PSTR("[DSP] Mutex timeout\n"));
            continue;
        }

        // Format thermistor temperature (1 decimal place)
        char tempBuf[8];
        dtostrf(sd.analogTemp, 5, 1, tempBuf);

        // Print structured report
        printf_P(PSTR("======= SENSOR REPORT =======\n"));

        printf_P(PSTR("[THERM] Raw:%4u | Temp:%s C | %S (%u/%u)\n"),
                 sd.analogRaw, tempBuf,
                 ad.analogState == THRESHOLD_ALERT
                     ? PSTR("ALERT!") : PSTR("NORMAL"),
                 ad.analogCounter, (uint8_t)DEBOUNCE_COUNT);

        if (sd.digitalValid)
        {
            printf_P(PSTR("[DHT11] Temp:%3d C | Hum:%3d%% | %S (%u/%u)\n"),
                     (int)sd.digitalTemp, (int)sd.digitalHumidity,
                     ad.digitalState == THRESHOLD_ALERT
                         ? PSTR("ALERT!") : PSTR("NORMAL"),
                     ad.digitalCounter, (uint8_t)DEBOUNCE_COUNT);
        }
        else {
            printf_P(PSTR("[DHT11] Waiting for valid data...\n"));
        }

        printf_P(PSTR("[CONF] HIGH=26.0 LOW=24.0 C | Dbnc:%u\n"),
                 (uint8_t)DEBOUNCE_COUNT);
        printf_P(PSTR("=============================\n"));
    }
}
