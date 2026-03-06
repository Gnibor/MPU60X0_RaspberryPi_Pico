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
#include "MPU-60X0_reg_map.h"
#include "gy521.h"

// ===========================
// === Function prototypes ===
// ===========================
void gy521_irq_handler(uint gpio, uint32_t events);

// ========================
// === Global Variables ===
// ========================
static gy521_s *g_gy521 = NULL; // Global pointer to the aktiv GY521-Device
static uint8_t g_gy521_cache[14] = {0}; // Temporary buffer for I2C reads
static int g_gy521_ret_cache = 0; // Temporary buffer for return values

#if GY521_INT_PIN
volatile bool g_mpu_irq_flag = false;

void gy521_irq_handler(uint gpio, uint32_t events){
    if(gpio == GY521_INT_PIN){
        g_mpu_irq_flag = true;
    }
}
#endif

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
#if GY521_INT_PULLUP
	gpio_pull_up(GY521_INT_PIN);
#endif

	gy521_s gy521; // Initalize device struct and function pointers
	memset(&gy521, 0, sizeof(gy521));

	gy521.conf.i2c_port = i2c_port;

	if(!addr) gy521.conf.addr = GY521_I2C_ADDR_GND;
	else gy521.conf.addr = addr;

	gy521.conf.fsr_div.accel = 16384.0f;
	gy521.conf.fsr_div.gyro = 131.0f;
	gy521.fn.device_reset = &gy521_device_reset;
	gy521.fn.sleep = &gy521_sleep;
	gy521.fn.cycle = &gy521_cycle_mode;
	gy521.fn.who_am_i = &gy521_who_am_i;
	gy521.fn.read_sensor = &gy521_read_sensor;
	gy521.fn.gyro_calibrate = &gy521_calibrate_gyro;
	gy521.fn.fsr = &gy521_fsr;
	gy521.fn.stby = &gy521_stby;
#if GY521_INT_PIN
	gy521.fn.interrupt.pin_cfg = &gy521_int_pin_cfg;
	gy521.fn.interrupt.enable = &gy521_int_enable;
	gy521.fn.interrupt.status = &gy521_int_status;
#endif

	return gy521;
}

// ==========================
// === I²C Register Write ===
// ==========================
bool gy521_write_register(uint8_t *data, uint8_t how_many, bool block){
	if(!g_gy521) return false;

	g_gy521_ret_cache = i2c_write_blocking(g_gy521->conf.i2c_port, g_gy521->conf.addr, data, how_many, block);
	if(g_gy521_ret_cache != how_many) return false;

	return true;
}

// =========================
// === I2C Register Read ===
// =========================
bool gy521_read_register(uint8_t reg, uint8_t *out, uint8_t how_many, bool block){
	if(!g_gy521) return false;

	if(!gy521_write_register(&reg, 1, true)) return false;

	g_gy521_ret_cache = i2c_read_blocking(GY521_I2C_PORT, g_gy521->conf.addr, out, how_many, block);
	if(g_gy521_ret_cache != how_many) return false;

	return true;
}

// =======================
// === Test Connection ===
// =======================
bool gy521_who_am_i(void){
	if(!gy521_read_register(GY521_REG_WHO_AM_I, g_gy521_cache, 1, false)) return false;

	return g_gy521_cache[0] == GY521_WHO_AM_I ? true : false;
}

bool gy521_device_reset(void){
	if(!gy521_read_register(GY521_REG_PWR_MGMT_1, g_gy521_cache, 1, true)) return false;

	g_gy521_cache[0] |= GY521_DEVICE_RESET;

	if(!gy521_write_register((uint8_t[]){GY521_REG_PWR_MGMT_1, g_gy521_cache[0]}, 2, false)) return false;

	sleep_ms(50); // Needed time for the reset

	return true;
}

// ==================
// === Sleep Mode ===
// ==================
bool gy521_sleep(bool device, bool temp){
	if(!gy521_read_register(GY521_REG_PWR_MGMT_1, g_gy521_cache, 1, true)) return false;

	// Sleep Bit
	if(device) g_gy521_cache[0] |= GY521_SLEEP;
	else g_gy521_cache[0] &= ~GY521_SLEEP;

	// Temperature disable Bit
	if(temp) g_gy521_cache[0] |= GY521_TEMP_DIS;
	else g_gy521_cache[0] &= ~GY521_TEMP_DIS;

	if(!gy521_write_register((uint8_t[]){GY521_REG_PWR_MGMT_1, g_gy521_cache[0]}, 2, false)) return false;

	sleep_ms(10); // Activation pause

	return true;
}

