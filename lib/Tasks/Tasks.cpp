#include "Tasks.h"
#include "Config.h"
#include "Signals.h"
#include "SensorAnalog.h"
#include "SensorDigital.h"
#include "Conditioning.h"
#include "Threshold.h"
#include "UartStdio.h"
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <stdlib.h>

// Threshold monitors
static ThresholdMonitor analogMon;
static ThresholdMonitor digitalMon;
static ConditioningState analogCondState;
static uint32_t analogRiseStartMs = 0;
static bool     analogRisePending = false;

static const uint16_t STACK_ACQ    = 220;
static const uint16_t STACK_COND   = 320;
static const uint16_t STACK_THRESH = 220;
static const uint16_t STACK_DISP   = 340;

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
                                "Acq", STACK_ACQ, NULL, 3, NULL);

    BaseType_t r2 = xTaskCreate(conditioningTask,
                                "Cond", STACK_COND, NULL, 2, NULL);

    BaseType_t r3 = xTaskCreate(thresholdTask,
                                "Thresh", STACK_THRESH, NULL, 2, NULL);

    BaseType_t r4 = xTaskCreate(displayTask,
                                "Disp", STACK_DISP, NULL, 1, NULL);

    return (r1 == pdPASS && r2 == pdPASS && r3 == pdPASS && r4 == pdPASS);
}

//  Task 1: Sensor Acquisition (period: 50 ms)
//  Reads NTC thermistor (every cycle) and DHT11 (every 2 s)
void Tasks::acquisitionTask(void *pvParameters)
{
    (void)pvParameters;
    printf_P(PSTR("[ACQ] Acquisition task started\n"));

    TickType_t xLastWake = xTaskGetTickCount();
    uint16_t dhtCounter  = 0;
    const uint16_t dhtInterval = DHT_READ_INTERVAL_MS / ACQUISITION_PERIOD_MS;

    for (;;)
    {
        // Analog sensor (NTC thermistor) - every cycle
        uint16_t raw   = SensorAnalog::readRaw();
        float    tempC = SensorAnalog::rawToCelsius(raw);
        bool     aOk   = (tempC > -200.0f);

        // Digital sensor (DHT11) - every ~2s
        float dhtTemp = 0.0f, dhtHum = 0.0f;
        bool  dhtOk   = false;

        if (dhtCounter >= dhtInterval) {
            dhtOk = SensorDigital::read(&dhtTemp, &dhtHum);
            dhtCounter = 0;
        }
        dhtCounter++;

        // Update shared data (mutex-protected)
        if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            sensorData.analogRaw  = raw;
            sensorData.analogTempRaw = tempC;
            sensorData.analogValid   = aOk;

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

//  Task 2: Signal Conditioning (period: 50 ms)
//  Pipeline: raw -> saturation -> median -> weighted average
void Tasks::conditioningTask(void *pvParameters)
{
    (void)pvParameters;
    printf_P(PSTR("[CON] Conditioning task started\n"));

    Conditioning::init(&analogCondState);
    TickType_t xLastWake = xTaskGetTickCount();

    for (;;)
    {
        float rawTemp = 0.0f;
        bool  rawValid = false;

        if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            rawTemp = sensorData.analogTempRaw;
            rawValid = sensorData.analogValid;
            xSemaphoreGive(dataMutex);
        }

        ConditioningSnapshot snap = { rawTemp, rawTemp, rawTemp, rawTemp, false };

        if (rawValid) {
            Conditioning::processSample(&analogCondState, rawTemp, &snap);
        }

        if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            conditioningData.raw = snap.raw;
            conditioningData.saturated = snap.saturated;
            conditioningData.median = snap.median;
            conditioningData.weighted = snap.weighted;
            conditioningData.valid = snap.valid;
            xSemaphoreGive(dataMutex);
        }

        vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(CONDITIONING_PERIOD_MS));
    }
}

//  Task 3: Threshold Alerting (period: 50 ms)
//  Applies hysteresis + counter-based debounce to both sensors
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
        bool  aValid = false;
        bool  dValid = false;

        // Read latest sensor values (mutex-protected)
        if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            aTemp  = conditioningData.weighted;
            aValid = conditioningData.valid;
            dTemp  = sensorData.digitalTemp;
            dValid = sensorData.digitalValid;
            xSemaphoreGive(dataMutex);
        }

        // Run threshold state machines
        if (aValid) {
            if (!analogRisePending && analogMon.state == THRESHOLD_NORMAL && aTemp > THRESH_HIGH) {
                analogRiseStartMs = millis();
                analogRisePending = true;
            }

            ThresholdState prev = analogMon.state;
            threshold_update(&analogMon, aTemp);

            if (prev == THRESHOLD_NORMAL && analogMon.state == THRESHOLD_ALERT && analogRisePending) {
                uint32_t dt = millis() - analogRiseStartMs;
                if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10)) == pdTRUE)
                {
                    alertData.analogLastRiseLatencyMs = (uint16_t)(dt > 0xFFFFUL ? 0xFFFFU : dt);
                    alertData.analogLatencyValid = true;
                    xSemaphoreGive(dataMutex);
                }
                analogRisePending = false;
            }

            if (analogMon.state == THRESHOLD_NORMAL && aTemp <= THRESH_HIGH) {
                analogRisePending = false;
            }
        }
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

//  Task 4: Display & Reporting (period: 500 ms)
//  Prints structured report with sensor values and alerts
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
        ConditioningData cd;
        AlertData  ad;

        if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50)) == pdTRUE)
        {
            sd = sensorData;
            cd = conditioningData;
            ad = alertData;
            xSemaphoreGive(dataMutex);
        }
        else {
            printf_P(PSTR("[DSP] Mutex timeout\n"));
            continue;
        }

        // Format thermistor intermediate pipeline values
        char rawBuf[8], satBuf[8], medBuf[8], wmaBuf[8];
        dtostrf(cd.raw, 5, 1, rawBuf);
        dtostrf(cd.saturated, 5, 1, satBuf);
        dtostrf(cd.median, 5, 1, medBuf);
        dtostrf(cd.weighted, 5, 1, wmaBuf);

        // Print structured report
        printf_P(PSTR("======= SENSOR REPORT =======\n"));

        printf_P(PSTR("[THERM] ADC:%4u | Raw:%s C | Sat:%s C | Med:%s C | WMA:%s C | Valid:%c\n"),
                 sd.analogRaw, rawBuf, satBuf, medBuf, wmaBuf,
                 cd.valid ? 'Y' : 'N');

        printf_P(PSTR("[A-THR] State:%S (%u/%u)\n"),
                 ad.analogState == THRESHOLD_ALERT
                     ? PSTR("ALERT!") : PSTR("NORMAL"),
                 ad.analogCounter, (uint8_t)DEBOUNCE_COUNT);

        if (ad.analogLatencyValid)
        {
            printf_P(PSTR("[A-LAT] NORMAL->ALERT latency: %u ms\n"),
                     ad.analogLastRiseLatencyMs);
        }
        else {
            printf_P(PSTR("[A-LAT] Waiting for first NORMAL->ALERT transition...\n"));
        }

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
