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
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include <pico/time.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "mpu60x0.h"
#include "MPU60X0_reg_map.h"
#include "rp_pico.h"
#include "ansi-esc.h"

// ===========================
// === Function prototypes ===
// ===========================

// ========================
// === Global Variables ===
// ========================
static mpu_cache_t gc_mpu[14] = {0};
static mpu_s *g_mpu = NULL; // Global pointer to the aktiv MPU-Device
static int g_mpu_ret_cache = 0; // Temporary buffer for return values

// ==========================
// === Initialize MPU ===
// ==========================
mpu_s mpu_init(i2c_inst_t *i2c_port, mpu_addr_t addr){
	mpu_s mpu; // Initalize device struct and function pointers
	memset(&mpu, 0, sizeof(mpu));

	if(!i2c_port){
		mpu.conf.i2c_port = i2c1;
		LOG_W("mpu_init(): i2c_port not valid instead "ANSI_ITALIC ANSI_BOLD"i2c1"ANSI_RESET" is used");
	}else mpu.conf.i2c_port = i2c_port;

	if(!addr){
		mpu.conf.addr = MPU_ADDR_AD0_GND;
		LOG_W("mpu_init(): addr not valid instead "ANSI_ITALIC ANSI_BOLD"0x%02X"ANSI_RESET" is used", MPU_ADDR_AD0_GND);
	}else mpu.conf.addr = addr;

	mpu.conf.fsr_div.accel = 16384.0f;
	mpu.conf.fsr_div.gyro = 131.0f;
	g_mpu = &mpu;

	if(!is_i2c_initialized(i2c_port)){
		i2c_init(mpu.conf.i2c_port, 400 * 1000); // 400 kHz I2C
		gpio_set_function(MPU_SDA_PIN, GPIO_FUNC_I2C);
		gpio_set_function(MPU_SCL_PIN, GPIO_FUNC_I2C);
		LOG_I("mpu_init(): I²C initialized at 400kHz SDA:%d SCL:%d", MPU_SDA_PIN, MPU_SCL_PIN);
	}else{
		LOG_W("mpu_init(): I²C is already initialized");
	}

#if MPU_USE_PULLUP
	gpio_pull_up(MPU_SDA_PIN);
	gpio_pull_up(MPU_SCL_PIN);
	LOG_D("mpu_init(): I²C pull-up on");
#endif

#if MPU_INT_PIN
	// Configure optional interrupt pin
	gpio_init(MPU_INT_PIN);
	gpio_set_dir(MPU_INT_PIN, GPIO_IN);
	LOG_I("mpu_init(): interrupt pin GPIO%d", MPU_INT_PIN);
#endif
#if MPU_INT_PULLUP
	gpio_pull_up(MPU_INT_PIN);
	LOG_D("mpu_init(): MPU_INT_PIN pull-up on");
#endif

	if(!mpu_who_am_i()){
		LOG_E("mpu_init(): failed exec mpu_who_am_i()");
	}

	if(!mpu_sleep(MPU_SLEEP_ALL_OFF)){
		LOG_E("mpu_init(): faild exec mpu_sleep(MPU_SLEEP_ALL_OFF)");
	}
	return mpu;
}

// =========================
// === Set device to use ===
// =========================
// to set device as used device for the functions who need the struct
bool mpu_use_struct(mpu_s *device){
	if (device == NULL){
		LOG_E("mpu_use_struct(): device = NULL");
		return false; // Check if device is set
	}
	g_mpu = device;
	LOG_I("mpu_use_struct(): g_mpu got set");

	return true;
}

// ==========================
// === I²C Register Write ===
// ==========================
bool mpu_write_register(uint8_t *data, uint8_t how_many, bool block){
	if(!g_mpu){
		return false;
		LOG_E("mpu_write_register(): empty "ANSI_ITALIC ANSI_BOLD"g_mpu"ANSI_RESET);
		LOG_I("mpu_write_register(): use mpu_use_struct() to set "ANSI_ITALIC ANSI_BOLD"g_mpu"ANSI_RESET);
	}
	g_mpu_ret_cache = i2c_write_blocking(g_mpu->conf.i2c_port, g_mpu->conf.addr, data, how_many, block);
	if(g_mpu_ret_cache == how_many){
		LOG_D("mpu_write_register(): %d bytes written to reg 0x%02X", how_many, data[0]);
	}else{
		LOG_E("mpu_write_register(): I²C failed at reg 0x%02X (return: %d)", data[0], how_many);
		return false;
	}

	return true;
}

// =========================
// === I2C Register Read ===
// =========================
bool mpu_read_register(uint8_t reg, uint8_t *out, uint8_t how_many, bool block){
	if(!g_mpu){
		return false;
		LOG_E("mpu_read_register(): empty "ANSI_ITALIC ANSI_BOLD"g_mpu"ANSI_RESET);
		LOG_I("mpu_read_register(): use mpu_use_struct() to set "ANSI_ITALIC ANSI_BOLD"g_mpu"ANSI_RESET);
	}

	if(!mpu_write_register(&reg, 1, true)){
		LOG_E("mpu_read_register(): failed to write reg 0x%02X", reg);
		return false;
	}
	g_mpu_ret_cache = i2c_read_blocking(g_mpu->conf.i2c_port, g_mpu->conf.addr, out, how_many, block);
	if(g_mpu_ret_cache == how_many){
		LOG_D("mpu_read_register(): %d bytes read from reg 0x%02X", how_many, reg);
		return true;
	}else{
		LOG_E("mpu_read_register(): I²C failed at reg 0x%02X (return: %d)", reg, how_many);
		return false;
	}
}

