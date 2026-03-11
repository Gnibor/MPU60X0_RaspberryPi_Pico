/*
 * ================================================================
 *  Project:      MPU-60X0 Driver for Raspberry Pi Pico
 *  File:         MPU60X0_reg_map.h
 *  Author:       (Gnibor) Robin Gerhartz
 *  License:      MIT License
 *  Repository:   https://github.com/Gnibor/MPU60X0_RaspberryPi_Pico
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
 *  Public API for the MPU-60X0 driver.
 *
 * ================================================================
 */
#ifndef _MPU_REG_MAP_H_
#define _MPU_REG_MAP_H_

#define MPU_REG_SELF_TEST_X		0x0D
#define MPU_XG_TEST			(0x1F)
#define MPU_XA_TEST			(0xE0)

#define MPU_REG_SELF_TEST_Y		0x0E
#define MPU_YG_TEST			(0x1F)
#define MPU_YA_TEST			(0xE0)

#define MPU_REG_SELF_TEST_Z		0x0F
#define MPU_ZG_TEST			(0x1F)
#define MPU_ZA_TEST			(0xE0)

#define MPU_REG_SELF_TEST_A		0x10
#define MPU_ZA_TEST_A			(3 << 0)
#define MPU_YA_TEST_A			(3 << 2)
#define MPU_XA_TEST_A			(3 << 4)

#define MPU_REG_XA_OFFS_H        0x06
#define MPU_REG_XA_OFFS_L        0x07
#define MPU_REG_YA_OFFS_H        0x08
#define MPU_REG_YA_OFFS_L        0x09
#define MPU_REG_ZA_OFFS_H        0x0A
#define MPU_REG_ZA_OFFS_L        0x0B

#define MPU_REG_XG_OFFS_H        0x13
#define MPU_REG_XG_OFFS_L        0x14
#define MPU_REG_YG_OFFS_H        0x15
#define MPU_REG_YG_OFFS_L        0x16
#define MPU_REG_ZG_OFFS_H        0x17
#define MPU_REG_ZG_OFFS_L        0x18

#define MPU_REG_SMPLRT_DIV		0x19
typedef enum {
	MPU_SMPLRT_8KHZ =		(0 << 0),
	MPU_SMPLRT_1KHZ =		(0x7 << 0),
	MPU_SMPLRT_500HZ =		(0xE << 0),
	MPU_SMPLRT_200HZ =		(0x27 << 0),   // 39 decimal = 0x27
	MPU_SMPLRT_100HZ =		(0x5C << 0)
} mpu_smplrt_div_t;

#define MPU_REG_CONFIG			0x1A
typedef enum {
	MPU_DLPF_CFG_260HZ =		(0 << 0),  // DLPF off, BW=260Hz, Fs=8kHz Gyro / 1kHz Accel
	MPU_DLPF_CFG_184HZ =		(1 << 0),
	MPU_DLPF_CFG_94HZ =		(2 << 0),
	MPU_DLPF_CFG_44HZ =		(3 << 0),
	MPU_DLPF_CFG_21HZ =		(4 << 0),
	MPU_DLPF_CFG_10HZ =		(5 << 0),
	MPU_DLPF_CFG_5HZ =		(6 << 0),
	MPU_DLPF_CFG_3600HZ =		(7 << 0)   // Gyro only
} mpu_dlpf_cfg_t;

typedef enum {
	MPU_EXT_SYNC_DISABLED =		(0 << 3),
	MPU_EXT_SYNC_TEMP_OUT =		(1 << 3),
	MPU_EXT_SYNC_XG =		(2 << 3),
	MPU_EXT_SYNC_YG =		(3 << 3),
	MPU_EXT_SYNC_ZG =		(4 << 3),
	MPU_EXT_SYNC_ACCEL_X =		(5 << 3),
	MPU_EXT_SYNC_ACCEL_Y =		(6 << 3),
	MPU_EXT_SYNC_ACCEL_Z =		(7 << 3)
} mpu_ext_sync_set_t;

