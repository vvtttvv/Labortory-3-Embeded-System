# Lab 5: Sensor Monitoring System with FreeRTOS

This project implements a multi-tasking sensor monitoring system on an Arduino Uno using FreeRTOS. It demonstrates real-time data acquisition from analog and digital sensors, robust signal processing (hysteresis & debouncing), and task synchronization.

## Features

- **Real-Time Multitasking**: Uses FreeRTOS to manage concurrent tasks for data acquisition, processing, and reporting.
- **Hybrid Sensor Interface**:
  - **Analog**: NTC Thermistor (10k) on `A0` for rapid temperature changes.
  - **Digital**: DHT11 on `D7` for ambient temperature and humidity.
- **Advanced Threshold Logic**:
  - Implements a generic **Hysteresis** mechanism (High/Low thresholds) to prevent oscillation.
  - **Debouncing**: Requiring $N$ consecutive samples to confirm a state change, filtering out noise.
- **Thread-Safe Architecture**: Uses **Mutexes** to protect shared sensor data between tasks.
- **Modular Codebase**: Organized into clear libraries for ease of maintenance and reuse.

## Hardware Setup

| Component | Pin | Description |
|-----------|-----|-------------|
| **NTC Thermistor** | `A0` | 10k NTC + 10k Series Resistor (Voltage Divider) |
| **DHT11 Sensor** | `D7` | Digital Temperature & Humidity Sensor |
| **Alert LED** | `D13` | Built-in LED, indicates Temperature Alert (> 26°C) |
| **Serial Monitor** | USB | 9600 Baud for debug output |

## Software Architecture

The system is divided into three main FreeRTOS tasks:

1.  **Acquisition Task (`Acq`)** - *Period: 50 ms*
    - Reads the NTC thermistor raw value every cycle.
    - Polls the DHT11 sensor every 2 seconds (due to sensor slowness).
    - Converts raw analog readings to Celsius using the Beta equation.
    - Updates a shared data structure protected by a Mutex.

2.  **Threshold Task (`Thresh`)** - *Period: 50 ms*
    - Reads the latest sensor data.
    - Feeds data into a generic **Threshold Monitor** logic.
    - Logic:
      - **ALERT** if value > `THRESH_HIGH` (26°C) for `DEBOUNCE_COUNT` (5) cycles.
      - **NORMAL** if value < `THRESH_LOW` (24°C) for `DEBOUNCE_COUNT` (5) cycles.
    - Controls the LED based on the determined state.

3.  **Display Task (`Disp`)** - *Period: 500 ms*
    - Formats and prints current readings, threshold states, and counters to the Serial Monitor.

## Project Structure

```
Lab5/
├── lib/
│   ├── Config/        # System constants (Pins, Thresholds, Intervals)
│   ├── SensorAnalog/  # NTC Thermistor driver (Steinhart-Hart / Beta)
│   ├── SensorDigital/ # DHT11 driver wrapper
│   ├── Signals/       # FreeRTOS handles (Mutexes, Semaphores)
│   ├── Tasks/         # Task implementation (Acq, Thresh, Disp)
│   ├── Threshold/     # Generic hysteresis & debounce logic unit
│   └── UartStdio/     # printf support for Serial
├── src/
│   └── main.cpp       # System initialization & scheduler start
└── platformio.ini     # Build configuration
```

## Getting Started

1.  ** Prerequisites**:
    - VS Code with PlatformIO extension.
    - Arduino Uno (or compatible AVR board).

2.  **Build & Upload**:
    - Open the project in PlatformIO.
    - Connect your board via USB.
    - Run the **Upload** task.

3.  **Monitor**:
    - Open the Serial Monitor (speed: **9600**).
    - You should see initialization logs followed by real-time sensor data.

## Configuration

You can adjust system parameters in `lib/Config/Config.h`:

- **Thresholds**: Change `THRESH_HIGH` and `THRESH_LOW`.
- **Debounce**: Adjust `DEBOUNCE_COUNT` to change sensitivity to noise.
- **Timing**: Modify `ACQUISITION_PERIOD_MS` or `DISPLAY_PERIOD_MS`.