// =======================
// === Test Connection ===
// =======================
bool mpu_who_am_i(void){
	if(!mpu_read_register(MPU_REG_WHO_AM_I, gc_mpu, 1, false)){
		LOG_E("mpu_who_am_i(): failed exec mpu_read_register()");
		return false;
	}

	if(gc_mpu[0] == MPU_WHO_AM_I){
		LOG_I("mpu_who_am_i(): device is a MPU60X0");
		return true;
	}else{
		LOG_E("mpu_who_am_i(): device is not a MPU60X0");
		return false;
	}
}

/*
 * Perform resets
 *
 * @param reset Bitmask defining which components to reset.
 * @return true if all I2C operations succeeded.
 */
bool mpu_reset(mpu_reset_t reset) {
	// 1. Read current register values into global cache to preserve existing bits
	// We read 3 bytes starting from SIGNAL_PATH_RESET (likely covering USER_CTRL & PWR_MGMT_1)
	if (!mpu_read_register(MPU_REG_SIGNAL_PATH_RESET, (uint8_t[]){gc_mpu[1], gc_mpu[2], gc_mpu[3]}, 3, true)) {
		LOG_E("mpu_reset(): Failed to read reg SIGNAL_PATH_RESET (0x%02X)", MPU_REG_SIGNAL_PATH_RESET);
		return false;
	}

	// 2. Modify SIGNAL_PATH_RESET bits (TEMP, ACCEL, GYRO)
	if (reset & MPU_RESET_TEMP)  gc_mpu[1] |= MPU_TEMP_RESET;
	if (reset & MPU_RESET_ACCEL) gc_mpu[1] |= MPU_ACCEL_RESET;
	if (reset & MPU_RESET_GYRO)  gc_mpu[1] |= MPU_GYRO_RESET;

	// 3. Modify USER_CTRL bits (SIG_COND, I2C_MST, FIFO)
	if (reset & MPU_RESET_SIG_COND) gc_mpu[2] |= MPU_SIG_COND_RESET;
	if (reset & MPU_RESET_I2C_MST)  gc_mpu[2] |= MPU_I2C_MST_RESET;
	if (reset & MPU_RESET_FIFO)     gc_mpu[2] |= MPU_FIFO_RESET;

	// 4. Modify PWR_MGMT_1 bits (DEVICE_RESET)
	if (reset & MPU_RESET_DEVICE) gc_mpu[3] |= MPU_DEVICE_RESET;

	// 5. Execution Logic
	if (reset & MPU_RESET_ALL) {
		LOG_I("mpu_reset(): Starting FULL chip reset sequence...");

		// Trigger Main Device Reset via PWR_MGMT_1
		gc_mpu[2] |= MPU_DEVICE_RESET; // Warning: index logic should match your register mapping
		if (!mpu_write_register((uint8_t[]){MPU_REG_PWR_MGMT_1, gc_mpu[3]}, 2, true)) return false;
		sleep_ms(150); // Wait for internal reboot

		// Reset Signal Paths (Analog/Digital Filters)
		LOG_D("mpu_reset(): Clearing Signal Paths...");
		gc_mpu[0] |= (MPU_TEMP_RESET | MPU_ACCEL_RESET | MPU_GYRO_RESET);
		if (!mpu_write_register((uint8_t[]){MPU_REG_SIGNAL_PATH_RESET, gc_mpu[1]}, 2, true)) return false;
		sleep_ms(200);

		// Reset User Control (FIFO, I2C Master, Logic)
		LOG_D("mpu_reset(): Clearing User Control Logic...");
		gc_mpu[1] |= (MPU_SIG_COND_RESET | MPU_I2C_MST_RESET | MPU_FIFO_RESET);
		if (!mpu_write_register((uint8_t[]){MPU_REG_USER_CTRL, gc_mpu[2]}, 2, false)) return false;
		sleep_ms(200);

		LOG_I("mpu_reset(): Full reset complete.");
	} else {
		// Partial reset of specific signal paths
		LOG_I("mpu_reset(): Partial reset requested (Mask: 0x%02X)", reset);
		gc_mpu[0] = MPU_REG_SIGNAL_PATH_RESET;
		if (!mpu_write_register(gc_mpu, 4, false)) {
			LOG_E("mpu_reset(): Write failed for partial reset");
			return false;
		}
		sleep_ms(100);
	}

	return true;
}

// ==================
// === Sleep Mode ===
// ==================
bool mpu_sleep(mpu_sleep_t sleep){
	if(!mpu_read_register(MPU_REG_PWR_MGMT_1, gc_mpu, 1, true)){
		LOG_E("mpu_sleep(): failed to read reg PWR_MGMT_1 (0x%02X)", MPU_REG_PWR_MGMT_1);
		return false;
	}

	// Sleep Bit
	if(sleep & MPU_SLEEP_DEVICE_ON){
		gc_mpu[0] |= MPU_SLEEP;
		LOG_I("mpu_sleep(): initiating device sleep sequence...");
	}else if(!(sleep & (MPU_SLEEP_DEVICE_ON << 1))){
		gc_mpu[0] &= ~MPU_SLEEP;
		LOG_I("mpu_sleep(): initiating device wake up...");
	}

	// Temperature disable Bit
	if(sleep & MPU_SLEEP_TEMP_ON){
		gc_mpu[0] |= MPU_TEMP_DIS;
		LOG_I("mpu_sleep(): initiating temp sleep sequence...");
	}else if(!(sleep & (MPU_SLEEP_TEMP_ON << 2))){
		gc_mpu[0] &= ~MPU_TEMP_DIS;
		LOG_I("mpu_sleep(): initiating temp wake up...");
	}

	if(!mpu_write_register((uint8_t[]){MPU_REG_PWR_MGMT_1, gc_mpu[0]}, 2, false)){
		LOG_E("mpu_sleep(): failed to write 0x%02X to reg PWR_MGMT_1 (0x%02X)", gc_mpu[0], MPU_REG_PWR_MGMT_1);
		return false;
	}

	sleep_ms(5); // Activation pause

	LOG_I("mpu_sleep(): done...");

	return true;
}