#define MPU_REG_GYRO_CONFIG		0x1B
typedef enum {
	MPU_FSR_250DPS =		(0 << 3),
	MPU_FSR_500DPS =		(1 << 3),
	MPU_FSR_1000DPS =		(2 << 3),
	MPU_FSR_2000DPS =		(3 << 3)
} mpu_fsr_t;


#define MPU_REG_ACCEL_CONFIG		0x1C
typedef enum {
	MPU_AHPF_RESET  =		(0 << 0), // Filter zurücksetzen
	MPU_AHPF_5HZ    =		(1 << 0), // Hochpassfilter 5Hz (Empfohlen für Motion Detection)
	MPU_AHPF_2_5HZ  =		(2 << 0),
	MPU_AHPF_1_25HZ =		(3 << 0),
	MPU_AHPF_0_63HZ = 		(4 << 0),
	MPU_AHPF_HOLD   = 		(7 << 0)  // Filterwert halten
} mpu_ahpf_t;
typedef enum {
	MPU_AFSR_2G =			(0 << 3),
	MPU_AFSR_4G =			(1 << 3),
	MPU_AFSR_8G =			(2 << 3),
	MPU_AFSR_16G =			(3 << 3)
} mpu_afsr_t;

#define MPU_ZA_ST			(1 << 5)
#define MPU_YA_ST			(1 << 6)
#define MPU_XA_ST			(1 << 7)
#define MPU_REG_FF_THR           0x1D
#define MPU_REG_FF_DUR           0x1E
#define MPU_REG_MOT_THR			0x1F
#define MPU_REG_MOT_DUR			0x20
#define MPU_REG_ZRMOT_THR        0x21
#define MPU_REG_ZRMOT_DUR        0x22
#define MPU_REG_FIFO_EN			0x23
#define MPU_SLV0_FIFO_EN		(1 << 0)
#define MPU_SLV1_FIFO_EN		(1 << 1)
#define MPU_SLV2_FIFO_EN		(1 << 2)
#define MPU_ACCEL_FIFO_EN		(1 << 3)
#define MPU_ZG_FIFO_EN			(1 << 4)
#define MPU_YG_FIFO_EN			(1 << 5)
#define MPU_XG_FIFO_EN			(1 << 6)
#define MPU_TEMP_FIFO_EN		(1 << 7)

#define MPU_REG_I2C_MST_CTRL		0x24
#define MPU_I2C_MST_CLK_DIV_23		(0 << 0)
#define MPU_I2C_MST_CLK_DIV_24		(1 << 0)
#define MPU_I2C_MST_CLK_DIV_25		(2 << 0)
#define MPU_I2C_MST_CLK_DIV_26		(3 << 0)
#define MPU_I2C_MST_CLK_DIV_27		(4 << 0)
#define MPU_I2C_MST_CLK_DIV_28		(5 << 0)
#define MPU_I2C_MST_CLK_DIV_29		(6 << 0)
#define MPU_I2C_MST_CLK_DIV_30		(7 << 0)
#define MPU_I2C_MST_CLK_DIV_31		(8 << 0)
#define MPU_I2C_MST_CLK_DIV_16		(9 << 0)
#define MPU_I2C_MST_CLK_DIV_17		(10 << 0)
#define MPU_I2C_MST_CLK_DIV_18		(11 << 0)
#define MPU_I2C_MST_CLK_DIV_19		(12 << 0)
#define MPU_I2C_MST_CLK_DIV_20		(13 << 0)
#define MPU_I2C_MST_CLK_DIV_21		(14 << 0)
#define MPU_I2C_MST_CLK_DIV_22		(15 << 0)
#define MPU_I2C_MST_P_NSR		(1 << 4)
#define MPU_SLV_3_FIFO_EN		(1 << 5)
#define MPU_WAIT_FOR_ES			(1 << 6)
#define MPU_MULTI_MST_EN		(1 << 7)

#define MPU_REG_I2C_SLV0_ADDR		0x25
#define MPU_I2C_SLV0_ADDR		(0x7F)
#define MPU_I2C_SLV0_RW			(1 << 7)

#define MPU_REG_I2C_SLV0_REG		0x26

