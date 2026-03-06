/*
 * ================================================================
 *  Project:      MPU-60X0 Driver for Raspberry Pi Pico
 *  File:         mpu60x0.c
 *  Author:       (Gnibor) Robin Gerhartz
 *  License:      MIT License
 *  Repository:   https://github.com/Gnibor/MPU60X0_RaspberryPi_Pico
 * ================================================================
 *
 *  Description:
 *  Low-level driver implementation for the
 *  MPU-6050 6-axis IMU sensor.
 *
 *  This file implements:
 *  - I²C communication
 *  - Register-level configuration
 *  - Sensor data acquisition
 *  - Automatic scaling (raw -> physical units)
 *  - Gyroscope zero-point calibration
 *  - Power management features
 *  - Timing management features (!!!CYCLE Still Work In Progress!!!)
 *  - Interrupt configuration and status check (!!!Work In Progress!!!)
 *
 *  The driver is written in a lightweight embedded style
 *  and uses function pointers inside a device structure
 *  to emulate object-oriented behavior in C.
 *
 * ================================================================
 */
#include "MPU60X0_reg_map.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <string.h>
#include "mpu60x0.h"

// ===========================
// === Function prototypes ===
// ===========================

// ========================
// === Global Variables ===
// ========================
static mpu_s *g_mpu = NULL; // Global pointer to the aktiv MPU-Device
static int g_mpu_ret_cache = 0; // Temporary buffer for return values

#if MPU_INT_PIN
#endif

// =========================
// === Set device to use ===
// =========================
// to set device as used device for fn.*
bool mpu_use_struct(mpu_s *device){
	if (device == NULL) return false; // Check if device is set

	g_mpu = device;

	return true;
}

// ==========================
// === Initialize MPU ===
// ==========================
mpu_s mpu_init(i2c_inst_t *i2c_port, mpu_addr_t addr){
	i2c_init(MPU_I2C_PORT, 400 * 1000); // 400 kHz I2C
	gpio_set_function(MPU_SDA_PIN, GPIO_FUNC_I2C);
	gpio_set_function(MPU_SCL_PIN, GPIO_FUNC_I2C);

#if MPU_USE_PULLUP
	gpio_pull_up(MPU_SDA_PIN);
	gpio_pull_up(MPU_SCL_PIN);
#endif

	// Configure optional interrupt pin
	gpio_init(MPU_INT_PIN);
	gpio_set_dir(MPU_INT_PIN, GPIO_IN);
#if MPU_INT_PULLUP
	gpio_pull_up(MPU_INT_PIN);
#endif

	mpu_s mpu; // Initalize device struct and function pointers
	memset(&mpu, 0, sizeof(mpu));

	mpu_cache_t mpu_cache[14] = {0};

	if(!i2c_port) mpu.conf.i2c_port = i2c1;
	else mpu.conf.i2c_port = i2c_port;

	if(!addr) mpu.conf.addr = MPU_ADDR_GND;
	else mpu.conf.addr = addr;


	mpu.conf.cache = mpu_cache;

	mpu.conf.fsr_div.accel = 16384.0f;
	mpu.conf.fsr_div.gyro = 131.0f;

	return mpu;
}

// ==========================
// === I²C Register Write ===
// ==========================
bool mpu_write_register(uint8_t *data, uint8_t how_many, bool block){
	if(!g_mpu) return false;

	g_mpu_ret_cache = i2c_write_blocking(g_mpu->conf.i2c_port, g_mpu->conf.addr, data, how_many, block);
	if(g_mpu_ret_cache != how_many) return false;

	return true;
}

// =========================
// === I2C Register Read ===
// =========================
bool mpu_read_register(uint8_t reg, uint8_t *out, uint8_t how_many, bool block){
	if(!g_mpu) return false;

	if(!mpu_write_register(&reg, 1, true)) return false;

	g_mpu_ret_cache = i2c_read_blocking(g_mpu->conf.i2c_port, g_mpu->conf.addr, out, how_many, block);
	if(g_mpu_ret_cache != how_many) return false;

	return true;
}

