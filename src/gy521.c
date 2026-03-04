/*
 * ================================================================
 *  Project:      GY-521 (MPU-6050) Driver for RP2040
 *  File:         gy521.c
 *  Author:       (Gnibor) Robin Gerhartz
 *  License:      MIT License
 *  Repository:   https://github.com/Gnibor/gy521_rp2040
 * ================================================================
 *
 *  Description:
 *  Low-level driver implementation for the GY-521 module
 *  based on the MPU-6050 6-axis IMU sensor.
 *
 *  This file implements:
 *  - I²C communication
 *  - Register-level configuration
 *  - Sensor data acquisition
 *  - Automatic scaling (raw -> physical units)
 *  - Gyroscope zero-point calibration
 *  - Power management features
 *
 *  The driver is written in a lightweight embedded style
 *  and uses function pointers inside a device structure
 *  to emulate object-oriented behavior in C.
 *
 * ================================================================
 */
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "gy521_reg_map.h"
#include "gy521.h"

// ===========================
// === Function prototypes ===
// ===========================
bool gy521_who_am_i(void);
bool gy521_sleep(bool device, bool temp); // Set sleep configuration
bool gy521_fsr(uint8_t fsr, uint8_t afsr);
bool gy521_calibrate_gyro(uint8_t sample); // calibrate gyro offsets (sample=10)
bool gy521_read_sensor(uint8_t accel_temp_gyro, bool scaled); // 0=all 1=accel 2=temp 3=gyro
bool gy521_int_pin_cfg(void);
bool gy521_int_enable(void);
bool gy521_int_status(void);

// ========================
// === Global Variables ===
// ========================
static gy521_s *g_gy521 = NULL; // Global pointer to the aktiv GY521-Device
static uint8_t g_gy521_cache[14] = {0}; // Temporary buffer for I2C reads
static int g_gy521_ret_cache = 0; // Temporary buffer for return values

// =========================
// === Set device to use ===
// =========================
// to set device as used device for fn.*
bool gy521_use(gy521_s *device){
	if (device == NULL) return false; // Check if device is set

	g_gy521 = device;

	return true;
}

// ========================
// === Initialize GY521 ===
// ========================
gy521_s gy521_init(i2c_inst_t *i2c_port, uint8_t addr){
	i2c_init(GY521_I2C_PORT, 400 * 1000); // 400 kHz I2C
	gpio_set_function(GY521_SDA_PIN, GPIO_FUNC_I2C);
	gpio_set_function(GY521_SCL_PIN, GPIO_FUNC_I2C);

#if GY521_USE_PULLUP
	gpio_pull_up(GY521_SDA_PIN);
	gpio_pull_up(GY521_SCL_PIN);
#endif

	// Configure optional interrupt pin
	gpio_init(GY521_INT_PIN);
	gpio_set_dir(GY521_INT_PIN, GPIO_IN);

	gy521_s gy521; // Initalize device struct and function pointers
	memset(&gy521, 0, sizeof(gy521));

	gy521.conf.i2c_port = i2c_port;

	if(!addr) gy521.conf.addr = GY521_I2C_ADDR_GND;
	else gy521.conf.addr = addr;

	gy521.conf.accel.fsr_divider = 131.0f;
	gy521.conf.gyro.fsr_divider = 16384.0f;
	gy521.fn.sleep = &gy521_sleep;
	gy521.fn.test_connection = &gy521_who_am_i;
	gy521.fn.read_sensor = &gy521_read_sensor;
	gy521.fn.gyro.calibrate = &gy521_calibrate_gyro;
	gy521.fn.fsr = &gy521_fsr;

	return gy521;
}

// ==========================
// === I²C Register Write ===
// ==========================
bool gy521_write_register(uint8_t *data, uint8_t how_many){
	if(!g_gy521) return false;

	g_gy521_ret_cache = i2c_write_blocking(g_gy521->conf.i2c_port, g_gy521->conf.addr, data, how_many, false);
	if(g_gy521_ret_cache != how_many) return false;

	return true;
}