#define MPU_REG_I2C_SLV0_CTRL		0x27
#define MPU_I2C_SLV0_LEN		(7 << 0)
#define MPU_I2C_SLV0_GRP		(1 << 4)
#define MPU_I2C_SLV0_REG_DIS		(1 << 5)
#define MPU_I2C_SLV0_BYTE_SW		(1 << 6)
#define MPU_I2C_SLV0_EN			(1 << 7)

#define MPU_REG_I2C_SLV1_ADDR		0x28
#define MPU_I2C_SLV1_ADDR		(0x7F)
#define MPU_I2C_SLV1_RW			(1 << 7)

#define MPU_REG_I2C_SLV1_REG		0x29

#define MPU_REG_I2C_SLV1_CTRL		0x2A
#define MPU_I2C_SLV1_LEN		(7 << 0)
#define MPU_I2C_SLV1_GRP		(1 << 4)
#define MPU_I2C_SLV1_REG_DIS		(1 << 5)
#define MPU_I2C_SLV1_BYTE_SW		(1 << 6)
#define MPU_I2C_SLV1_EN			(1 << 7)

#define MPU_REG_I2C_SLV2_ADDR		0x2B
#define MPU_I2C_SLV2_ADDR		(0x7F)
#define MPU_I2C_SLV2_RW			(1 << 7)

#define MPU_REG_I2C_SLV2_REG		0x2C

#define MPU_REG_I2C_SLV2_CTRL		0x2D
#define MPU_I2C_SLV2_LEN		(0x0F << 0)
#define MPU_I2C_SLV2_GRP		(1 << 4)
#define MPU_I2C_SLV2_REG_DIS		(1 << 5)
#define MPU_I2C_SLV2_BYTE_SW		(1 << 6)
#define MPU_I2C_SLV2_EN			(1 << 7)

#define MPU_REG_I2C_SLV3_ADDR		0x2E
#define MPU_I2C_SLV3_ADDR		(0x7F)
#define MPU_I2C_SLV3_RW			(1 << 7)

#define MPU_REG_I2C_SLV3_REG		0x2F

#define MPU_REG_I2C_SLV3_CTRL		0x30
#define MPU_I2C_SLV3_LEN		(0xF << 0)
#define MPU_I2C_SLV3_GRP		(1 << 4)
#define MPU_I2C_SLV3_REG_DIS		(1 << 5)
#define MPU_I2C_SLV3_BYTE_SW		(1 << 6)
#define MPU_I2C_SLV3_EN			(1 << 7)

#define MPU_REG_I2C_SLV4_ADDR		0x31
#define MPU_I2C_SLV2_ADDR		(0x7F)
#define MPU_I2C_SLV2_RW			(1 << 7)

#define MPU_REG_I2C_SLV4_REG		0x32
#define MPU_REG_I2C_SLV4_DO		0x33

#define MPU_REG_I2C_SLV4_CTRL		0x34
#define MPU_I2C_MST_DLY			(0xF << 0)
#define MPU_I2C_SLV4_REG_DIS		(1 << 5)
#define MPU_I2C_SLV4_INT_EN		(1 << 6)
#define MPU_I2C_SLV4_EN			(1 << 7)

#define MPU_REG_I2C_SLV4_DI		0x35

#define MPU_REG_I2C_MST_STATUS		0x36
#define MPU_I2C_SLV0_NACK		(1 << 0)
#define MPU_I2C_SLV1_NACK		(1 << 1)
#define MPU_I2C_SLV2_NACK		(1 << 2)
#define MPU_I2C_SLV3_NACK		(1 << 3)
#define MPU_I2C_SLV4_NACK		(1 << 4)
#define MPU_I2C_LOST_ARB		(1 << 5)
#define MPU_I2C_SLV4_DONE		(1 << 6)
#define MPU_PASS_THROUGH		(1 << 7)