// =======================
// === Test Connection ===
// =======================
bool mpu_who_am_i(void){
	if(!mpu_read_register(MPU_REG_WHO_AM_I, g_mpu->conf.cache, 1, false)) return false;

	return g_mpu->conf.cache[0] == MPU_WHO_AM_I ? true : false;
}

bool mpu_device_reset(void){
	if(!mpu_read_register(MPU_REG_PWR_MGMT_1, g_mpu->conf.cache, 1, true)) return false;

	g_mpu->conf.cache[0] |= MPU_DEVICE_RESET;

	if(!mpu_write_register((uint8_t[]){MPU_REG_PWR_MGMT_1, g_mpu->conf.cache[0]}, 2, false)) return false;

	sleep_ms(50); // Needed time for the reset

	return true;
}

// ==================
// === Sleep Mode ===
// ==================
bool mpu_sleep(mpu_sleep_t sleep){
	if(!mpu_read_register(MPU_REG_PWR_MGMT_1, g_mpu->conf.cache, 1, true)) return false;

	// Sleep Bit
	if(sleep & MPU_SLEEP_DEVICE_ON) g_mpu->conf.cache[0] |= MPU_SLEEP;
	else if(!(sleep & (MPU_SLEEP_DEVICE_ON << 1))) g_mpu->conf.cache[0] &= ~MPU_SLEEP;

	// Temperature disable Bit
	if(sleep & MPU_SLEEP_TEMP_ON) g_mpu->conf.cache[0] |= MPU_TEMP_DIS;
	else if(!(sleep & (MPU_SLEEP_TEMP_ON << 2))) g_mpu->conf.cache[0] &= ~MPU_TEMP_DIS;

	if(!mpu_write_register((uint8_t[]){MPU_REG_PWR_MGMT_1, g_mpu->conf.cache[0]}, 2, false)) return false;

	sleep_ms(10); // Activation pause

	return true;
}

// =====================
// === Stand-By Mode ===
// =====================
bool mpu_stby(mpu_stby_t stby){
	if(!mpu_read_register(MPU_REG_PWR_MGMT_2, g_mpu->conf.cache, 1, true)) return false;

	g_mpu->conf.cache[0] &= ~MPU_STBY_ALL;
	g_mpu->conf.cache[0] |= stby;

	if(!mpu_write_register((uint8_t[]){MPU_REG_PWR_MGMT_2, g_mpu->conf.cache[0]}, 2, false)) return false;

	return true;
}

// ==========================
// === DLPF configuration ===
// ==========================
bool mpu_dlpf_cfg(mpu_dlpf_cfg_t cfg){
	if(!mpu_read_register(MPU_REG_CONFIG, g_mpu->conf.cache, 1, true)) return false;

	g_mpu->conf.cache[0] &= ~MPU_DLPF_CFG_3600HZ;
	g_mpu->conf.cache[0] |= cfg;

	if(!mpu_write_register((uint8_t[]){MPU_REG_CONFIG, g_mpu->conf.cache[0]}, 2, false)) return false;

	return true;
}

// ===================================
// ===  Set Full-Scale Range (FSR) ===
// === & Calculate Scaling Factors ===
// ===================================
bool mpu_fsr(mpu_fsr_t fsr, mpu_afsr_t afsr){
	// Read FSR Register
	if(!mpu_read_register(MPU_REG_GYRO_CONFIG, g_mpu->conf.cache, 2, true)) return false;
	
	// Gyro FSR bits
	g_mpu->conf.cache[0] &= ~MPU_FSR_2000DPS; // Delete bits 4:3
	g_mpu->conf.cache[0] |= fsr; // Set FSR Bits

	// Automatic scaling calculation:
	// 131 / 2^bits → sensitivity in °/s
	g_mpu->conf.fsr_div.gyro = 131.0f / (1 << ((fsr >> 3) & 0x03));

	// Accel FSR bits
	g_mpu->conf.cache[1] &= ~MPU_AFSR_16G;
	g_mpu->conf.cache[1] |= afsr;

	// Automatic scaling calculation (raw / divider = G)
	g_mpu->conf.fsr_div.accel = 16384.0f / (1 << ((afsr >> 3) & 0x03));

	// Write back to registers
	if(!mpu_write_register((uint8_t[]){MPU_REG_GYRO_CONFIG, g_mpu->conf.cache[0], g_mpu->conf.cache[1]}, 3, false)) return false;

	return true;
}