// =========================
// === I2C Register Read ===
// =========================
bool gy521_read_register(uint8_t reg, uint8_t *out, uint8_t how_many){
	if(!g_gy521) return false;

	uint8_t g_gy521_ret_cache = i2c_write_blocking(GY521_I2C_PORT, g_gy521->conf.addr, (uint8_t[]){reg}, 1, true);
	if(g_gy521_ret_cache != 1) return false;

	g_gy521_ret_cache = i2c_read_blocking(GY521_I2C_PORT, g_gy521->conf.addr, out, how_many, false);
	if(g_gy521_ret_cache != how_many) return false;

	return true;
}

// =======================
// === Test Connection ===
// =======================
bool gy521_who_am_i(void){
	uint8_t who_am_i;
	if(!gy521_read_register(GY521_REG_WHO_AM_I, &who_am_i, 1)) return false;
	return who_am_i == 0x68 ? true : false;
}

// ==================
// === Sleep Mode ===
// ==================
bool gy521_sleep(bool device, bool temp){
	if(!g_gy521) return false;
	if(!gy521_read_register(GY521_REG_PWR_MGMT_1, g_gy521_cache, 1)) return false;

	// Sleep Bit
	if(device) g_gy521_cache[0] |= GY521_SLEEP;
	else g_gy521_cache[0] &= ~GY521_SLEEP;

	// Temperature disable Bit
	if(temp) g_gy521_cache[0] |= GY521_TEMP_DIS;
	else g_gy521_cache[0] &= ~GY521_TEMP_DIS;

	g_gy521_ret_cache = i2c_write_blocking(GY521_I2C_PORT, g_gy521->conf.addr, (uint8_t[]){GY521_REG_PWR_MGMT_1, g_gy521_cache[0]}, 2, false);
	if(g_gy521_ret_cache != 2) return false;

	return true;
}

// ===================================
// ===  Set Full-Scale Range (FSR) ===
// === & Calculate Scaling Factors ===
// ===================================
bool gy521_fsr(uint8_t fsr, uint8_t afsr){
	if(!g_gy521) return false;
	// Read FSR Register
	if(!gy521_read_register(GY521_REG_GYRO_CONFIG, g_gy521_cache, 2)) return false;
	
	// Gyro FSR bits
	g_gy521_cache[0] &= ~0x18; // Delete bits 4:3
	g_gy521_cache[0] |= fsr; // Set FSR Bits

	// Automatic scaling calculation:
	// 131 / 2^bits → sensitivity in °/s
	g_gy521->conf.gyro.fsr_divider = 131.0f / (1 << ((fsr >> 3) & 0x03));

	// Accel FSR bits
	g_gy521_cache[1] &= ~0x18;
	g_gy521_cache[1] |= afsr;

	// Automatic scaling calculation (raw / divider = G)
	g_gy521->conf.accel.fsr_divider = 16384.0f / (1 << ((afsr >> 3) & 0x03));

	// Write back to registers
	 g_gy521_ret_cache = i2c_write_blocking(GY521_I2C_PORT, g_gy521->conf.addr, (uint8_t[]){GY521_REG_GYRO_CONFIG, g_gy521_cache[0], g_gy521_cache[1]}, 3, false);
	if(g_gy521_ret_cache != 3) return false;

	return true;
}

// ====================================================
// === Calibrate gyro (determine zero-point offset) ===
// ====================================================
bool gy521_calibrate_gyro(uint8_t samples){
	if(!g_gy521) return false;
	gy521_axis_raw_t raw;
	
	int64_t sum_gx = 0;
	int64_t sum_gy = 0;
	int64_t sum_gz = 0;

	for (uint8_t i = 0; i < samples; i++){
		if(!gy521_read_register(GY521_REG_GYRO_XOUT_H, g_gy521_cache, 6)) return false;

		raw.x = (g_gy521_cache[0]  << 8) | g_gy521_cache[1];
		raw.y = (g_gy521_cache[2]  << 8) | g_gy521_cache[3];
		raw.z = (g_gy521_cache[4]  << 8) | g_gy521_cache[5];

		sum_gx += raw.x;
		sum_gy += raw.y;
		sum_gz += raw.z;

		sleep_ms(5); // small delay between measurements
	}

	// Store averages as offsets
	g_gy521->conf.gyro.offset.x = sum_gx / samples;
	g_gy521->conf.gyro.offset.y = sum_gy / samples;
	g_gy521->conf.gyro.offset.z = sum_gz / samples;

	return true;
}

