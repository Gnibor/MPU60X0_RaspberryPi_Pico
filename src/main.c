/*
 * ================================================================
 *  Project:      GY-521 (MPU-6050) Driver Example for RP2040
 *  File:         main.c
 *  Author:       (Gnibor) Robin Gerhartz
 *  License:      MIT License
 *  Repository:   https://github.com/Gnibor/gy521_rp2040
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
 *  This file is meant as a usage example for the gy521 driver.
 *
 * ================================================================
 */
#include "pico/stdlib.h"
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>

#include "default.h"
#include "gy521.h"
#include "MPU-60X0_reg_map.h"

int main(void){
	stdio_init_board();
	gy521_s gy521 = gy521_init(i2c1, GY521_I2C_ADDR_GND);
	gy521_use(&gy521);

	if(gy521.fn.device_reset()) printf("__!Device resetted!__\n");
	sleep_ms(50);
	int retries = 3;
	bool connected = false;
	printf("Try connecting GY-521...\n");
	while(retries--){
		connected = gy521.fn.who_am_i();
		if(connected) break;

		printf("Retrying...\n");
		sleep_ms(750);
	}
	if(!connected) printf("GY-521 not found!\n");
	else printf("GY-521 ready!\n");

	if(!gy521.fn.sleep(false,false)) printf("sleep did not get deactivated!!!\n");
	else printf("sleep is deactivated!\n");
	sleep_ms(10);

	if(!gy521.fn.fsr(GY521_FSR_500DPS, GY521_AFSR_2G)) printf("Could not set the SFR/AFSR\n");
	else printf("FSR=2000dps, AFSR=8g\n");

	printf("Try to calibrate GY-521\n");
	sleep_ms(2000);
	if(gy521.fn.gyro_calibrate(10)) printf("GY-521 gyro is now calibrated.\n");
	else printf("GY-521 gyro could not be calibrated.\n");


	printf("how big is the struct: %dbytes\n", sizeof(gy521));

	//if(!gy521.fn.stby(GY521_STBY_YG)) printf("Could not set stand-by for YA!!!\n");
	//else printf("YA is now stand-by!\n");

	// INT Pin configuration in the GY521
	/*gy521.fn.interrupt.pin_cfg(
		GY521_INT_LEVEL_LOW  | // 1 = Level, 0 = pulse
		GY521_INT_OPEN_DRAIN | // 1 = Push-Pull, 0 = Open-Drain
		GY521_LATCH_INT_EN   | // Latch Interrupt active
		GY521_INT_RD_CLEAR     // Interrupt cleared by reading the fn.interrupt.status()
	);*/

	if(gy521.fn.cycle(GY521_CYCLE_LP, GY521_LP_WAKE_1_25HZ)) printf("Enable Cycle mode!!!\n");
	//else printf("Could not enable Cycle mode!!!\n");
	sleep_ms(10);

	// Data ready interrupt activate
	//gy521.fn.interrupt.enable(GY521_DATA_RDY_EN);

	while(1){
	//	if(gy521.fn.interrupt.status()){
			if(gy521.fn.read_sensor(GY521_ACCEL | GY521_SCALED))
				printf("G=X:%6.3f Y:%6.3f Z:%6.3f | °C=%6.2f | °/s=X:%9.3f Y:%9.3f Z:%9.3f\n", 
					gy521.v.accel.g.x, gy521.v.accel.g.y, gy521.v.accel.g.z, 
					gy521.v.temp.celsius, 
					gy521.v.gyro.dps.x, gy521.v.gyro.dps.y, gy521.v.gyro.dps.z);
	//	}
		sleep_ms(400);
	}
}