// ====================================================
// === Calibrate gyro (determine zero-point offset) ===
// ====================================================
bool mpu_calibrate_gyro(uint8_t samples){
	if(!g_mpu) return false;
	
	int64_t sum_gx = 0;
	int64_t sum_gy = 0;
	int64_t sum_gz = 0;

	for(uint8_t i = 0; i < samples; i++){
		if(!mpu_read_register(MPU_REG_GYRO_XOUT_H, g_mpu->conf.cache, 6, false)) return false;

		g_mpu->v.gyro.raw.x = (g_mpu->conf.cache[0]  << 8) | g_mpu->conf.cache[1];
		g_mpu->v.gyro.raw.y = (g_mpu->conf.cache[2]  << 8) | g_mpu->conf.cache[3];
		g_mpu->v.gyro.raw.z = (g_mpu->conf.cache[4]  << 8) | g_mpu->conf.cache[5];

		sum_gx += g_mpu->v.gyro.raw.x;
		sum_gy += g_mpu->v.gyro.raw.y;
		sum_gz += g_mpu->v.gyro.raw.z;

		sleep_ms(5); // small delay between measurements
	}

	// Store averages as offsets
	g_mpu->conf.gyro_offset.x = sum_gx / samples;
	g_mpu->conf.gyro_offset.y = sum_gy / samples;
	g_mpu->conf.gyro_offset.z = sum_gz / samples;

	return true;
}