// ==================
// === Cycle Mode ===	Low-Power Mode still work in progress
// ==================
/*
 * Set the MPU-6050 cycle mode.
 *
 * Parameters:
 *  - mode:         Select normal cycle mode, low-power cycle mode, or disable cycle.
 *  - smplrt_wake:  Determines the wake-up frequency or the sample-rate divider.
 *                  * For low-power cycle mode, use the predefined gy521_lp_wake_t enum
 *                    for common frequencies (e.g., 1.25Hz, 5Hz, 10Hz, 20Hz, 40Hz).
 *                  * For normal cycle mode use gy521_splrt_div_t enum or use custom rates, any value 0-255 is valid.
 *                    The effective sample rate will be calculated as:
 *                      SampleRate = GyroOutputRate / (1 + smplrt_wake)
 *                    where GyroOutputRate = 8kHz or 1kHz depending on the FCHOICE settings.
 *
 * Notes:
 *  - In low-power mode, all gyroscope axes are automatically put into standby.
 *  - The function updates both PWR_MGMT_1 and PWR_MGMT_2 registers in a single I2C transaction.
 *  - This allows flexible wake frequencies while preserving efficient register writes.
 */
bool gy521_cycle_mode(gy521_cycle_t mode, uint8_t smplrt_wake){
	// Read current power management registers (PWR_MGMT_1 and PWR_MGMT_2)
	if(!gy521_read_register(GY521_REG_PWR_MGMT_1, g_gy521_cache, 2, true)) return false;

	// Enable or disable cycle mode
	if(mode == GY521_CYCLE_ON || mode == GY521_CYCLE_LP){
		g_gy521_cache[0] |= GY521_CYCLE;

		// Standard cycle mode: set sample rate divider
		if(mode != GY521_CYCLE_LP){
			// smplrt_wake = divider for wake-up frequency: freq = 1 kHz / (divider + 1)
			if(!gy521_write_register((uint8_t[]){GY521_REG_SMPLRT_DIV, smplrt_wake}, 2, true)) return false;
		// Low-power cycle mode
		}else if(mode == GY521_CYCLE_LP){
			g_gy521_cache[0] |= GY521_SLEEP; // Put device in sleep, accelerometer wakes up periodically

			g_gy521_cache[1] &= ~GY521_LP_WAKE_40HZ; // Clear previous wake-up frequency bits
			//g_gy521_cache[1] |= smplrt_wake;  // Set new low-power wake-up frequency
			g_gy521_cache[1] |= GY521_STBY_GYRO; // Keep gyro in standby during LP cycle
		}
	}else{
		g_gy521_cache[0] &= ~GY521_CYCLE;  // Clear CYCLE bit
		g_gy521_cache[0] &= ~GY521_SLEEP;
		g_gy521_cache[1] &= ~GY521_LP_WAKE_40HZ; // Clear LP wake frequency bits
		g_gy521_cache[1] &= ~GY521_STBY_GYRO; // Reactivate gyro if it was in standby
	}

	// Write back updated registers
	if(!gy521_write_register((uint8_t[]){GY521_REG_PWR_MGMT_1, g_gy521_cache[0], g_gy521_cache[1]}, 3, false)) return false;

	sleep_ms(10); // Activation pause

	return true;
}

// =====================
// === Stand-By Mode ===
// =====================
bool gy521_stby(uint8_t stby){
	if(!gy521_read_register(GY521_REG_PWR_MGMT_2, g_gy521_cache, 1, true)) return false;

	g_gy521_cache[0] &= ~GY521_STBY_ALL;
	g_gy521_cache[0] |= stby;

	if(!gy521_write_register((uint8_t[]){GY521_REG_PWR_MGMT_2, g_gy521_cache[0]}, 2, false)) return false;

	return true;
}

// ==========================
// === DLPF configuration ===
// ==========================
bool gy521_dlpf_cfg(gy521_dlpf_cfg_t cfg){
	if(!gy521_read_register(GY521_REG_CONFIG, g_gy521_cache, 1, true)) return false;

	g_gy521_cache[0] &= ~GY521_DLPF_CFG_3600HZ;
	g_gy521_cache[0] |= cfg;

	if(!gy521_write_register((uint8_t[]){GY521_REG_CONFIG, g_gy521_cache[0]}, 2, false)) return false;

	return true;
}

