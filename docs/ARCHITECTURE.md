# Lab 6 Architecture

## 1. Component List and Roles

- MCU + FreeRTOS scheduler: runs periodic tasks and enforces timing.
- `SensorAnalog`: acquires ADC value and converts NTC raw signal to Celsius.
- `SensorDigital`: acquires DHT11 temperature/humidity.
- `Conditioning`: analog preprocessing pipeline (saturation, median filter, weighted average).
- `Threshold`: hysteresis + debounce finite-state logic for alerting.
- `Signals`: shared inter-task data and mutex protection.
- `Tasks`: application behavior (Acquisition, Conditioning, Threshold, Display).
- `UartStdio`: `printf` over serial.

## 2. Structural Diagram

```mermaid
graph LR
  A[Acquisition Task] -->|raw analog + digital| B[Shared Data (Signals)]
  C[Conditioning Task] -->|sat/median/wma| B
  D[Threshold Task] -->|alert states| B
  E[Display Task] -->|reads snapshot| B

  A --> F[SensorAnalog]
  A --> G[SensorDigital]
  C --> H[Conditioning Module]
  D --> I[Threshold Module]
  E --> J[UART STDIO]
```

## 3. Layered HW/SW View

```mermaid
graph TD
  L4[Application Layer: Tasks] --> L3[Service Layer: Threshold + Conditioning + Signals]
  L3 --> L2[ECAL Drivers: SensorAnalog, SensorDigital, UartStdio]
  L2 --> L1[HAL/MCAL: Arduino Core + FreeRTOS + AVR Peripherals]
  L1 --> L0[Hardware: ATmega328P, NTC on A0, DHT11 on D7, LED D13]
```

## 4. Behavior Flow (Main Data Path)

```mermaid
flowchart TD
  S[Periodic acquisition tick] --> R1[Read analog ADC]
  R1 --> R2[Convert raw ADC to temperature]
  R2 --> R3[Read digital sensor when interval elapsed]
  R3 --> M1[Store snapshot under mutex]

  M1 --> C1[Conditioning tick]
  C1 --> C2[Saturate]
  C2 --> C3[Median filter]
  C3 --> C4[Weighted average]
  C4 --> M2[Store conditioned values under mutex]

  M2 --> T1[Threshold tick]
  T1 --> T2[Hysteresis + debounce update]
  T2 --> T3[Update alert states + LED]

  T3 --> D1[Display tick]
  D1 --> D2[Read full snapshot under mutex]
  D2 --> D3[Print structured report to STDIO]
```

## 5. Alert State Machine

```mermaid
stateDiagram-v2
  [*] --> NORMAL

  NORMAL --> NORMAL: value <= HIGH or debounce not reached
  NORMAL --> ALERT: value > HIGH for N consecutive samples

  ALERT --> ALERT: value >= LOW or debounce not reached
  ALERT --> NORMAL: value < LOW for N consecutive samples
```