// ===========================================
// === Read Sensor Data + Optional Scaling ===
// ===========================================
bool mpu_read_sensor(mpu_sensor_t sensors){
	if(!g_mpu) return false;
	// Read all sensors
	uint8_t mask = (sensors & MPU_ALL);
	// If to or more sensors are read it reads all for less overhead.
	if((mask & (mask - 1))){
		if(!mpu_read_register(MPU_REG_ACCEL_XOUT_H, g_mpu->conf.cache, 14, false)) return false;

		g_mpu->v.accel.raw.x = (g_mpu->conf.cache[0]  << 8) | g_mpu->conf.cache[1];
		g_mpu->v.accel.raw.y = (g_mpu->conf.cache[2]  << 8) | g_mpu->conf.cache[3];
		g_mpu->v.accel.raw.z = (g_mpu->conf.cache[4]  << 8) | g_mpu->conf.cache[5];
		g_mpu->v.temp.raw =    (g_mpu->conf.cache[6]  << 8) | g_mpu->conf.cache[7];
		g_mpu->v.gyro.raw.x =  (g_mpu->conf.cache[8]  << 8) | g_mpu->conf.cache[9];
		g_mpu->v.gyro.raw.y =  (g_mpu->conf.cache[10] << 8) | g_mpu->conf.cache[11];
		g_mpu->v.gyro.raw.z =  (g_mpu->conf.cache[12] << 8) | g_mpu->conf.cache[13];

	}else{
		// Only accelerometer
		if(mask & MPU_ACCEL){
			if(!mpu_read_register(MPU_REG_ACCEL_XOUT_H, g_mpu->conf.cache, 6, false)) return false;

			g_mpu->v.accel.raw.x = (g_mpu->conf.cache[0]  << 8) | g_mpu->conf.cache[1];
			g_mpu->v.accel.raw.y = (g_mpu->conf.cache[2]  << 8) | g_mpu->conf.cache[3];
			g_mpu->v.accel.raw.z = (g_mpu->conf.cache[4]  << 8) | g_mpu->conf.cache[5];

		}
		// Only temperatur
		if(mask & MPU_TEMP){
			if(!mpu_read_register(MPU_REG_TEMP_OUT_H, g_mpu->conf.cache, 2, false)) return false;

			g_mpu->v.temp.raw = (g_mpu->conf.cache[0]  << 8) | g_mpu->conf.cache[1];

		}
		// Only gyroscope
		if(mask & MPU_GYRO){
			if(!mpu_read_register(MPU_REG_GYRO_XOUT_H, g_mpu->conf.cache, 6, false)) return false;

			g_mpu->v.gyro.raw.x = (g_mpu->conf.cache[0]  << 8) | g_mpu->conf.cache[1];
			g_mpu->v.gyro.raw.y = (g_mpu->conf.cache[2] << 8) | g_mpu->conf.cache[3];
			g_mpu->v.gyro.raw.z = (g_mpu->conf.cache[4] << 8) | g_mpu->conf.cache[5];
		}
	}

	// Optional: scale raw values
	if(sensors & MPU_SCALED){
		// Raw -> G for accelerometer
		if(mask & MPU_ACCEL){
			g_mpu->v.accel.g.x = (g_mpu->v.accel.raw.x - g_mpu->conf.accel_offset.x) / g_mpu->conf.fsr_div.accel;
			g_mpu->v.accel.g.y = (g_mpu->v.accel.raw.y - g_mpu->conf.accel_offset.y) / g_mpu->conf.fsr_div.accel;
			g_mpu->v.accel.g.z = (g_mpu->v.accel.raw.z - g_mpu->conf.accel_offset.z) / g_mpu->conf.fsr_div.accel;
		}
		// Raw -> °C
		if(mask & MPU_TEMP)
			g_mpu->v.temp.celsius = (g_mpu->v.temp.raw / 340.0f) + 36.53f;
		// Raw -> °/s for gyroscope
		if(mask & MPU_GYRO){
			g_mpu->v.gyro.dps.x = (g_mpu->v.gyro.raw.x - g_mpu->conf.gyro_offset.x) / g_mpu->conf.fsr_div.gyro;
			g_mpu->v.gyro.dps.y = (g_mpu->v.gyro.raw.y - g_mpu->conf.gyro_offset.y) / g_mpu->conf.fsr_div.gyro;
			g_mpu->v.gyro.dps.z = (g_mpu->v.gyro.raw.z - g_mpu->conf.gyro_offset.z) / g_mpu->conf.fsr_div.gyro;
		}
	}

	return true;
}

#if MPU_USE_CYCLE
// ==================
// === Cycle Mode ===	!!!Still work in progress!!!
// ==================
/*
 * Set the MPU-6050 cycle mode.
 *
 * Parameters:
 *  - mode:         Select normal cycle mode, low-power cycle mode, or disable cycle.
 *  - smplrt_wake:  Determines the wake-up frequency or the sample-rate divider.
 *                  * For low-power cycle mode, use the predefined mpu_lp_wake_t enum
 *                    for common frequencies (e.g., 1.25Hz, 5Hz, 10Hz, 20Hz, 40Hz).
 *                  * For normal cycle mode use mpu_splrt_div_t enum or use custom rates, any value 0-255 is valid.
 *                    The effective sample rate will be calculated as:
 *                      SampleRate = GyroOutputRate / (1 + smplrt_wake)
 *                    where GyroOutputRate = 8kHz or 1kHz depending on the FCHOICE settings.
 *
 * Notes:
 *  - In low-power mode, all gyroscope axes are automatically put into standby.
 *  - The function updates both PWR_MGMT_1 and PWR_MGMT_2 registers in a single I2C transaction.
 *  - This allows flexible wake frequencies while preserving efficient register writes.
 */