#if GY521_INT_PIN
// ===================================
// === Interrupt pin configuration ===
// ===================================
bool gy521_int_pin_cfg(uint8_t cfg){
	if(!gy521_read_register(GY521_REG_INT_PIN_CFG, g_gy521_cache, 1, true)) return false;

	g_gy521_cache[0] &= ~GY521_INT_PIN_CFG_ALL;
	g_gy521_cache[0] |= cfg;

	// Write back to registers
	if(!gy521_write_register((uint8_t[]){GY521_REG_INT_PIN_CFG, g_gy521_cache[0]}, 2, false)) return false;

	return true;
}

// ============================
// === Interrupt pin enable ===
// ============================
bool gy521_int_enable(uint8_t cfg){
	if(!gy521_read_register(GY521_REG_INT_ENABLE, g_gy521_cache, 1, true)) return false;

	g_gy521_cache[0] &= ~GY521_INT_PIN_CFG_ALL;
	g_gy521_cache[0] |= cfg;

	// Write back to registers
	if(!gy521_write_register((uint8_t[]){GY521_REG_INT_ENABLE, g_gy521_cache[0]}, 2, false)) return false;

	gpio_set_irq_enabled_with_callback(GY521_INT_PIN, GPIO_IRQ_EDGE_RISE, true, &gy521_irq_handler);

	return true;
}

// =============================
// === Read interrupt status ===
// =============================
bool gy521_int_status(void){
	if(!g_mpu_irq_flag) return false;
	else g_mpu_irq_flag = false;

	if(!gy521_read_register(GY521_REG_INT_STATUS, g_gy521_cache, 1, false)) return false;

	if((g_gy521_cache[0] & GY521_DATA_RDY_INT) ||
	   (g_gy521_cache[0] & GY521_I2C_MST_INT) ||
	   (g_gy521_cache[0] & GY521_FIFO_OFLOW_INT)) return true;
	else return false;
}
#endif

// ===================================
// ===  Set Full-Scale Range (FSR) ===
// === & Calculate Scaling Factors ===
// ===================================
bool gy521_fsr(gy521_fsr_t fsr, gy521_afsr_t afsr){
	if(!g_gy521) return false;
	// Read FSR Register
	if(!gy521_read_register(GY521_REG_GYRO_CONFIG, g_gy521_cache, 2, true)) return false;
	
	// Gyro FSR bits
	g_gy521_cache[0] &= ~GY521_FSR_2000DPS; // Delete bits 4:3
	g_gy521_cache[0] |= fsr; // Set FSR Bits

	// Automatic scaling calculation:
	// 131 / 2^bits → sensitivity in °/s
	g_gy521->conf.fsr_div.gyro = 131.0f / (1 << ((fsr >> 3) & 0x03));

	// Accel FSR bits
	g_gy521_cache[1] &= ~GY521_AFSR_16G;
	g_gy521_cache[1] |= afsr;

	// Automatic scaling calculation (raw / divider = G)
	g_gy521->conf.fsr_div.accel = 16384.0f / (1 << ((afsr >> 3) & 0x03));

	// Write back to registers
	if(!gy521_write_register((uint8_t[]){GY521_REG_GYRO_CONFIG, g_gy521_cache[0], g_gy521_cache[1]}, 3, false)) return false;

	return true;
}

// ====================================================
// === Calibrate gyro (determine zero-point offset) ===
// ====================================================
bool gy521_calibrate_gyro(uint8_t samples){
	if(!g_gy521) return false;
	
	int64_t sum_gx = 0;
	int64_t sum_gy = 0;
	int64_t sum_gz = 0;

	for(uint8_t i = 0; i < samples; i++){
		if(!gy521_read_register(GY521_REG_GYRO_XOUT_H, g_gy521_cache, 6, false)) return false;

		g_gy521->v.gyro.raw.x = (g_gy521_cache[0]  << 8) | g_gy521_cache[1];
		g_gy521->v.gyro.raw.y = (g_gy521_cache[2]  << 8) | g_gy521_cache[3];
		g_gy521->v.gyro.raw.z = (g_gy521_cache[4]  << 8) | g_gy521_cache[5];

		sum_gx += g_gy521->v.gyro.raw.x;
		sum_gy += g_gy521->v.gyro.raw.y;
		sum_gz += g_gy521->v.gyro.raw.z;

		sleep_ms(5); // small delay between measurements
	}

	// Store averages as offsets
	g_gy521->conf.gyro_offset.x = sum_gx / samples;
	g_gy521->conf.gyro_offset.y = sum_gy / samples;
	g_gy521->conf.gyro_offset.z = sum_gz / samples;

	return true;
}