// =====================
// === Stand-By Mode ===
// =====================
bool mpu_stby(mpu_stby_t stby){
	if(!mpu_read_register(MPU_REG_PWR_MGMT_2, gc_mpu, 1, true)){
		LOG_E("mpu_stby(): failed to read reg PWR_MGMT_2 (0x%02X)", MPU_REG_PWR_MGMT_2);
		return false;
	}

	gc_mpu[0] &= ~MPU_STBY_ALL;
	gc_mpu[0] |= stby;

	if(!mpu_write_register((uint8_t[]){MPU_REG_PWR_MGMT_2, gc_mpu[0]}, 2, false)){
		LOG_E("mpu_stby(): failed to write 0x%02X to reg PWR_MGMT_2 (0x%02X)", gc_mpu[0], MPU_REG_PWR_MGMT_2);
		return false;
	}

	sleep_ms(5);

	LOG_I("mpu_stby(): written 0x%02X to reg PWR_MGMT_2 (0x%02X)", gc_mpu[0], MPU_REG_PWR_MGMT_2);

	return true;
}

bool mpu_clk_sel(mpu_clk_sel_t clksel){
	if(!mpu_read_register(MPU_REG_PWR_MGMT_1, gc_mpu, 1, true)){
		LOG_E("mpu_clk_sel(): failed to read reg PWR_MGMT_1 (0x%02X)", MPU_REG_PWR_MGMT_1);
		return false;
	}

	gc_mpu[0] &= ~MPU_CLK_STOP;
	gc_mpu[0] |= clksel;

	if(!mpu_write_register((uint8_t[]){MPU_REG_PWR_MGMT_1, gc_mpu[0]}, 2, false)){
		LOG_E("mpu_clk_sel(): failed to write 0x%02X to reg PWR_MGMT_1 (0x%02X)", gc_mpu[0], MPU_REG_PWR_MGMT_1);
		return false;
	}

	sleep_ms(5);

	LOG_I("mpu_clk_sel(): written 0x%02X to reg PWR_MGMT_1 (0x%02X)", gc_mpu[0], MPU_REG_PWR_MGMT_1);

	return true;
}

// ==========================
// === DLPF configuration ===
// ==========================
bool mpu_dlpf_cfg(mpu_dlpf_cfg_t cfg){
	if(!mpu_read_register(MPU_REG_CONFIG, gc_mpu, 1, true)){
		LOG_E("mpu_dlpf_cfg(): failed to read reg CONFIG (0x%02X)", MPU_REG_CONFIG);
		return false;
	}

	gc_mpu[0] &= ~MPU_DLPF_CFG_3600HZ;
	gc_mpu[0] |= cfg;

	if(!mpu_write_register((uint8_t[]){MPU_REG_CONFIG, gc_mpu[0]}, 2, false)){
		LOG_E("mpu_dlpf_cfg(): failed to write 0x%02X to reg CONFIG (0x%02X)", gc_mpu[0], MPU_REG_CONFIG);
		return false;
	}

	sleep_ms(5);

	LOG_I("mpu_dlpf_cfg(): written 0x%02X to reg CONFIG (0x%02X)", gc_mpu[0], MPU_REG_CONFIG);

	return true;
}

// =======================
// === Set Sample Rate ===
// =======================
bool mpu_smplrt_div(mpu_smplrt_div_t smplrt_div){
	gc_mpu[0] = MPU_REG_SMPLRT_DIV;
	gc_mpu[1] = smplrt_div;

	if(!mpu_write_register(gc_mpu, 2, false)){
		LOG_E("mpu_smplrt_div(): failed to write 0x%02X to reg SMPLRT_DIV (0x%02X)", gc_mpu[1], gc_mpu[0]);
		return false;
	}

	sleep_ms(5);

	LOG_I("mpu_smplrt_div(): written 0x%02X to reg SMPLRT_DIV (0x%02X)", gc_mpu[1], gc_mpu[0]);

	return true;
}

// ==============================
// === Accel High Pass Filter ===
// ==============================
bool mpu_ahpf(mpu_ahpf_t ahpf){
	if(!mpu_read_register(MPU_REG_ACCEL_CONFIG, gc_mpu, 1, true)){
		LOG_E("mpu_ahpf(): failed to read reg ACCEL_CONFIG (0x%02X)", MPU_REG_ACCEL_CONFIG);
		return false;
	}

	gc_mpu[0] &= ~MPU_AHPF_HOLD;
	gc_mpu[1] = (gc_mpu[0] | ahpf);
	gc_mpu[0] = MPU_REG_ACCEL_CONFIG;

	if(!mpu_write_register(gc_mpu, 2, false)){
		LOG_E("mpu_ahpf(): failed to write 0x%02X to reg ACCEL_CONFIG (0x%02X)", gc_mpu[1], gc_mpu[0]);
		return false;
	}

	sleep_ms(5);

	LOG_I("mpu_ahpf(): written 0x%02X to reg ACCEL_CONFIG (0x%02X)", gc_mpu[1], gc_mpu[0]);

	return true;
}

