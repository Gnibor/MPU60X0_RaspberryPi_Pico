/*
 * ================================================================
 *  Project:      GY-521 (MPU-6050) Driver Example for RP2040
 *  File:         main.c
 *  Author:       (Gnibor) Robin Gerhartz
 *  License:      MIT License
 *  Repository:   https://github.com/Gnibor/mpu60x0_rp2040
 * ================================================================
 *
 *  Description:
 *  Example application demonstrating how to initialize,
 *  configure and read data from the GY-521 (MPU-6050)
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
#include "pico/stdlib.h"
#include <hardware/gpio.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>

#include "default.h"
#include "mpu60x0.h"
#include "MPU-60X0_reg_map.h"

int main(void){
	stdio_init_board();
	mpu_s mpu = mpu_init(i2c1, MPU60X0_I2C_ADDR_GND);
	mpu_use_struct(&mpu);

	if(mpu_device_reset()) printf("__!Device resetted!__\n");
	sleep_ms(50);
	int retries = 3;
	bool connected = false;
	printf("Try connecting GY-521...\n");
	while(retries--){
		connected = mpu_who_am_i();
		if(connected) break;

		printf("Retrying...\n");
		sleep_ms(750);
	}
	if(!connected) printf("GY-521 not found!\n");
	else printf("GY-521 ready!\n");

	if(!mpu_sleep(MPU_SLEEP_ALL_OFF)) printf("sleep did not get deactivated!!!\n");
	else printf("sleep is deactivated!\n");
	sleep_ms(10);

	if(!mpu_fsr(MPU60X0_FSR_500DPS, MPU60X0_AFSR_2G)) printf("Could not set the SFR/AFSR\n");
	else printf("FSR=2000dps, AFSR=8g\n");

	printf("Try to calibrate GY-521\n");
	sleep_ms(2000);
	if(mpu_calibrate_gyro(10)) printf("GY-521 gyro is now calibrated.\n");
	else printf("GY-521 gyro could not be calibrated.\n");

	mpu_dlpf_cfg(MPU60X0_DLPF_CFG_260HZ);

	printf("how big is the struct: %dbytes\n", sizeof(mpu));

	//if(!mpu.fn.stby(MPU60X0_STBY_YG)) printf("Could not set stand-by for YA!!!\n");
	//else printf("YA is now stand-by!\n");

	// INT Pin configuration in the MPU60X0
	/*mpu_int_pin_cfg(
		MPU60X0_INT_LEVEL_LOW  | // 1 = Level, 0 = pulse
		MPU60X0_INT_OPEN_DRAIN | // 1 = Push-Pull, 0 = Open-Drain
		MPU60X0_LATCH_INT_EN   | // Latch Interrupt active
		MPU60X0_INT_RD_CLEAR     // Interrupt cleared by reading the fn.interrupt.status()
	);*/

	if(mpu_cycle_mode(MPU_CYCLE_ON, MPU60X0_SMPLRT_1KHZ)) printf("Enable Cycle mode!!!\n");
	else printf("Could not enable Cycle mode!!!\n");
	sleep_ms(10);

	// Data ready interrupt activate
	//mpu_int_enable(MPU_DATA_RDY_EN);
	//gpio_set_irq_enabled_with_callback(MPU_INT_PIN, GPIO_IRQ_EDGE_RISE, true, &mpu_irq_handler);

	while(1){
		//if(g_mpu_int_flag) printf("Hello from the Interrupt Flag!!!!!!!!!!!!!!!!\n");
		if(mpu_int_status()){
			if(mpu_read_sensor(MPU_ALL | MPU_SCALED))
				printf("G=X:%6.3f Y:%6.3f Z:%6.3f | °C=%6.2f | °/s=X:%9.3f Y:%9.3f Z:%9.3f\n", 
					mpu.v.accel.g.x, mpu.v.accel.g.y, mpu.v.accel.g.z, 
					mpu.v.temp.celsius, 
					mpu.v.gyro.dps.x, mpu.v.gyro.dps.y, mpu.v.gyro.dps.z);
		}
		sleep_ms(20);
	}
}