// ===========================================
// === Read Sensor Data + Optional Scaling ===
// ===========================================
bool gy521_read_sensor(uint8_t accel_temp_gyro, bool scaled){
	if(!g_gy521) return false;
	// Read all sensors
	if(accel_temp_gyro == 0){
		if(!gy521_read_register(GY521_REG_ACCEL_XOUT_H, g_gy521_cache, 14)) return false;

		g_gy521->v.accel.raw.x = (g_gy521_cache[0]  << 8) | g_gy521_cache[1];
		g_gy521->v.accel.raw.y = (g_gy521_cache[2]  << 8) | g_gy521_cache[3];
		g_gy521->v.accel.raw.z = (g_gy521_cache[4]  << 8) | g_gy521_cache[5];
		g_gy521->v.temp.raw = (g_gy521_cache[6]  << 8) | g_gy521_cache[7];
		g_gy521->v.gyro.raw.x = (g_gy521_cache[8]  << 8) | g_gy521_cache[9];
		g_gy521->v.gyro.raw.y = (g_gy521_cache[10] << 8) | g_gy521_cache[11];
		g_gy521->v.gyro.raw.z = (g_gy521_cache[12] << 8) | g_gy521_cache[13];

	// Only accelerometer
	}else if(accel_temp_gyro == 1){
		if(!gy521_read_register(GY521_REG_ACCEL_XOUT_H, g_gy521_cache, 6)) return false;

		g_gy521->v.accel.raw.x = (g_gy521_cache[0]  << 8) | g_gy521_cache[1];
		g_gy521->v.accel.raw.y = (g_gy521_cache[2]  << 8) | g_gy521_cache[3];
		g_gy521->v.accel.raw.z = (g_gy521_cache[4]  << 8) | g_gy521_cache[5];

	// Only temperatur
	}else if(accel_temp_gyro == 2){
		if(!gy521_read_register(GY521_REG_TEMP_OUT_H, g_gy521_cache, 2)) return false;

		g_gy521->v.temp.raw = (g_gy521_cache[0]  << 8) | g_gy521_cache[1];

	// Only gyroscope
	}else if(accel_temp_gyro == 3){
		if(!gy521_read_register(GY521_REG_GYRO_XOUT_H, g_gy521_cache, 6)) return false;

		g_gy521->v.gyro.raw.x = (g_gy521_cache[0]  << 8) | g_gy521_cache[1];
		g_gy521->v.gyro.raw.y = (g_gy521_cache[2] << 8) | g_gy521_cache[3];
		g_gy521->v.gyro.raw.z = (g_gy521_cache[4] << 8) | g_gy521_cache[5];
	}

	// Optional: scale raw values
	if(scaled){
		// Raw -> G for accelerometer
		if(accel_temp_gyro == 0 || accel_temp_gyro == 1){
			g_gy521->v.accel.g.x = g_gy521->v.accel.raw.x / g_gy521->conf.accel.fsr_divider;
			g_gy521->v.accel.g.y = g_gy521->v.accel.raw.y / g_gy521->conf.accel.fsr_divider;
			g_gy521->v.accel.g.z = g_gy521->v.accel.raw.z / g_gy521->conf.accel.fsr_divider;
		}

		// Raw -> °C
		if(accel_temp_gyro == 0 || accel_temp_gyro == 2)
			g_gy521->v.temp.celsius = (g_gy521->v.temp.raw / 340.0f) + 36.53f;

		// Raw -> °/s for gyroscope
		if(accel_temp_gyro == 0 || accel_temp_gyro == 3){
			g_gy521->v.gyro.dps.x = (g_gy521->v.gyro.raw.x - g_gy521->conf.gyro.offset.x) / g_gy521->conf.gyro.fsr_divider;
			g_gy521->v.gyro.dps.y = (g_gy521->v.gyro.raw.y - g_gy521->conf.gyro.offset.y) / g_gy521->conf.gyro.fsr_divider;
			g_gy521->v.gyro.dps.z = (g_gy521->v.gyro.raw.z - g_gy521->conf.gyro.offset.z) / g_gy521->conf.gyro.fsr_divider;
		}
	}

	return true;
}
