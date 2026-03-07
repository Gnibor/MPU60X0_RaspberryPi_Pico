# MPU-6000/MPU-6050 Driver for Raspberry Pi Pico

Lightweight C driver for the **MPU-60X0** 6-axis IMU designed for the **Raspberry Pi Pico**. Provides clean register-level implementation with automatic scaling and structured device API.

---

## Project Note
This is my first project in almost a decade. My focus is on:

- Clean low-level implementation  
- Transparent register control  
- Minimal abstraction  
- Explicit configuration  
- Expandability toward full MPU-60X0 feature coverage  

---

## Requirements
- Raspberry Pi pico-sdk  
- Raspberry Pi Pico toolchain  
- CMake-based build environment  

Dependencies: pico/stdlib, hardware/i2c, string  

---

## Badges
![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)  ![Platform: RP2040](https://img.shields.io/badge/Platform-Rraspberry_Pi_Pico-blue)  ![Language: C](https://img.shields.io/badge/Language-C-informational)  ![Interface: I2C](https://img.shields.io/badge/Interface-I²C-orange)  

---

## Features (Current Implementation)
- I²C communication (400 kHz)  
- WHO_AM_I device verification  
- Device reset and wake-up  
- Clock source selection  
- DLPF Configuration  
- Accelerometer & Gyroscope Full-Scale-Range configuration  
- Standby control per axis
- Sleep mode all or temperatur
- Gyroscope zero-offset calibration  
- Raw + scaled sensor output: Acceleration in ***g***, Angular velocity in ***°/s***, Temperature in ***°C***  
- No dynamic memory allocation  
- Fully configurable via macros  

---

## Experimental

### Extended Power Management
- Cycle mode support  
- Low-power wake control  

### Interrupt Configuration & Handling
- INT_ENABLE register configuration  
- INT_STATUS decoding  
- Data-ready interrupt support  
- External GPIO interrupt integration  
- Interrupt-driven sampling  

---

## Development Plan

### I2C Slave (I2C_SLVx) Configuration
- External sensor passthrough  
- I2C_SLV0–I2C_SLV4 setup  
- Master mode configuration  

### FIFO Support
- FIFO_EN configuration  
- FIFO_COUNT handling  
- Burst read from FIFO  
- Continuous buffered sampling mode  

### Complete Register Coverage
- Structured access to all MPU-6000/MPU-6050 registers  
- Optional register debug dump function  

---

## Hardware
- Raspberry Pi Pico (RP2040/RP2350)  
- MPU-60X0 (GY-521)  
- I²C wiring (default): SDA → GPIO 6, SCL → GPIO 7  
- Interrupt pin (default): 26  

---

## Configuration
Hardware config can be adjusted in `mpu60x0.h` or in `main.c` before `#include "mpu60x0.h"`:

```c
#define MPU_I2C_PORT i2c1
#define MPU_SDA_PIN 6
#define MPU_SCL_PIN 7
#define MPU_USE_PULLUP 0
#include "mpu60x0.h"
```

---

## Basic Usage

```c
int main(void)
{
    stdio_usb_init();
    while (!stdio_usb_connected()) sleep_ms(100);

    mpu_s imu = mpu_init(MPU_I2C_PORT, MPU_I2C_ADDR_GND);
    mpu_use_struct(&imu);

    if (!mpu_who_am_i()) printf("Device not found!\n");

    mpu_sleep(MPU_SLEEP_ALL_OFF);

    mpu_fsr(MPU_FSR_500DPS, MPU_AFSR_2G);

    mpu_calibrate_gyro(10);

    while (1) {
        if(imu.fn.read(MPU_ALL | MPU_SCALED)){
            printf("Accel: %.2f %.2f %.2f g\n",
                   imu.v.accel.g.x,
                   imu.v.accel.g.y,
                   imu.v.accel.g.z);
        }
        sleep_ms(500);
    }
}
```

---

## API Overview
 
### Initialization

```c
mpu_s device = mpu_init(MPU_I2C_PORT, MPU_I2C_ADDR_GND);
mpu_use_struct(&device);
```

Initializes I²C and returns a fully configured device struct.
And set 'device' as active.

---

### Core Functions

| Function | Description |
|----------|------------|
| `mpu_s mpu_init(i2c_inst_t, mpu_addr_t)` | Initilize I²C connection and returns a device struct |
| `bool mpu_use_struct(mpu_s)` | Set the global device pointer for functions to `mpu_s` |
| `bool mpu_who_am_i(void)` | Verifies device via WHO_AM_I register |
| `bool mpu_device_reset(void)` | Performs a device reset |
| `bool mpu_sleep(mpu_sleep_s)` | Enables/disables sleep mode for the complete device or just the temperatur sensor|
| `bool mpu_fsr(mpu_fsr_s, mpu_afsr_s)` | Sets full-scale range and updates scaling divider |
| `bool mpu_stby(mpu_stby_t)` | Enables/disables standby per axis, sensor or all |
| `bool mpu_read_sensor(mpu_sensor_s)` | Reads sensor data (raw or scaled) |
| `bool mpu_calibrate_gyro(samples)` | Computes `samples` **(5-20)** time to calculate gyro zero-offset |
| `bool mpu_cycle_mode(mpu_cycle_t, smplrt_wake)` | Activate/deactivate **cycle** or **cycle low power** mode and set the mpu_smplrt_t |

#### Interrupt Functions (Experimental)

If you wanna use interrupts `#define MPU_INT_PIN` can not be `0`.

| Function | Description |
|----------|------------|
| `bool mpu_int_pin_cfg(mpu_int_pin_cfg_t)` | Configures the interrupt pin choosen with `mpu_int_pin_cfg_t` |
| `bool mpu_int_enable(mpu_int_enable_t)` | Enables interrupts choosen with `mpu_int_enable_t` |
| `bool mpu_int_status(void)` | Reads the INT_STATUS register and returns `true` if any is set |

#### Cycle Mode (Experimental)

if you wanna use the `mpu_cycle_mode(mpu_cycle_t mode, uint8_t smplrt_wake);` function the `#define MPU_USE_CYCLE` must be set to `1`.

---

## Scaling

Raw sensor values are automatically converted when `mpu_read_sensor(MPU_* | MPU_SCALED)` is given.

### Accelerometer

```
g = raw / fsr_divider
```

### Gyroscope

```
°/s = (raw - offset) / fsr_divider
```

### Temperature

```
°C = (raw / 340) + 36.53
```

---

## Design Philosophy

This driver:

- Avoids dynamic memory
- Avoids hidden global state (except calibration offset)
- Uses explicit configuration
- Provides register-level transparency
- Emulates object-oriented behavior using structured function pointers

The goal is clarity, control and minimal abstraction for embedded systems.

---

## License

MIT License

Copyright (c) 2026  
(Gnibor) Robin Gerhartz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.

---

## Repository

https://github.com/Gnibor/MPU60X0_RaspberryPi_Pico

---

## Status

Early but functional implementation.  
Actively developed toward full MPU-6050 feature support.

Contributions (especially with I²C_SLV and FIFO), suggestions and improvements are welcome.