#define MPU_REG_INT_PIN_CFG		0x37
typedef enum {
    MPU_I2C_BYPASS_EN   =		(1 << 1),
    MPU_FSYNC_INT_EN    =		(1 << 2),
    MPU_FSYNC_INT_LEVEL =		(1 << 3),
    MPU_INT_RD_CLEAR    =		(1 << 4),
    MPU_LATCH_INT_EN    =		(1 << 5),
    MPU_INT_OPEN_DRAIN  =		(1 << 6),
    MPU_INT_LEVEL_LOW   =		(1 << 7),

    MPU_INT_PIN_CFG_ALL =		0xFE  // alle Bits außer Bit 0 (falls reserviert)
} mpu_int_pin_cfg_t;

#define MPU_REG_INT_ENABLE		0x38
typedef enum{
	MPU_DATA_RDY_EN    =		(1 << 0),
	MPU_I2C_MST_INT_EN =		(1 << 3),
	MPU_FIFO_OFLOW_EN  =		(1 << 4),
	MPU_INT_MOTION_EN  =		(1 << 6),
	MPU_INT_ENABLE_ALL =		0x59
} mpu_int_enable_t;
#define MPU_REG_DMP_INT_STATUS		0x39 // DMP spezifischer Interrupt Status
#define MPU_REG_INT_STATUS		0x3A
#define MPU_DATA_RDY_INT		(1 << 0)
#define MPU_I2C_MST_INT			(1 << 3)
#define MPU_FIFO_OFLOW_INT		(1 << 4)
#define MPU_MOTION_INT			(1 << 6)

#define MPU_REG_ACCEL_XOUT_H		0x3B
#define MPU_REG_ACCEL_XOUT_l		0x3C
#define MPU_REG_ACCEL_YOUT_H		0x3D
#define MPU_REG_ACCEL_YOUT_l		0x3E
#define MPU_REG_ACCEL_ZOUT_H		0x3F
#define MPU_REG_ACCEL_ZOUT_l		0x40
#define MPU_REG_TEMP_OUT_H		0x41
#define MPU_REG_TEMP_OUT_L		0x42
#define MPU_REG_GYRO_XOUT_H		0x43
#define MPU_REG_GYRO_XOUT_l		0x44
#define MPU_REG_GYRO_YOUT_H		0x45
#define MPU_REG_GYRO_YOUT_l		0x46
#define MPU_REG_GYRO_ZOUT_H		0x47
#define MPU_REG_GYRO_ZOUT_l		0x48
#define MPU_REG_EXT_SENS_DATA_00	0x49
#define MPU_REG_EXT_SENS_DATA_01	0x4A
#define MPU_REG_EXT_SENS_DATA_02	0x4B
#define MPU_REG_EXT_SENS_DATA_03	0x4C
#define MPU_REG_EXT_SENS_DATA_04	0x4D
#define MPU_REG_EXT_SENS_DATA_05	0x4E
#define MPU_REG_EXT_SENS_DATA_06	0x4F
#define MPU_REG_EXT_SENS_DATA_07	0x50
#define MPU_REG_EXT_SENS_DATA_08	0x51
#define MPU_REG_EXT_SENS_DATA_09	0x52
#define MPU_REG_EXT_SENS_DATA_10	0x53
#define MPU_REG_EXT_SENS_DATA_11	0x54
#define MPU_REG_EXT_SENS_DATA_12	0x55
#define MPU_REG_EXT_SENS_DATA_13	0x56
#define MPU_REG_EXT_SENS_DATA_14	0x57
#define MPU_REG_EXT_SENS_DATA_15	0x58
#define MPU_REG_EXT_SENS_DATA_16	0x59
#define MPU_REG_EXT_SENS_DATA_17	0x5A
#define MPU_REG_EXT_SENS_DATA_18	0x5B
#define MPU_REG_EXT_SENS_DATA_19	0x5C
#define MPU_REG_EXT_SENS_DATA_20	0x5D
#define MPU_REG_EXT_SENS_DATA_21	0x5E
#define MPU_REG_EXT_SENS_DATA_22	0x5F
#define MPU_REG_EXT_SENS_DATA_23	0x60
#define MPU_REG_I2C_SLV0_DO		0x63
#define MPU_REG_I2C_SLV1_DO		0x64
#define MPU_REG_I2C_SLV2_DO		0x65
#define MPU_REG_I2C_SLV3_DO		0x66

