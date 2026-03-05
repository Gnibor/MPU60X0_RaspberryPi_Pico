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
#include "MPU-60X0_reg_map.h"
#include "pico/stdlib.h"
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>

#include "default.h"
#include "gy521.h"

volatile bool mpu_irq_flag = false;

void gy521_irq_handler(uint gpio, uint32_t events){
    if(gpio == GY521_INT_PIN){
        mpu_irq_flag = true;
    }
}

int main(void){
	stdio_init_board();
	gy521_s gy521 = gy521_init(i2c1, GY521_I2C_ADDR_GND);
	gy521_use(&gy521);

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
/*	gy521.opt.reset.device = true;
	gy521.opt.scaled = true;
	gy521.opt.sleep.device = false;
	gy521.opt.fsr.accel.g8 = true;
	gy521.opt.fsr.gyro.dps2000 = true;
	gy521.opt.clksel.gyro.x = true;
*/

	//gy521.conf.gyro.y.stby = true;
	//if(gy521.fn.stby()) printf("YG in standby\n");
	//gy521.conf.temp.sleep = true;
	//if(gy521.fn.sleep()) printf("temp in standby\n");

	gy521.fn.fsr(GY521_FSR_2000DPS, GY521_AFSR_8G);

	printf("Try to calibrate GY-521\n");
	sleep_ms(2000);
	if(gy521.fn.gyro.calibrate(10)) printf("GY-521 is now calibrated.\n");
	else printf("GY-521 could not be calibrated.\n");

	printf("how big is it?: %d\n", sizeof(gy521));

	// INT Pin Konfiguration im MPU
    gy521.fn.interrupt.pin_cfg(
        GY521_INT_LEVEL_LOW | // Pegel statt Puls
        GY521_INT_OPEN_DRAIN   | // Push-Pull / Open-Drain (je nach Definition: hier Push-Pull)
        GY521_LATCH_INT_EN    | // Latch Interrupt aktiv
        GY521_INT_RD_CLEAR // Interrupt beim Lesen zurücksetzen
    );

    // Data Ready Interrupt aktivieren
    gy521.fn.interrupt.enable(GY521_DATA_RDY_INT);

gpio_set_irq_enabled_with_callback(
        GY521_INT_PIN,
        GPIO_IRQ_EDGE_RISE,
        true,
        &gy521_irq_handler
    );

	while(1){
		if(gy521.fn.interrupt.status())
			if(gy521.v.interrupt.data_rdy)
				if(gy521.fn.read_sensor(GY521_ALL | GY521_SCALED))
					printf("G=X:%6.3f Y:%6.3f Z:%6.3f | °C=%6.2f | °/s=X:%9.3f Y:%9.3f Z:%9.3f\n", 
						gy521.v.accel.g.x, gy521.v.accel.g.y, gy521.v.accel.g.z, 
						gy521.v.temp.celsius, 
						gy521.v.gyro.dps.x, gy521.v.gyro.dps.y, gy521.v.gyro.dps.z);
		sleep_ms(100);
	}
}