// ==================
// === Cycle Mode ===
// ==================
/*
 * Set the MPU-6050 cycle mode.
 *
 * Parameters:
 *  - mode:         Select normal cycle mode, low-power cycle mode, or disable cycle.
 *  - smplrt_wake:  Determines the wake-up frequency.
 *                  * For low-power cycle mode, use the predefined mpu_lp_wake_t enum
 *                    for common frequencies (e.g., 1.25Hz, 5Hz, 10Hz, 20Hz, 40Hz).
 *
 * Notes:
 *  - In low-power mode, all gyroscope axes are automatically put into standby.
 *  - The function updates both PWR_MGMT_1 and PWR_MGMT_2 registers in a single I2C transaction.
 */
bool mpu_cycle_mode(mpu_cycle_t mode, mpu_lp_wake_t wake_up_rate){
	// Read current power management registers (PWR_MGMT_1 and PWR_MGMT_2)
	if(!mpu_read_register(MPU_REG_PWR_MGMT_1, gc_mpu, 2, true)){
		LOG_E("mpu_cycle_mode(): failed to read 2 bytes from reg PWR_MGMT_1 (0x%02X)", MPU_REG_PWR_MGMT_1);
		return false;
	}

	// Enable or disable cycle mode
	if(mode == MPU_CYCLE_ON || mode == MPU_CYCLE_LP){
		gc_mpu[0] |= MPU_CYCLE; // Activate CYCLE (set to 1)
		gc_mpu[0] &= ~MPU_SLEEP; // Deactivate SLEEP (set to 0)

		gc_mpu[1] &= ~MPU_LP_WAKE_40HZ; // Clear previous wake-up frequency bits
		gc_mpu[1] |= wake_up_rate; // Set new wake-up frequency
		if(mode == MPU_CYCLE_LP){
			gc_mpu[0] |= MPU_TEMP_DIS; // Deactivate Temperature sensor (set to 1)

			gc_mpu[1] |= MPU_STBY_GYRO; // Keep gyro in standby during LP cycle

			LOG_I("mpu_cycle_mode(): initiating low power cycle mode");
		}
		LOG_I("mpu_cycle_mode(): initiating cycle mode");
	}else{
		LOG_I("mpu_cycle_mode(): initiating deactivation of cycle mode");
		gc_mpu[0] &= ~MPU_CYCLE;  // Clear CYCLE bit
		gc_mpu[0] &= ~MPU_TEMP_DIS; // Reactivate temp if it was in standby
		gc_mpu[1] &= ~MPU_LP_WAKE_40HZ; // Clear LP wake frequency bits
		gc_mpu[1] &= ~MPU_STBY_GYRO; // Reactivate gyro if it was in standby
	}

	// Write back updated registers
	if(!mpu_write_register((uint8_t[]){MPU_REG_PWR_MGMT_1, gc_mpu[0], gc_mpu[1]}, 3, false)){
		LOG_E("mpu_cycle_mode(): failed to write 0x%02X, 0x%02X to reg PWR_MGMT_1 (0x%02X)", gc_mpu[0], gc_mpu[1], MPU_REG_PWR_MGMT_1);
		return false;
	}

	sleep_ms(10); // Activation pause

	LOG_I("mpu_cycle_mode(): initiating complete. written 0x%02X, 0x%02X to reg PWR_MGMT_1 (0x%02X)", gc_mpu[0], gc_mpu[1], MPU_REG_PWR_MGMT_1);

	return true;
}

// ===================================
// ===  Set Full-Scale Range (FSR) ===
// === & Calculate Scaling Factors ===
// ===================================
bool mpu_fsr(mpu_fsr_t fsr, mpu_afsr_t afsr){
	// Read FSR Register
	if(!mpu_read_register(MPU_REG_GYRO_CONFIG, gc_mpu, 2, true)){
		LOG_E("mpu_fsr(): failed to read 2 bytes from reg GYRO_CONFIG (0x%02X)", MPU_REG_GYRO_CONFIG);
		return false;
	}
	
	// Gyro FSR bits
	gc_mpu[0] &= ~MPU_FSR_2000DPS; // Delete bits 4:3
	gc_mpu[0] |= fsr; // Set FSR Bits
	LOG_I("mpu_fsr(): prepare fsr (0x%02X)", fsr);

	// Automatic scaling calculation:
	// 131 / 2^bits → sensitivity in °/s
	g_mpu->conf.fsr_div.gyro = 131.0f / (1 << ((fsr >> 3) & 0x03));
	LOG_I("mpu_fsr(): FSR_DIV set to %d", g_mpu->conf.fsr_div.gyro);

	// Accel FSR bits
	gc_mpu[1] &= ~MPU_AFSR_16G;
	gc_mpu[1] |= afsr;
	LOG_I("mpu_fsr(): prepare afsr (0x%02X)", afsr);

	// Automatic scaling calculation (raw / divider = G)
	g_mpu->conf.fsr_div.accel = 16384.0f / (1 << ((afsr >> 3) & 0x03));
	LOG_I("mpu_fsr(): AFSR_DIV set to %d", g_mpu->conf.fsr_div.accel);

	// Write back to registers
	if(!mpu_write_register((uint8_t[]){MPU_REG_GYRO_CONFIG, gc_mpu[0], gc_mpu[1]}, 3, false)){
		LOG_E("mpu_fsr(): failed to write 0x%02X, 0x%02X to reg GYRO_CONFIG (0x%02X)", gc_mpu[0], gc_mpu[1], MPU_REG_GYRO_CONFIG);
		return false;
	}

	sleep_ms(5);

	return true;
}