#define MPU_REG_I2C_MST_DELAY_CTRL	0x67
#define MPU_I2C_SLV0_DLY_EN		(1 << 0)
#define MPU_I2C_SLV1_DLY_EN		(1 << 1)
#define MPU_I2C_SLV2_DLY_EN		(1 << 2)
#define MPU_I2C_SLV3_DLY_EN		(1 << 3)
#define MPU_I2C_SLV4_DLY_EN		(1 << 4)
#define MPU_DELAY_ES_SHADOW		(1 << 7)

#define MPU_REG_SIGNAL_PATH_RESET	0x68
#define MPU_TEMP_RESET			(1 << 0)
#define MPU_ACCEL_RESET			(1 << 1)
#define MPU_GYRO_RESET			(1 << 2)

#define MPU_REG_MOT_DETECT_CTRL		0x69 // Motion Detection Zähler & Delay
typedef enum {
    MOT_COUNT_RESET   = 0x00, // Sofortiger Reset wenn Bewegung stoppt
    MOT_COUNT_DEC_1   = 0x01, // Zähler sinkt langsam (weniger nervös)
    MOT_COUNT_DEC_2   = 0x02,
    MOT_COUNT_DEC_4   = 0x03,
    ACCEL_ON_DELAY  = (0x01 << 4), // Standard Startup Delay
} mpu_mot_count_t;

typedef enum {
    MOT_DELAY_0MS     = 0x00, // Einschwingzeit des Filters
    MOT_DELAY_1MS     = 0x10,
    MOT_DELAY_2MS     = 0x20,
    MOT_DELAY_3MS     = 0x30
} mpu_mot_delay_t;

#define MPU_REG_USER_CTRL		0x6A
#define MPU_SIG_COND_RESET		(1 << 0)
#define MPU_I2C_MST_RESET		(1 << 1)
#define MPU_FIFO_RESET			(1 << 2)
#define MPU_I2C_IF_DIS			(1 << 4)
#define MPU_I2C_MST_EN			(1 << 5)
#define MPU_FIFO_EN			(1 << 6)

#define MPU_REG_PWR_MGMT_1		0x6B
typedef enum {
	MPU_CLK_INTERNAL =		(0 << 0),
	MPU_CLK_XGYRO =			(1 << 0),
	MPU_CLK_YGYRO =			(2 << 0),
	MPU_CLK_ZGYRO =			(3 << 0),
	MPU_CLK_EXT32KHZ =		(4 << 0),
	MPU_CLK_EXT19MHZ =		(5 << 0),
	MPU_CLK_STOP =			(7 << 0)
} mpu_clk_sel_t;
#define MPU_TEMP_DIS			(1 << 3)
#define MPU_CYCLE			(1 << 5)
#define MPU_SLEEP			(1 << 6)
#define MPU_DEVICE_RESET		(1 << 7)


#define MPU_REG_PWR_MGMT_2		0x6C
typedef enum{
	MPU_STBY_ZG =			(1 << 0),
	MPU_STBY_YG =			(1 << 1),
	MPU_STBY_XG =			(1 << 2),
	MPU_STBY_GYRO =			(7 << 0),
	MPU_STBY_ACCEL =		(7 << 3),
	MPU_STBY_ALL =			(0x3F)
} mpu_stby_t;
typedef enum {
	MPU_LP_WAKE_1_25HZ =		(0 << 6),
	MPU_LP_WAKE_5HZ  =		(1 << 6),
	MPU_LP_WAKE_20HZ =		(2 << 6),
	MPU_LP_WAKE_40HZ =		(3 << 6)
} mpu_lp_wake_t;

#define MPU_REG_FIFO_COUNTH		0x72
#define MPU_REG_FIFO_COUNTL		0x73
#define MPU_REG_FIFO_R_W		0x74

#define MPU_REG_WHO_AM_I		0x75
#define MPU_WHO_AM_I			0x68

#endif