// ===========================================
// === Read Sensor Data + Optional Scaling ===
// ===========================================
bool gy521_read_sensor(gy521_sensors_t sensors){
	if(!g_gy521) return false;
	// Read all sensors
	uint8_t mask = (sensors & GY521_ALL);
	// If to or more sensors are read it reads all for less overhead.
	if((mask & (mask - 1))){
		if(!gy521_read_register(GY521_REG_ACCEL_XOUT_H, g_gy521_cache, 14, false)) return false;

		g_gy521->v.accel.raw.x = (g_gy521_cache[0]  << 8) | g_gy521_cache[1];
		g_gy521->v.accel.raw.y = (g_gy521_cache[2]  << 8) | g_gy521_cache[3];
		g_gy521->v.accel.raw.z = (g_gy521_cache[4]  << 8) | g_gy521_cache[5];
		g_gy521->v.temp.raw =    (g_gy521_cache[6]  << 8) | g_gy521_cache[7];
		g_gy521->v.gyro.raw.x =  (g_gy521_cache[8]  << 8) | g_gy521_cache[9];
		g_gy521->v.gyro.raw.y =  (g_gy521_cache[10] << 8) | g_gy521_cache[11];
		g_gy521->v.gyro.raw.z =  (g_gy521_cache[12] << 8) | g_gy521_cache[13];

	}else{
		// Only accelerometer
		if(mask & GY521_ACCEL){
			if(!gy521_read_register(GY521_REG_ACCEL_XOUT_H, g_gy521_cache, 6, false)) return false;

			g_gy521->v.accel.raw.x = (g_gy521_cache[0]  << 8) | g_gy521_cache[1];
			g_gy521->v.accel.raw.y = (g_gy521_cache[2]  << 8) | g_gy521_cache[3];
			g_gy521->v.accel.raw.z = (g_gy521_cache[4]  << 8) | g_gy521_cache[5];

		}
		// Only temperatur
		if(mask & GY521_TEMP){
			if(!gy521_read_register(GY521_REG_TEMP_OUT_H, g_gy521_cache, 2, false)) return false;

			g_gy521->v.temp.raw = (g_gy521_cache[0]  << 8) | g_gy521_cache[1];

		}
		// Only gyroscope
		if(mask & GY521_GYRO){
			if(!gy521_read_register(GY521_REG_GYRO_XOUT_H, g_gy521_cache, 6, false)) return false;

			g_gy521->v.gyro.raw.x = (g_gy521_cache[0]  << 8) | g_gy521_cache[1];
			g_gy521->v.gyro.raw.y = (g_gy521_cache[2] << 8) | g_gy521_cache[3];
			g_gy521->v.gyro.raw.z = (g_gy521_cache[4] << 8) | g_gy521_cache[5];
		}
	}

	// Optional: scale raw values
	if(sensors & GY521_SCALED){
		// Raw -> G for accelerometer
		if(mask & GY521_ACCEL){
			g_gy521->v.accel.g.x = (g_gy521->v.accel.raw.x - g_gy521->conf.accel_offset.x) / g_gy521->conf.fsr_div.accel;
			g_gy521->v.accel.g.y = (g_gy521->v.accel.raw.y - g_gy521->conf.accel_offset.y) / g_gy521->conf.fsr_div.accel;
			g_gy521->v.accel.g.z = (g_gy521->v.accel.raw.z - g_gy521->conf.accel_offset.z) / g_gy521->conf.fsr_div.accel;
		}
		// Raw -> °C
		if(mask & GY521_TEMP)
			g_gy521->v.temp.celsius = (g_gy521->v.temp.raw / 340.0f) + 36.53f;
		// Raw -> °/s for gyroscope
		if(mask & GY521_GYRO){
			g_gy521->v.gyro.dps.x = (g_gy521->v.gyro.raw.x - g_gy521->conf.gyro_offset.x) / g_gy521->conf.fsr_div.gyro;
			g_gy521->v.gyro.dps.y = (g_gy521->v.gyro.raw.y - g_gy521->conf.gyro_offset.y) / g_gy521->conf.fsr_div.gyro;
			g_gy521->v.gyro.dps.z = (g_gy521->v.gyro.raw.z - g_gy521->conf.gyro_offset.z) / g_gy521->conf.fsr_div.gyro;
		}
	}

	return true;
}
