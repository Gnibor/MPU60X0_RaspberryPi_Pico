/*
 * ================================================================
 *  Project:      MPU-60X0 Driver Example for RP2040
 *  File:         main.c
 *  Author:       (Gnibor) Robin Gerhartz
 *  License:      MIT License
 *  Repository:   https://github.com/Gnibor/mpu60x0_rp2040
 * ================================================================
 *
 *  Description:
 *  Example application demonstrating how to initialize,
 *  configure and read data from the MPU-60X0
 *  accelerometer + gyroscope module using the RP2040.
 *
 *  Features demonstrated:
 *  - I²C initialization
 *  - Device detection (WHO_AM_I check)
 *  - Device reset and wake-up
 *  - Full-scale range configuration
 *  - Clock source selection
 *  - Axis standby control
 *  - Gyroscope calibration
 *  - Continuous sensor readout (scaled output)
 *
 *  This file is meant as a usage example for the mpu60x0 driver.
 *
 * ================================================================
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "mpu60x0.h"
#include "MPU60X0_reg_map.h"

int main(void){
	stdio_usb_init();
	while(!stdio_usb_connected()) sleep_ms(100);
	
	//mpu_s mpu = mpu_init(MPU_I2C_PORT, MPU_ADDR_AD0_GND);
	mpu_s mpu = mpu_init(MPU_I2C_PORT, MPU_ADDR_AD0_GND);
	mpu_use_struct(&mpu);

	//mpu_reset(MPU_RESET_SIG_COND);

	if(mpu_reset(MPU_RESET_ALL)) printf("__!Device resetted!__\n");
	sleep_ms(50);

	//if(mpu_clk_sel(MPU_CLK_XGYRO)) printf("CLK_SEL is set to the internal 8Mhz clock!!!\n");

	if(mpu_fsr(MPU_FSR_250DPS, MPU_AFSR_2G)) printf("FSR=2000dps, AFSR=8g\n");
	printf("Try to calibrate GY-521\n");
	sleep_ms(2000);
	if(mpu_calibrate((MPU_ACCEL_X | MPU_GYRO), 10)) printf("GY-521 gyro and accel is now calibrated.\n");
	//mpu_dlpf_cfg(MPU_DLPF_CFG_184HZ);

	printf("how big is the struct: %dbytes\n", sizeof(mpu));

	//if(!mpu_stby(MPU60X0_STBY_YG)) printf("Could not set stand-by for YA!!!\n");
	//else printf("YA is now stand-by!\n");

	// INT Pin configuration in the MPU60X0
	mpu_int_pin_cfg(
		//MPU_INT_LEVEL_LOW  | // 1 = Level, 0 = pulse
		//MPU_INT_OPEN_DRAIN | // 1 = Push-Pull, 0 = Open-Drain
		MPU_LATCH_INT_EN   | // Latch Interrupt active
		MPU_INT_RD_CLEAR     // Interrupt cleared by reading the mpu_int_status()
	);

	mpu_int_motion_cfg(1, 160);

	//if(mpu_cycle_mode(MPU_CYCLE_ON, MPU_LP_WAKE_5HZ)) printf("Enable Cycle mode!!!\n");
	sleep_ms(10);

	// Data ready interrupt activate
	mpu_int_enable(MPU_INT_MOTION_EN);

	while(1){
		if(mpu_int_status()){
			if(mpu_read_sensor(MPU_ALL | MPU_SCALED))
				printf("G=X:%6.3f Y:%6.3f Z:%6.3f | °C=%6.2f | °/s=X:%9.3f Y:%9.3f Z:%9.3f\n",
					mpu.v.accel.g.x, mpu.v.accel.g.y, mpu.v.accel.g.z,
					mpu.v.temp.celsius,
					mpu.v.gyro.dps.x, mpu.v.gyro.dps.y, mpu.v.gyro.dps.z);
		}
		sleep_ms(250);
	}
}