bool mpu_cycle_mode(mpu_cycle_t mode, uint8_t smplrt_wake){
	// Read current power management registers (PWR_MGMT_1 and PWR_MGMT_2)
	if(!mpu_read_register(MPU_REG_PWR_MGMT_1, g_mpu->conf.cache, 2, true)) return false;

	// Enable or disable cycle mode
	if(mode == MPU_CYCLE_ON || mode == MPU_CYCLE_LP){
		g_mpu->conf.cache[0] |= MPU_CYCLE;

		// Standard cycle mode: set sample rate divider
		if(mode != MPU_CYCLE_LP){
			// smplrt_wake = divider for wake-up frequency: freq = 1 kHz / (divider + 1)
			if(!mpu_write_register((uint8_t[]){MPU_REG_SMPLRT_DIV, smplrt_wake}, 2, true)) return false;
		// Low-power cycle mode
		}else if(mode == MPU_CYCLE_LP){
			g_mpu->conf.cache[0] |= MPU_SLEEP; // Put device in sleep, accelerometer wakes up periodically

			g_mpu->conf.cache[1] &= ~MPU_LP_WAKE_40HZ; // Clear previous wake-up frequency bits
			//g_mpu->conf.cache[1] |= smplrt_wake;  // Set new low-power wake-up frequency
			g_mpu->conf.cache[1] |= MPU_STBY_GYRO; // Keep gyro in standby during LP cycle
		}
	}else{
		g_mpu->conf.cache[0] &= ~MPU_CYCLE;  // Clear CYCLE bit
		g_mpu->conf.cache[0] &= ~MPU_SLEEP;
		g_mpu->conf.cache[1] &= ~MPU_LP_WAKE_40HZ; // Clear LP wake frequency bits
		g_mpu->conf.cache[1] &= ~MPU_STBY_GYRO; // Reactivate gyro if it was in standby
	}

	// Write back updated registers
	if(!mpu_write_register((uint8_t[]){MPU_REG_PWR_MGMT_1, g_mpu->conf.cache[0], g_mpu->conf.cache[1]}, 3, false)) return false;

	sleep_ms(10); // Activation pause

	return true;
}
#endif

#if MPU_INT_PIN
volatile bool g_mpu_int_flag;

void mpu_irq_handler(uint gpio, uint32_t events){
    if(gpio == MPU_INT_PIN){
        g_mpu_int_flag = true;
    }
}

// ===================================
// === Interrupt pin configuration === !!!Still work in progress!!!
// ===================================
bool mpu_int_pin_cfg(mpu_int_pin_cfg_t cfg){
	if(!mpu_read_register(MPU_REG_INT_PIN_CFG, g_mpu->conf.cache, 1, true)) return false;

	g_mpu->conf.cache[0] &= ~MPU_INT_PIN_CFG_ALL;
	g_mpu->conf.cache[0] |= cfg;

	// Write back to registers
	if(!mpu_write_register((uint8_t[]){MPU_REG_INT_PIN_CFG, g_mpu->conf.cache[0]}, 2, false)) return false;

	return true;
}

// ============================
// === Interrupt pin enable === !!!Still work in progress!!!
// ============================
bool mpu_int_enable(mpu_int_enable_t type){
	if(!mpu_read_register(MPU_REG_INT_ENABLE, g_mpu->conf.cache, 1, true)) return false;

	g_mpu->conf.cache[0] &= ~MPU_INT_PIN_CFG_ALL;
	g_mpu->conf.cache[0] |= type;

	// Write back to registers
	if(!mpu_write_register((uint8_t[]){MPU_REG_INT_ENABLE, g_mpu->conf.cache[0]}, 2, false)) return false;


	return true;
}

// =============================
// === Read interrupt status === !!!Still work in progress!!!
// =============================
bool mpu_int_status(void){
	if(!mpu_read_register(MPU_REG_INT_STATUS, g_mpu->conf.cache, 1, false)) return false;

	if((g_mpu->conf.cache[0] & MPU_DATA_RDY_INT) ||
	   (g_mpu->conf.cache[0] & MPU_I2C_MST_INT) ||
	   (g_mpu->conf.cache[0] & MPU_FIFO_OFLOW_INT)) return true;
	else return false;
}
#endif
