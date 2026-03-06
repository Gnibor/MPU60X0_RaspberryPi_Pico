/*
 * ================================================================
 *  Project:      GY-521 (MPU-6050) Driver for RP2040
 *  File:         mpu60x0.h
 *  Author:       (Gnibor) Robin Gerhartz
 *  License:      MIT License
 *  Repository:   https://github.com/Gnibor/mpu60x0_rp2040
 * ================================================================
 *
 *  MIT License
 *
 *  Copyright (c) 2026 (Gnibor) Robin Gerhartz
 *
 *  Permission is hereby granted, free of charge, to any person
 *  obtaining a copy of this software and associated documentation
 *  files (the "Software"), to deal in the Software without restriction,
 *  including without limitation the rights to use, copy, modify,
 *  merge, publish, distribute, sublicense, and/or sell copies of
 *  the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 * ================================================================
 *
 *  Public API for the GY-521 (MPU-6050) driver.
 *
 * ================================================================
 */
#pragma once
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdint.h>
#include <sys/types.h>
#include "MPU-60X0_reg_map.h"

// =============================
// === Configurable Hardware ===
// =============================
#ifndef MPU60X0_I2C_PORT
#define MPU60X0_I2C_PORT i2c1 // Default I2C port
#endif

#ifndef MPU60X0_SDA_PIN
#define MPU60X0_SDA_PIN 6    // Default SDA pin (can be overridden)
#endif

#ifndef MPU60X0_SCL_PIN
#define MPU60X0_SCL_PIN 7   // Default SCL pin (can be overridden)
#endif

#ifndef MPU60X0_USE_PULLUP
#define MPU60X0_USE_PULLUP 1 // 1 = enable internal pull-up, 0 = disabled
#endif

#ifndef MPU60X0_INT_PIN
#define MPU60X0_INT_PIN 26  // Optional interrupt pin (0 and the interrupt parts are not loaded)
#endif

#ifndef MPU60X0_INT_PULLUP
#define MPU60X0_INT_PULLUP 0
#endif

#define MPU60X0_I2C_ADDR_GND 0x68 // Default I2C address for GY-521(MPU-6050) (AD0 pin -> Gnd)
#define MPU60X0_I2C_ADDR_VCC 0x69 // Default I2C address for GY-521(MPU-6050) (AD0 pin -> Vcc)


/*
 * Identifiers for selecting specific sensor blocks
 * Used in mpu_read()
 */
typedef enum{
	MPU_ACCEL = (1 << 0),
	MPU_TEMP = (1 << 1),
	MPU_GYRO = (1 << 2),
	MPU_ALL = (MPU_ACCEL | MPU_TEMP | MPU_GYRO),
	MPU_SCALED = (1 << 3)
} mpu_sensors_t;

/*
 * Identifiers for selecting differrent function options
 * Used in mpu_dlpf_cfg()
 */
typedef enum{
	MPU_CYCLE_LP  = 2,
	MPU_CYCLE_ON  = 1,
	MPU_CYCLE_OFF = 0
} mpu_cycle_t;

/*
 * Used in mpu_sleep()
 */
typedef enum{
	MPU_SLEEP_DEVICE_ON	= (1 << 0),
	MPU_SLEEP_DEVICE_OFF	= (0 << 0),
	MPU_SLEEP_TEMP_ON	= (1 << 1),
	MPU_SLEEP_TEMP_OFF	= (0 << 1),
	MPU_SLEEP_ALL_OFF	= 0
} mpu_sleep_t;

// ========================
// === Global Variables ===
// ========================
extern volatile bool g_mpu_int_flag;

// =======================
// === Data Structures ===
// =======================
/*
 * Raw axis values directly from registers
 * Signed 16-bit values from sensor
 */
typedef struct{
	int16_t x,y,z;
} mpu_axis_raw_t;

/*
 * Scaled axis values
 * - Accel: g-force
 * - Gyro: degrees per second
 */
typedef struct{
	float x,y,z;
} mpu_axis_scaled_t;

/*
 * Axis offset as calculated from
 * the calibrate function
 */
typedef struct{
	int32_t x, y, z;
} mpu_offset_t;

/*
 * Main device structure
 *
 * This struct contains:
 * - Measured values
 * - Configuration state
 * - Function pointers (pseudo OOP style)
 */
typedef struct mpu_s{
	// =====================
	// === Sensor Values ===
	// =====================
	struct{
		struct{
			mpu_axis_raw_t raw; // Raw accelerometer values
			mpu_axis_scaled_t g; // Converted acceleration in G
		} accel;

		struct{
			mpu_axis_raw_t raw; // Raw gyro values
			mpu_axis_scaled_t dps; // Converted gyro in °/s
		} gyro;

		struct{
			int16_t raw; // Raw temperature values
			float celsius; // Converted temperature in °C
		} temp;
	} v;

	// =====================
	// === Configuration ===
	// =====================
	struct{
		i2c_inst_t *i2c_port;
		uint8_t addr; // Device Address
		uint8_t *cache;
		uint8_t clksel;
		mpu_offset_t gyro_offset, accel_offset;

		struct{
			float accel, gyro;
		} fsr_div;
	} conf;
} mpu_s;

// ============================
// === Function declaration ===
// ============================
/*
 * mpu_init(addr);
 * Initializes the I²C connection and default configuration.
 * Returns a fully initialized mpu_s struct with function pointers and default values.
 */
mpu_s mpu_init(i2c_inst_t *i2c_port, uint8_t addr);
bool mpu_use_struct(mpu_s *device);
bool mpu_write_register(uint8_t *data, uint8_t how_many, bool block);
bool mpu_read_register(uint8_t reg, uint8_t *out, uint8_t how_many, bool block);
bool mpu_dlpf_cfg(mpu60x0_dlpf_cfg_t cfg);
bool mpu_who_am_i(void);
bool mpu_device_reset(void);
bool mpu_sleep(mpu_sleep_t sleep); // Set sleep configuration
bool mpu_stby(uint8_t stby);
bool mpu_cycle_mode(mpu_cycle_t mode, uint8_t smplrt_wake);
bool mpu_fsr(mpu60x0_fsr_t fsr, mpu60x0_afsr_t afsr);
bool mpu_calibrate_gyro(uint8_t sample); // calibrate gyro offsets (sample=10)
bool mpu_read_sensor(mpu_sensors_t sensors); // 0=all 1=accel 2=temp 3=gyro
#if MPU60X0_INT_PIN
void mpu_irq_handler(uint gpio, uint32_t events);
bool mpu_int_pin_cfg(uint8_t cfg);
bool mpu_int_enable(uint8_t cfg);
bool mpu_int_status(void);
#endif