// ====================================================
// ======= Calibrate Accelerometer or Gyroscope =======
// ========== (determine zero-point offset) ===========
// ====================================================
// * Calibrate the accelerometer or gyroscope.        *
// * Use the `mpu_sensor_t` for setting wich sensor   *
// * should get calibrated. Use for the Accelerometer *
// * the in `mpu_sensor_t` given axis type or your    *
// * accelerometer will get complietly 0.             *
// * Offset will get saved in the device struct.      *
// ====================================================
/*
 * @parameter:
 * 	- sensor = mpu_sensor_t sets wich sensor(s)
 * 	- samples = how many samples it should make
 *
 * @return:
 * 	- true = everything is ok
 * 	- false = struct not set to use or failed to read the sensor
 */
bool mpu_calibrate(mpu_sensor_t sensor, uint8_t samples){
	if(!g_mpu){
		return false;
		LOG_E("mpu_calibrate(): empty "ANSI_ITALIC ANSI_BOLD"g_mpu"ANSI_RESET);
		LOG_I("mpu_calibrate(): use mpu_use_struct() to set "ANSI_ITALIC ANSI_BOLD"g_mpu"ANSI_RESET);
	}

	int64_t sum_x = 0; // cache
	int64_t sum_y = 0; // cache
	int64_t sum_z = 0; // cache

	uint8_t mask = (sensor & MPU_ALL); // mask where only the sensors bits are given

	if(mask & MPU_GYRO){ // Checks if gyro should be calibrated
		LOG_I("mpu_calibrate(): starting gyro calibration...");
		for(uint8_t i = 0; i < samples; i++){
			if(!mpu_read_register(MPU_REG_GYRO_XOUT_H, gc_mpu, 6, false)){ // Read the gyro output
				LOG_E("mpu_calibrate(): failed to read 6 bytes from reg GYRO_XOUT_H (0x%02X)", MPU_REG_GYRO_XOUT_H);
				return false;
			}

			g_mpu->v.gyro.raw.x = (gc_mpu[0]  << 8) | gc_mpu[1]; // Store x axis output in gyro.raw.x
			g_mpu->v.gyro.raw.y = (gc_mpu[2]  << 8) | gc_mpu[3]; // Store y axis output in gyro.raw.y
			g_mpu->v.gyro.raw.z = (gc_mpu[4]  << 8) | gc_mpu[5]; // Store z axis output in gyro.raw.z

			sum_x += g_mpu->v.gyro.raw.x; // Add x axis output to sum_x
			sum_y += g_mpu->v.gyro.raw.y; // Add y axis output to sum_y
			sum_z += g_mpu->v.gyro.raw.z; // Add z axis output to sum_z

			sleep_ms(5); // small delay between measurements
		}

		g_mpu->conf.offset_gyro.x = sum_x / samples; // Store x axis average as offset_gyro.x
		g_mpu->conf.offset_gyro.y = sum_y / samples; // Store y axis average as offset_gyro.y
		g_mpu->conf.offset_gyro.z = sum_z / samples; // Store z axis average as offset_gyro.z

		LOG_I("mpu_calibrate(): gyro calibration done");
		LOG_D("mpu_calibrate(): gyro offset x=%d, y=%d, z=%d", g_mpu->conf.offset_gyro.x, g_mpu->conf.offset_gyro.y, g_mpu->conf.offset_gyro.z);

	}
	if (mask & MPU_ACCEL){ // Checks if accelerometer should be calibrated
		LOG_I("mpu_calibrate(): starting accel calibration...");
		for(uint8_t i = 0; i < samples; i++){
			sum_x = 0; sum_y = 0; sum_z = 0; // Set sum back to `0` in case both sensors got read
			if(!mpu_read_register(MPU_REG_ACCEL_XOUT_H, gc_mpu, 6, false)){ // Read the accel output
				LOG_E("mpu_calibrate(): failed to read 6 bytes from reg ACCEL_XOUT_H (0x%02X)", MPU_REG_ACCEL_XOUT_H);
				return false;
			}

			g_mpu->v.accel.raw.x = (gc_mpu[0]  << 8) | gc_mpu[1]; // Store x axis output in accel.raw.x
			g_mpu->v.accel.raw.y = (gc_mpu[2]  << 8) | gc_mpu[3]; // Store y axis output in accel.raw.y
			g_mpu->v.accel.raw.z = (gc_mpu[4]  << 8) | gc_mpu[5]; // Store z axis output in accel.raw.z

			sum_x += g_mpu->v.accel.raw.x; // Add x axis output to sum_x
			sum_y += g_mpu->v.accel.raw.y; // Add y axis output to sum_y
			sum_z += g_mpu->v.accel.raw.z; // Add z axis output to sum_z

			sleep_ms(5); // small delay between measurements
		}

		g_mpu->conf.offset_accel.x = sum_x / samples; // Store x axis average as offset_accel.x
		g_mpu->conf.offset_accel.y = sum_y / samples; // Store y axis average as offset_accel.y
		g_mpu->conf.offset_accel.z = sum_z / samples; // Store z axis average as offset_accel.z

		if(sensor & MPU_ACCEL_X){
			g_mpu->conf.offset_accel.x -= 16384.0f; // minus earth own gravity
		}else if(sensor & MPU_ACCEL_Y){
			g_mpu->conf.offset_accel.y -= 16384.0f; // minus earth own gravity
		}else if(sensor & MPU_ACCEL_Z){
			g_mpu->conf.offset_accel.z -= 16384.0f; // minus earth own gravity
		}
		LOG_I("mpu_calibrate(): accel calibration done");
		LOG_D("mpu_calibrate(): accel offset x=%d, y=%d, z=%d", g_mpu->conf.offset_accel.x, g_mpu->conf.offset_accel.y, g_mpu->conf.offset_accel.z);
	}

	return true; // If everything goes right
}

