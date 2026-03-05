/*
 * ================================================================
 *  Project:      GY-521 (MPU-6050) Driver for RP2040
 *  File:         gy521.h
 *  Author:       (Gnibor) Robin Gerhartz
 *  License:      MIT License
 *  Repository:   https://github.com/Gnibor/gy521_rp2040
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
#ifndef GY521_I2C_PORT
#define GY521_I2C_PORT i2c1 // Default I2C port
#endif

#ifndef GY521_SDA_PIN
#define GY521_SDA_PIN 6    // Default SDA pin (can be overridden)
#endif

#ifndef GY521_SCL_PIN
#define GY521_SCL_PIN 7   // Default SCL pin (can be overridden)
#endif

#ifndef GY521_USE_PULLUP
#define GY521_USE_PULLUP 1 // 1 = enable internal pull-up, 0 = disabled
#endif

#ifndef GY521_INT_PIN
#define GY521_INT_PIN 26  // Optional interrupt pin (0 and the interrupt parts are not loaded)
#endif

#define GY521_I2C_ADDR_GND 0x68 // Default I2C address for GY-521(MPU-6050) (AD0 pin -> Gnd)
#define GY521_I2C_ADDR_VCC 0x69 // Default I2C address for GY-521(MPU-6050) (AD0 pin -> Vcc)


/*
 * Identifiers for selecting specific sensor blocks
 * Used in gy521_read()
 *
 * 0 = read all
 * 1 = accel only
 * 2 = temperature only
 * 3 = gyro only
 */
//#define GY521_ALL 0
//#define GY521_ACCEL 1
//#define GY521_TEMP 2
//#define GY521_GYRO 3

typedef enum{
	GY521_ACCEL = (1 << 0),
	GY521_TEMP = (1 << 1),
	GY521_GYRO = (1 << 2),
	GY521_ALL = (GY521_ACCEL | GY521_TEMP | GY521_GYRO),
	GY521_SCALED = (1 << 3)
} gy521_sensors_t;

// =======================
// === Data Structures ===
// =======================

/*
 * Raw axis values directly from registers
 * Signed 16-bit values from sensor
 */
typedef struct{
	int16_t x,y,z;
} gy521_axis_raw_t;

/*
 * Axis offset as calculated from
 * the calibrate function
 */
typedef struct{
	int32_t x, y, z;
} gy521_offset_t;

/*
 * Scaled axis values
 * - Accel: g-force
 * - Gyro: degrees per second
 */
typedef struct{
	float x,y,z;
} gy521_axis_scaled_t;

/*
 * per Axis bool for options
 */
typedef struct{
	bool x, y, z;
} gy521_axis_bool_t;

/*
 * Main device structure
 *
 * This struct contains:
 * - Measured values
 * - Configuration state
 * - Function pointers (pseudo OOP style)
 */
typedef struct gy521_s{
	// =====================
	// === Sensor Values ===
	// =====================
	struct{
		struct{
			gy521_axis_raw_t raw; // Raw accelerometer values
			gy521_axis_scaled_t g; // Converted acceleration in G
		} accel;

		struct{
			gy521_axis_raw_t raw; // Raw gyro values
			gy521_axis_scaled_t dps; // Converted gyro in °/s
		} gyro;

		struct{
			int16_t raw; // Raw temperature values
			float celsius; // Converted temperature in °C
		} temp;

		struct{
			bool data_rdy, i2c_mst, fifo_owflow;
		} interrupt;
	} v;

	// =====================
	// === Configuration ===
	// =====================
	struct{
		i2c_inst_t *i2c_port;
		uint8_t addr; // Device Address
		uint8_t *cache;
		uint8_t clksel;
		
		struct{
			uint8_t fsr;
			float fsr_divider;
		} accel;

		struct{
			uint8_t fsr;
			float fsr_divider;
			gy521_offset_t offset;
		} gyro;
	} conf;

	// =========================
	// === Function Pointers ===
	// =========================
	struct{
		bool (*who_am_i)(void);
		bool (*reset)(void);
		bool (*sleep)(bool device, bool temp);
		bool (*read_sensor)(gy521_sensors_t);
		bool (*fsr)(gy521_fsr_t, gy521_afsr_t);
		bool (*stby)(void);
		bool (*smplrt_div)(uint8_t div);

		struct{
			bool (*calibrate)(uint8_t);
		} gyro;

		struct{
			bool (*pin_cfg)(uint8_t cfg);
			bool (*enable)(uint8_t cfg);
			bool (*status)(void);
		} interrupt;
	} fn;
} gy521_s;

// ============================
// === Function declaration ===
// ============================
/*
 * gy521_init(addr);
 * Initializes the I²C connection and default configuration.
 * Returns a fully initialized gy521_s struct with function pointers and default values.
 */
gy521_s gy521_init(i2c_inst_t *i2c_port, uint8_t addr);
bool gy521_use(gy521_s *device);
bool gy521_write_register(uint8_t *data, uint8_t how_many, bool block);
bool gy521_read_reg(uint8_t reg, uint8_t *out, uint8_t how_many);