// ============================================
// === Read Sensor Data + Optional Scaling ====
// ============================================
// * Reads all sensors or only optional       *
// * scaling based on the given argument.     *
// ============================================
/* @parameter:
 *	- sensor = Takes sensors given as `mpu_sensor_t`
 *
 * @return:
 *	- true = could read (and Calculate) the given `sensors`
 *	- false = failed to read or calculate the given `sensors`
 *
 * @notes:
 *	- If two sensors are given all three will be read (less I²C overhead), but only the given will be scaled (if given).
 */
bool mpu_read_sensor(mpu_sensor_t sensor){
	if(!g_mpu){
		return false;
		LOG_E("mpu_read_sensor(): empty "ANSI_ITALIC ANSI_BOLD"g_mpu"ANSI_RESET);
		LOG_I("mpu_read_sensor(): use mpu_use_struct() to set "ANSI_ITALIC ANSI_BOLD"g_mpu"ANSI_RESET);
	}

	uint8_t mask = (sensor & MPU_ALL); // A mask where only sensor bits are given
	
	if((mask & (mask - 1))){ // If two or more sensors are read it reads all for less overhead.

		if(!mpu_read_register(MPU_REG_ACCEL_XOUT_H, gc_mpu, 14, false)){ // Read all output register
			LOG_E("mpu_read_sensor(): failed to read 14 bytes from reg ACCEL_XOUT_H (0x%02X)", MPU_REG_ACCEL_XOUT_H);
			return false;
		}

		g_mpu->v.accel.raw.x = (gc_mpu[0]  << 8) | gc_mpu[1]; // Save raw accelerometer x axis
		g_mpu->v.accel.raw.y = (gc_mpu[2]  << 8) | gc_mpu[3]; // Save raw accelerometer y axis
		g_mpu->v.accel.raw.z = (gc_mpu[4]  << 8) | gc_mpu[5]; // Save raw accelerometer z axis
		g_mpu->v.temp.raw =    (gc_mpu[6]  << 8) | gc_mpu[7]; // Save raw temperatur
		g_mpu->v.gyro.raw.x =  (gc_mpu[8]  << 8) | gc_mpu[9]; // Save raw gyro x axis
		g_mpu->v.gyro.raw.y =  (gc_mpu[10] << 8) | gc_mpu[11]; // Save raw gyro y axis ;
		g_mpu->v.gyro.raw.z =  (gc_mpu[12] << 8) | gc_mpu[13]; // Save raw gyro z axis ;

		LOG_D("mpu_read_sensor(): accel: x=%d, y=%d, z=%d; temp=%d; gyro: x=%d, y=%d, z=%d;",
				g_mpu->v.accel.raw.x, g_mpu->v.accel.raw.y, g_mpu->v.accel.raw.z,
				g_mpu->v.temp.raw,
				g_mpu->v.gyro.raw.x, g_mpu->v.gyro.raw.y, g_mpu->v.gyro.raw.z);
	}else{
		if(mask & MPU_ACCEL){ // Only accelerometer
			if(!mpu_read_register(MPU_REG_ACCEL_XOUT_H, gc_mpu, 6, false)){ // Read accelerometer output register
				LOG_E("mpu_read_sensor(): failed to read 6 bytes from reg ACCEL_XOUT_H (0x%02X)", MPU_REG_ACCEL_XOUT_H);
				return false;
			}

			g_mpu->v.accel.raw.x = (gc_mpu[0]  << 8) | gc_mpu[1]; // Save raw accelerometer x axis
			g_mpu->v.accel.raw.y = (gc_mpu[2]  << 8) | gc_mpu[3]; // Save raw accelerometer y axis
			g_mpu->v.accel.raw.z = (gc_mpu[4]  << 8) | gc_mpu[5]; // Save raw accelerometer z axis

			LOG_D("mpu_read_sensor(): accel: x=%d, y=%d, z=%d;",
					g_mpu->v.accel.raw.x, g_mpu->v.accel.raw.y, g_mpu->v.accel.raw.z);
		}
		if(mask & MPU_TEMP){ // Only temperatur
			if(!mpu_read_register(MPU_REG_TEMP_OUT_H, gc_mpu, 2, false)){ // Reads temperatur output register
				LOG_E("mpu_read_sensor(): failed to read 2 bytes from reg TEMP_OUT_H (0x%02X)", MPU_REG_TEMP_OUT_H);
				return false;
			}

			g_mpu->v.temp.raw = (gc_mpu[0]  << 8) | gc_mpu[1]; // Save raw temperatur

			LOG_D("mpu_read_sensor(): temp=%d;", g_mpu->v.temp.raw);
		}
		if(mask & MPU_GYRO){ // Only gyroscope
			if(!mpu_read_register(MPU_REG_GYRO_XOUT_H, gc_mpu, 6, false)){ // Read gyro output register
				LOG_E("mpu_read_sensor(): failed to read 6 bytes from reg GYRO_XOUT_H (0x%02X)", MPU_REG_GYRO_XOUT_H);
				return false;
			}

			g_mpu->v.gyro.raw.x = (gc_mpu[0]  << 8) | gc_mpu[1]; // Save raw gyro x axis
			g_mpu->v.gyro.raw.y = (gc_mpu[2] << 8) | gc_mpu[3]; // Save raw gyro y axis
			g_mpu->v.gyro.raw.z = (gc_mpu[4] << 8) | gc_mpu[5]; // Save raw gyro z axis

			LOG_D("mpu_read_sensor(): gyro: x=%d, y=%d, z=%d;",
					g_mpu->v.gyro.raw.x, g_mpu->v.gyro.raw.y, g_mpu->v.gyro.raw.z);
		}
	}

	if(sensor & MPU_SCALED){ // Optional: scale raw values
		if(mask & MPU_ACCEL){ // Raw -> G for accelerometer
			g_mpu->v.accel.g.x = (g_mpu->v.accel.raw.x - g_mpu->conf.offset_accel.x) / g_mpu->conf.fsr_div.accel; // Calculate raw x axis to G
			g_mpu->v.accel.g.y = (g_mpu->v.accel.raw.y - g_mpu->conf.offset_accel.y) / g_mpu->conf.fsr_div.accel; // Calculate raw y axis to G
			g_mpu->v.accel.g.z = (g_mpu->v.accel.raw.z - g_mpu->conf.offset_accel.z) / g_mpu->conf.fsr_div.accel; // Calculate raw z axis to G

			LOG_D("mpu_read_sensor(): scaled accel: x=%0.3fg, y=%0.3fg, z=%0.3fg;",
					g_mpu->v.accel.g.x, g_mpu->v.accel.g.y, g_mpu->v.accel.g.z);
		}
		if(mask & MPU_TEMP){ // Raw -> °C
			g_mpu->v.temp.celsius = (g_mpu->v.temp.raw / 340.0f) + 36.53f; // Calculate raw temperatur to °C

			LOG_D("mpu_read_sensor(): scaled temp: %02.2f°C;", g_mpu->v.temp.celsius);
		}

		if(mask & MPU_GYRO){ // Raw -> °/s for gyroscope
			g_mpu->v.gyro.dps.x = (g_mpu->v.gyro.raw.x - g_mpu->conf.offset_gyro.x) / g_mpu->conf.fsr_div.gyro; // Calculate raw x axis to °/s
			g_mpu->v.gyro.dps.y = (g_mpu->v.gyro.raw.y - g_mpu->conf.offset_gyro.y) / g_mpu->conf.fsr_div.gyro; // Calculate raw y axis to °/s
			g_mpu->v.gyro.dps.z = (g_mpu->v.gyro.raw.z - g_mpu->conf.offset_gyro.z) / g_mpu->conf.fsr_div.gyro; // Calculate raw z axis to °/s

			LOG_D("mpu_read_sensor(): scaled gyro: x=%0.3f°/s, y=%0.3f°/s, z=%0.3f°/s;",
					g_mpu->v.gyro.dps.x, g_mpu->v.gyro.dps.y, g_mpu->v.gyro.dps.z);
		}
	}

	return true;
}

#if MPU_INT_PIN // Checks if MPU_INT_PIN is greater than 0
volatile bool g_mpu_int_flag; // True if an interrupt occurred, else false

// ======================================
// ========= Interrupt Handler ==========
// ======================================
// * Gets called if the MPU_INT_PIN 
// * goes high. Sets `g_mpu_int_flag`
// * true if high.
// ======================================
/*
 * @parameter:
 * 	- gpio = the pin who got the interrupt
 * 	- events = the called interrupt event
 */
void mpu_irq_handler(uint gpio, uint32_t events){
    if(gpio == MPU_INT_PIN){   // Checks if the called pin is MPU_INT_PIN
        g_mpu_int_flag = true; // if it is set `g_mpu_int_flag` true.
    }
}

// ==========================================
// ====== Interrupt Pin configuration =======
// ==========================================
// * Configure what the inerrupt pin of the *
// * MPU schould do.                        *
// * The function takes the bitmask given   *
// * as `cfg` and writes it to the          *
// * INT_PIN_CFG register.                  *
// ==========================================
/*
 * @parameter:
 * 	- cfg = bitmask for the INT_PIN_CFG register
 *
 * @return:
 * 	- true = everything is ok
 * 	- false = failed to write to the INT_PIN_CFG register
 */
bool mpu_int_pin_cfg(mpu_int_pin_cfg_t cfg){
	if(!mpu_read_register(MPU_REG_INT_PIN_CFG, gc_mpu, 1, true)){ // Reads the INT_PIN_CFG register and save it in gc_mpu
			LOG_E("mpu_int_pin_cfg(): failed to read 1 byte from reg INT_PIN_CFG (0x%02X)", MPU_REG_INT_PIN_CFG);
			return false;
		}

	gc_mpu[0] &= ~MPU_INT_PIN_CFG_ALL; // Unsets all interrupt bits
	gc_mpu[0] |= cfg; // Set the bits given in `cfg`

	if(!mpu_write_register((uint8_t[]){MPU_REG_INT_PIN_CFG, gc_mpu[0]}, 2, false)){ // Write back to registers
		LOG_E("mpu_int_pin_cfg(): failed to write 0x%02X to reg INT_PIN_CFG (0x%02X)", gc_mpu[0], MPU_REG_INT_PIN_CFG);
		return false;
	}

	sleep_ms(2); // Little activation pause

	LOG_I("mpu_int_pin_cfg(): interrupt pin configured. written 0x%02X to reg INT_PIN_CFG (0x%02X)", cfg, MPU_REG_INT_PIN_CFG);

	return true; // If everything goes right
}

// =============================================
// ====== Motion Interrupt configuration =======
// =============================================
// * Prepare for motion interrupts.            *
// * The function takes how many milli seconds *
// * the sensor has to move and with how much  *
// * milli gram of force it has to move.       *
// * And writes the info in the INT_MOTION_CFG *
// * register.                                 *
// =============================================
/*
 * @parameter:
 * 	- ms = milli sedonds to move 1-255
 * 	- mg = milli gram of force 1-255 * 32
 *
 * @return:
 * 	- true = everything ok could write cfg
 * 	- false = error failed to write cfg
 */
bool mpu_int_motion_cfg(uint8_t ms, uint16_t mg){
	if(ms < 1)         ms = 1;   // Check if argument `ms` are to small
	else if (ms > 255) ms = 255; // or to big and set it to min/max

	if(mg < 32)        mg = 1;   // Check if `mg` is to small
	else if(mg > 8160) mg = 255; // or to big and set to min/max
	else               mg /= 32; // else divide by 32 for the mpu

	if(!mpu_ahpf(MPU_AHPF_5HZ)){ // Set the accel high pass filter to 5Hz
		LOG_E("mpu_int_motion_cfg(): failed to set ahpf to 5Hz");
		return false;
	}
	if(!mpu_write_register((uint8_t[]){MPU_REG_MOT_THR, mg, ms}, 3, false)){ // Write the motion threashold to the given arguments
		LOG_E("mpu_int_motion_cfg(): failed to write 0x%02X, 0x%02X to reg MOT_THR (0x%02X)", mg, ms, MPU_REG_MOT_THR);
		return false;
	}

	sleep_ms(2); // Little activation pause

	LOG_I("mpu_int_motion_cfg(): motion int configured. written 0x%02X, 0x%02X to reg MOT_THR (0x%02X)", mg, ms, MPU_REG_MOT_THR);

	return true; // If everything is ok
}

// ==============================================================
// ==================== Interrupt pin enable ====================
// ==============================================================
// * Set the interrupt handler callback to `mpu_irq_handler`.   *
// * Reads the INT_ENABLE register, unsets the bits for         *
// * the interrupts then sets bit in the INT_ENABLE register,   *
// * with the bitmask given as the argument `interrupt`.        *
// ==============================================================
/*
 * @parameter:
 * 	- interrupt = takes a bitmask for the INT_STATUS register
 *
 * @return:
 * 	- true = when it could write to the INT_STATUS register
 * 	- false = if anything goes wrong
 */
bool mpu_int_enable(mpu_int_enable_t interrupt){
	gpio_set_irq_enabled_with_callback(MPU_INT_PIN, GPIO_IRQ_EDGE_RISE, true, &mpu_irq_handler); // Listen MPU_INT_PIN call `mpu_irq_handler` if pin HIGH
	LOG_I("mpu_int_enable(): set IRQ with rising edge on GPIO%d", MPU_INT_PIN);

	if(!mpu_read_register(MPU_REG_INT_ENABLE, gc_mpu, 1, true)){ // Read the INT_ENABLE register
			LOG_E("mpu_int_enable(): failed to read 1 byte from reg INT_ENABLE (0x%02X)", MPU_REG_INT_ENABLE);
			return false;
		}

	gc_mpu[0] &= ~MPU_INT_ENABLE_ALL; // Unsets all interrupt bits
	gc_mpu[0] |= interrupt; // Sets with the bitmask given by argument

	if(!mpu_write_register((uint8_t[]){MPU_REG_INT_ENABLE, gc_mpu[0]}, 2, false)){ // Write back to registers
		LOG_E("mpu_int_enable(): failed to write 0x%02X to reg INT_ENABLE (0x%02X)", gc_mpu[0], MPU_REG_INT_ENABLE);
		return false;
	}

	sleep_ms(2); // Little activation pause
	
	LOG_I("mpu_int_enable(): interrupt activated");

	return true; // When nothing goes wrong
}

// ================================================
// ============ Read interrupt status =============
// ================================================
// * If a interrupt at MPU_INT_PIN is detected,   *
// * this function reads the interrupt status     *
// * register and returns true when any interrupt *
// * bit is 1 other wise false.                   *
// ================================================
/*
 * @return:
 * 	- true = an interrupt has occurred
 * 	- false = no interrupt
 */
bool mpu_int_status(void){
	if(!g_mpu_int_flag) return false; // Checks if an interrupt occurred at MPU_INT_PIN
	else g_mpu_int_flag = false; // When an interrupt has occurred set the flag false

	if(!mpu_read_register(MPU_REG_INT_STATUS, gc_mpu, 1, false)){ // Read INT_STATUS register save output in gc_mpu else return false
			LOG_E("mpu_int_status(): failed to read 1 byte from reg INT_STATUS (0x%02X)", MPU_REG_INT_STATUS);
			return false;
		}

	LOG_D("mpu_int_status(): interrupt = "ANSI_BOLD ANSI_GREEN"true"ANSI_RESET);

	if((gc_mpu[0] & MPU_DATA_RDY_INT) || // Check data ready interrupt
	   (gc_mpu[0] & MPU_I2C_MST_INT)  || // Check I²C master interrupt
	   (gc_mpu[0] & MPU_MOTION_INT)   || // Check motion interrupt
	   (gc_mpu[0] & MPU_FIFO_OFLOW_INT)) return true; // Check fifo overflow interrupt and return true if any was set
	else return false;
}
#endif
