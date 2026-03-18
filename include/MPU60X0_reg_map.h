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
#ifndef _MPU60X0_REG_MAP_H_
#define _MPU60X0_REG_MAP_H_

/* --- Self Test Registers --- */
#define MPU_REG_SELF_TEST_X        0x0D // Self-test response for X-axis
#define MPU_XG_TEST                (0x1F) // Gyro X-axis self-test exponent
#define MPU_XA_TEST                (0xE0) // Accel X-axis self-test exponent

#define MPU_REG_SELF_TEST_Y        0x0E // Self-test response for Y-axis
#define MPU_YG_TEST                (0x1F) // Gyro Y-axis self-test exponent
#define MPU_YA_TEST                (0xE0) // Accel Y-axis self-test exponent

#define MPU_REG_SELF_TEST_Z        0x0F // Self-test response for Z-axis
#define MPU_ZG_TEST                (0x1F) // Gyro Z-axis self-test exponent
#define MPU_ZA_TEST                (0xE0) // Accel Z-axis self-test exponent

#define MPU_REG_SELF_TEST_A        0x10 // Self-test response for Accelerometer
#define MPU_ZA_TEST_A              (3 << 0) // Accel Z-axis self-test 2-bit field
#define MPU_YA_TEST_A              (3 << 2) // Accel Y-axis self-test 2-bit field
#define MPU_XA_TEST_A              (3 << 4) // Accel X-axis self-test 2-bit field

/* --- Offset Registers (Factory Trim) --- */
#define MPU_REG_XA_OFFS_H          0x06 // Accel X-axis offset High byte
#define MPU_REG_XA_OFFS_L          0x07 // Accel X-axis offset Low byte
#define MPU_REG_YA_OFFS_H          0x08 // Accel Y-axis offset High byte
#define MPU_REG_YA_OFFS_L          0x09 // Accel Y-axis offset Low byte
#define MPU_REG_ZA_OFFS_H          0x0A // Accel Z-axis offset High byte
#define MPU_REG_ZA_OFFS_L          0x0B // Accel Z-axis offset Low byte

#define MPU_REG_XG_OFFS_H          0x13 // Gyro X-axis offset High byte
#define MPU_REG_XG_OFFS_L          0x14 // Gyro X-axis offset Low byte
#define MPU_REG_YG_OFFS_H          0x15 // Gyro Y-axis offset High byte
#define MPU_REG_YG_OFFS_L          0x16 // Gyro Y-axis offset Low byte
#define MPU_REG_ZG_OFFS_H          0x17 // Gyro Z-axis offset High byte
#define MPU_REG_ZG_OFFS_L          0x18 // Gyro Z-axis offset Low byte

/* --- Configuration Registers --- */
#define MPU_REG_SMPLRT_DIV         0x19 // Sample rate divider (Divides Gyro output)
/** @brief Sample rate divider presets. Formula: Rate = Gyro_Rate / (1 + div). */
typedef enum {
    MPU_SMPLRT_8KHZ  = (0 << 0),   /**< 8kHz output (DLPF must be disabled/7). */
    MPU_SMPLRT_1KHZ  = (0x7 << 0),  /**< 1kHz output. */
    MPU_SMPLRT_500HZ = (0xE << 0),  /**< 500Hz output. */
    MPU_SMPLRT_200HZ = (0x27 << 0), /**< 200Hz output. */
    MPU_SMPLRT_100HZ = (0x5C << 0)  /**< 100Hz output. */
} mpu_smplrt_div_t;

#define MPU_REG_CONFIG             0x1A // General configuration (DLPF & Sync)
/** @brief Digital Low Pass Filter (DLPF) bandwidth configurations. */
typedef enum {
    MPU_DLPF_CFG_260HZ  = (0 << 0), /**< 260Hz bandwidth, 0.6ms delay. */
    MPU_DLPF_CFG_184HZ  = (1 << 0), /**< 184Hz bandwidth, 2.0ms delay. */
    MPU_DLPF_CFG_94HZ   = (2 << 0), /**< 94Hz bandwidth, 3.0ms delay. */
    MPU_DLPF_CFG_44HZ   = (3 << 0), /**< 44Hz bandwidth, 4.9ms delay. */
    MPU_DLPF_CFG_21HZ   = (4 << 0), /**< 21Hz bandwidth, 8.5ms delay. */
    MPU_DLPF_CFG_10HZ   = (5 << 0), /**< 10Hz bandwidth, 13.8ms delay. */
    MPU_DLPF_CFG_5HZ    = (6 << 0), /**< 5Hz bandwidth, 19.0ms delay. */
    MPU_DLPF_CFG_3600HZ = (7 << 0)  /**< No filter (8kHz gyro, 1kHz accel). */
} mpu_dlpf_cfg_t;

/** @brief External Frame Sync (FSYNC) configuration for @ref mpu_fsync_config. */
typedef enum {
    MPU_EXT_SYNC_DISABLED = (0 << 3), /**< FSYNC pin function disabled. */
    MPU_EXT_SYNC_TEMP_OUT = (1 << 3), /**< FSYNC bit stored in TEMP_OUT_L[0]. */
    MPU_EXT_SYNC_XG       = (2 << 3), /**< FSYNC bit stored in GYRO_XOUT_L[0]. */
    MPU_EXT_SYNC_YG       = (3 << 3), /**< FSYNC bit stored in GYRO_YOUT_L[0]. */
    MPU_EXT_SYNC_ZG       = (4 << 3), /**< FSYNC bit stored in GYRO_ZOUT_L[0]. */
    MPU_EXT_SYNC_ACCEL_X  = (5 << 3), /**< FSYNC bit stored in ACCEL_XOUT_L[0]. */
    MPU_EXT_SYNC_ACCEL_Y  = (6 << 3), /**< FSYNC bit stored in ACCEL_YOUT_L[0]. */
    MPU_EXT_SYNC_ACCEL_Z  = (7 << 3)  /**< FSYNC bit stored in ACCEL_ZOUT_L[0]. */
} mpu_ext_sync_set_t;

#define MPU_REG_GYRO_CONFIG        0x1B // Gyroscope configuration (Full scale range)
/** @brief Gyroscope full scale range (FSR) settings. */
typedef enum {
    MPU_FSR_250DPS  = (0 << 3), /**< +/- 250 deg/s range. */
    MPU_FSR_500DPS  = (1 << 3), /**< +/- 500 deg/s range. */
    MPU_FSR_1000DPS = (2 << 3), /**< +/- 1000 deg/s range. */
    MPU_FSR_2000DPS = (3 << 3)  /**< +/- 2000 deg/s range. */
} mpu_fsr_t;

#define MPU_REG_ACCEL_CONFIG       0x1C // Accelerometer configuration (FSR & HPF)
/**
 * @brief Accelerometer High Pass Filter (AHPF) settings (Register 0x1C).
 * 
 * The filter removes the DC offset (like gravity) from the accelerometer 
 * data. This is essential for motion detection.
 */
typedef enum {
    MPU_AHPF_RESET  = (0 << 0), /**< Reset the high pass filter. */
    MPU_AHPF_5HZ    = (1 << 0), /**< Filter cutoff at 5Hz. */
    MPU_AHPF_2_5HZ  = (2 << 0), /**< Filter cutoff at 2.5Hz. */
    MPU_AHPF_1_25HZ = (3 << 0), /**< Filter cutoff at 1.25Hz. */
    MPU_AHPF_0_63HZ = (4 << 0), /**< Filter cutoff at 0.63Hz. */
    MPU_AHPF_HOLD   = (7 << 0)  /**< Freeze the filter at its current value. */
} mpu_ahpf_t;

/** @brief Accelerometer full scale range (AFSR) settings. */
typedef enum {
    MPU_AFSR_2G  = (0 << 3), /**< +/- 2g range. */
    MPU_AFSR_4G  = (1 << 3), /**< +/- 4g range. */
    MPU_AFSR_8G  = (2 << 3), /**< +/- 8g range. */
    MPU_AFSR_16G = (3 << 3)  /**< +/- 16g range. */
} mpu_afsr_t;

/* --- Threshold & Duration Registers --- */
#define MPU_ZA_ST                  (1 << 5) // Accel Z self-test trigger bit
#define MPU_YA_ST                  (1 << 6) // Accel Y self-test trigger bit
#define MPU_XA_ST                  (1 << 7) // Accel X self-test trigger bit
#define MPU_REG_FF_THR             0x1D // Free-fall threshold
#define MPU_REG_FF_DUR             0x1E // Free-fall duration
#define MPU_REG_MOT_THR            0x1F // Motion detection threshold
#define MPU_REG_MOT_DUR            0x20 // Motion detection duration
#define MPU_REG_ZRMOT_THR          0x21 // Zero-motion detection threshold
#define MPU_REG_ZRMOT_DUR          0x22 // Zero-motion detection duration

/* --- FIFO Enable Register --- */
#define MPU_REG_FIFO_EN            0x23 // FIFO buffer enable control
#define MPU_SLV0_FIFO_EN           (1 << 0) // Enable SLV0 data to FIFO
#define MPU_SLV1_FIFO_EN           (1 << 1) // Enable SLV1 data to FIFO
#define MPU_SLV2_FIFO_EN           (1 << 2) // Enable SLV2 data to FIFO
#define MPU_ACCEL_FIFO_EN          (1 << 3) // Enable Accelerometer data to FIFO
#define MPU_ZG_FIFO_EN             (1 << 4) // Enable Gyro Z data to FIFO
#define MPU_YG_FIFO_EN             (1 << 5) // Enable Gyro Y data to FIFO
#define MPU_XG_FIFO_EN             (1 << 6) // Enable Gyro X data to FIFO
#define MPU_TEMP_FIFO_EN           (1 << 7) // Enable Temperature data to FIFO

/* --- I2C Master Control --- */
#define MPU_REG_I2C_MST_CTRL       0x24 // I2C Master clock and logic control
#define MPU_I2C_MST_CLK_DIV_23     (0 << 0) // Clock 23 -> 348kHz
#define MPU_I2C_MST_CLK_DIV_24     (1 << 0) // Clock 24 -> 333kHz
#define MPU_I2C_MST_CLK_DIV_25     (2 << 0) // Clock 25 -> 320kHz
#define MPU_I2C_MST_CLK_DIV_26     (3 << 0) // Clock 26 -> 308kHz
#define MPU_I2C_MST_CLK_DIV_27     (4 << 0) // Clock 27 -> 296kHz
#define MPU_I2C_MST_CLK_DIV_28     (5 << 0) // Clock 28 -> 286kHz
#define MPU_I2C_MST_CLK_DIV_29     (6 << 0) // Clock 29 -> 276kHz
#define MPU_I2C_MST_CLK_DIV_30     (7 << 0) // Clock 30 -> 267kHz
#define MPU_I2C_MST_CLK_DIV_31     (8 << 0) // Clock 31 -> 258kHz
#define MPU_I2C_MST_CLK_DIV_16     (9 << 0) // Clock 16 -> 500kHz
#define MPU_I2C_MST_CLK_DIV_17     (10 << 0) // Clock 17 -> 471kHz
#define MPU_I2C_MST_CLK_DIV_18     (11 << 0) // Clock 18 -> 444kHz
#define MPU_I2C_MST_CLK_DIV_19     (12 << 0) // Clock 19 -> 421kHz
#define MPU_I2C_MST_CLK_DIV_20     (13 << 0) // Clock 20 -> 400kHz
#define MPU_I2C_MST_CLK_DIV_21     (14 << 0) // Clock 21 -> 381kHz
#define MPU_I2C_MST_CLK_DIV_22     (15 << 0) // Clock 22 -> 364kHz
#define MPU_I2C_MST_P_NSR          (1 << 4) // Stop/Start between slave reads
#define MPU_SLV_3_FIFO_EN          (1 << 5) // Slave 3 FIFO write enable
#define MPU_WAIT_FOR_ES            (1 << 6) // Wait for external sensor data
#define MPU_MULTI_MST_EN           (1 << 7) // Enable multi-master mode

/* --- I2C Slave 0 Control --- */
#define MPU_REG_I2C_SLV0_ADDR      0x25 // Slave 0 I2C address
#define MPU_I2C_SLV0_ADDR          (0x7F) // Address mask
#define MPU_I2C_SLV0_RW            (1 << 7) // Read/Write flag

#define MPU_REG_I2C_SLV0_REG       0x26 // Slave 0 register address
#define MPU_REG_I2C_SLV0_CTRL      0x27 // Slave 0 control bits
#define MPU_I2C_SLV0_LEN           (7 << 0) // Number of bytes to read
#define MPU_I2C_SLV0_GRP           (1 << 4) // External word grouping
#define MPU_I2C_SLV0_REG_DIS       (1 << 5) // Disable register access
#define MPU_I2C_SLV0_BYTE_SW       (1 << 6) // Swap bytes (Little/Big Endian)
#define MPU_I2C_SLV0_EN            (1 << 7) // Enable Slave 0

/* --- Slave 1 to 3 Registers (Repeat structure of SLV0) --- */
#define MPU_REG_I2C_SLV1_ADDR      0x28
#define MPU_I2C_SLV1_RW            (1 << 7)
#define MPU_REG_I2C_SLV1_REG       0x29
#define MPU_REG_I2C_SLV1_CTRL      0x2A
#define MPU_I2C_SLV1_EN            (1 << 7)

#define MPU_REG_I2C_SLV2_ADDR      0x2B
#define MPU_I2C_SLV2_RW            (1 << 7)
#define MPU_REG_I2C_SLV2_REG       0x2C
#define MPU_REG_I2C_SLV2_CTRL      0x2D
#define MPU_I2C_SLV2_EN            (1 << 7)

#define MPU_REG_I2C_SLV3_ADDR      0x2E
#define MPU_I2C_SLV3_RW            (1 << 7)
#define MPU_REG_I2C_SLV3_REG       0x2F
#define MPU_REG_I2C_SLV3_CTRL      0x30
#define MPU_I2C_SLV3_EN            (1 << 7)

/* --- I2C Slave 4 (Indirect Master access) --- */
#define MPU_REG_I2C_SLV4_ADDR      0x31 // Slave 4 address
#define MPU_REG_I2C_SLV4_REG       0x32 // Slave 4 register
#define MPU_REG_I2C_SLV4_DO        0x33 // Slave 4 Data Output
#define MPU_REG_I2C_SLV4_CTRL      0x34 // Slave 4 control
#define MPU_I2C_MST_DLY            (0xF << 0) // Master sample delay
#define MPU_I2C_SLV4_INT_EN        (1 << 6) // Interrupt enable for SLV4
#define MPU_I2C_SLV4_EN            (1 << 7) // Enable Slave 4
#define MPU_REG_I2C_SLV4_DI        0x35 // Slave 4 Data Input

/* --- Status Registers --- */
#define MPU_REG_I2C_MST_STATUS     0x36 // I2C Master status register
#define MPU_I2C_SLV0_NACK          (1 << 0) // Slave 0 NACK received
#define MPU_I2C_SLV1_NACK          (1 << 1) // Slave 1 NACK received
#define MPU_I2C_SLV2_NACK          (1 << 2) // Slave 2 NACK received
#define MPU_I2C_SLV3_NACK          (1 << 3) // Slave 3 NACK received
#define MPU_I2C_SLV4_NACK          (1 << 4) // Slave 4 NACK received
#define MPU_I2C_LOST_ARB           (1 << 5) // Lost arbitration on I2C bus
#define MPU_I2C_SLV4_DONE          (1 << 6) // Slave 4 transfer complete
#define MPU_PASS_THROUGH           (1 << 7) // FSYNC pin status

/* --- Interrupt Configuration --- */
#define MPU_REG_INT_PIN_CFG        0x37 // Interrupt pin/Bypass configuration
/**
 * @brief Configuration flags for the INT/FSYNC pin behavior (Register 0x37).
 */
typedef enum {
    MPU_I2C_BYPASS_EN   = (1 << 1), /**< Direct I2C access for secondary sensors on Aux bus. */
    MPU_FSYNC_INT_EN    = (1 << 2), /**< Enable FSYNC pin to trigger an interrupt. */
    MPU_FSYNC_INT_LEVEL = (1 << 3), /**< FSYNC logic level (0=Active High, 1=Active Low). */
    MPU_INT_RD_CLEAR    = (1 << 4), /**< Interrupt status is cleared on any read operation. */
    MPU_LATCH_INT_EN    = (1 << 5), /**< Keep INT pin held until interrupt status is read. */
    MPU_INT_OPEN_DRAIN  = (1 << 6), /**< Set INT pin to open drain (default is push-pull). */
    MPU_INT_LEVEL_LOW   = (1 << 7), /**< Set INT pin active level to Low (default is High). */
    MPU_INT_PIN_CFG_ALL = 0xFE      /**< Mask for all configuration bits in this register. */
} mpu_int_pin_cfg_t;

#define MPU_REG_INT_ENABLE         0x38 // Interrupt enable register
/**
 * @brief Available interrupt sources for the MPU_INT_ENABLE register (0x38).
 * 
 * These flags can be ORed together to enable multiple interrupt sources 
 * simultaneously via @ref mpu_int_enable().
 */
typedef enum {
    MPU_DATA_RDY_EN    = (1 << 0), /**< Trigger on new data sample.*/
    MPU_I2C_MST_INT_EN = (1 << 3), /**< Trigger on I2C Master external transaction finish. */
    MPU_FIFO_OFLOW_EN  = (1 << 4), /**< Trigger on FIFO buffer overflow. */
    MPU_INT_MOTION_EN  = (1 << 6), /**< Trigger on motion detection. */
    MPU_INT_ENABLE_ALL = 0x59      /**< Mask for common interrupt bits. */
} mpu_int_enable_t;

#define MPU_REG_DMP_INT_STATUS     0x39 // Digital Motion Processor status
#define MPU_REG_INT_STATUS         0x3A // General interrupt status
#define MPU_DATA_RDY_INT           (1 << 0) // Data ready occurred
#define MPU_I2C_MST_INT            (1 << 3) // Master I2C event occurred
#define MPU_FIFO_OFLOW_INT         (1 << 4) // FIFO overflow occurred
#define MPU_MOTION_INT             (1 << 6) // Motion detected

/* --- Sensor Data Output (Read-only) --- */
#define MPU_REG_ACCEL_XOUT_H       0x3B // Accel X High byte
#define MPU_REG_ACCEL_XOUT_L       0x3C // Accel X Low byte
#define MPU_REG_ACCEL_YOUT_H       0x3D // Accel Y High byte
#define MPU_REG_ACCEL_YOUT_L       0x3E // Accel Y Low byte
#define MPU_REG_ACCEL_ZOUT_H       0x3F // Accel Z High byte
#define MPU_REG_ACCEL_ZOUT_L       0x40 // Accel Z Low byte

#define MPU_REG_TEMP_OUT_H         0x41 // Temp High byte
#define MPU_REG_TEMP_OUT_L         0x42 // Temp Low byte

#define MPU_REG_GYRO_XOUT_H        0x43 // Gyro X High byte
#define MPU_REG_GYRO_XOUT_L        0x44 // Gyro X Low byte
#define MPU_REG_GYRO_YOUT_H        0x45 // Gyro Y High byte
#define MPU_REG_GYRO_YOUT_L        0x46 // Gyro Y Low byte
#define MPU_REG_GYRO_ZOUT_H        0x47 // Gyro Z High byte
#define MPU_REG_GYRO_ZOUT_L        0x48 // Gyro Z Low byte

/* --- External Sensor Data (Slave results) --- */
#define MPU_REG_EXT_SENS_DATA_00   0x49 // Start of external sensor data block
#define MPU_REG_EXT_SENS_DATA_01   0x4A
#define MPU_REG_EXT_SENS_DATA_02   0x4B
#define MPU_REG_EXT_SENS_DATA_03   0x4C
#define MPU_REG_EXT_SENS_DATA_04   0x4D
#define MPU_REG_EXT_SENS_DATA_05   0x4E
#define MPU_REG_EXT_SENS_DATA_06   0x4F
#define MPU_REG_EXT_SENS_DATA_07   0x50
#define MPU_REG_EXT_SENS_DATA_08   0x51
#define MPU_REG_EXT_SENS_DATA_09   0x52
#define MPU_REG_EXT_SENS_DATA_10   0x53
#define MPU_REG_EXT_SENS_DATA_11   0x54
#define MPU_REG_EXT_SENS_DATA_12   0x55
#define MPU_REG_EXT_SENS_DATA_13   0x56
#define MPU_REG_EXT_SENS_DATA_14   0x57
#define MPU_REG_EXT_SENS_DATA_15   0x58
#define MPU_REG_EXT_SENS_DATA_16   0x59
#define MPU_REG_EXT_SENS_DATA_17   0x5A
#define MPU_REG_EXT_SENS_DATA_18   0x5B
#define MPU_REG_EXT_SENS_DATA_19   0x5C
#define MPU_REG_EXT_SENS_DATA_20   0x5D
#define MPU_REG_EXT_SENS_DATA_21   0x5E
#define MPU_REG_EXT_SENS_DATA_22   0x5F
#define MPU_REG_EXT_SENS_DATA_23   0x60 // End of external sensor data block

/* --- Slave Data Output (Writing to slaves) --- */
#define MPU_REG_I2C_SLV0_DO        0x63 // Data out for Slave 0
#define MPU_REG_I2C_SLV1_DO        0x64 // Data out for Slave 1
#define MPU_REG_I2C_SLV2_DO        0x65 // Data out for Slave 2
#define MPU_REG_I2C_SLV3_DO        0x66 // Data out for Slave 3

/* --- I2C Master Delay Control --- */
#define MPU_REG_I2C_MST_DELAY_CTRL 0x67 // Slave sample rate delay control
#define MPU_I2C_SLV0_DLY_EN        (1 << 0) // Enable delay for Slave 0
#define MPU_I2C_SLV4_DLY_EN        (1 << 4) // Enable delay for Slave 4
#define MPU_DELAY_ES_SHADOW        (1 << 7) // Delay shadow register update

/* --- Path Resets --- */
#define MPU_REG_SIGNAL_PATH_RESET  0x68 // Reset digital signal paths
#define MPU_TEMP_RESET             (1 << 0) // Reset temp path
#define MPU_ACCEL_RESET            (1 << 1) // Reset accel path
#define MPU_GYRO_RESET             (1 << 2) // Reset gyro path

/* --- Motion Detection Control --- */
#define MPU_REG_MOT_DETECT_CTRL    0x69 // Motion detection counter and delay
/** @brief Motion detection hardware counter decrement settings. */
typedef enum {
    MOT_COUNT_RESET   = 0x00, /**< Reset counter to 0 immediately. */
    MOT_COUNT_DEC_1   = 0x01, /**< Decrement counter by 1. */
    MOT_COUNT_DEC_2   = 0x02, /**< Decrement counter by 2. */
    MOT_COUNT_DEC_4   = 0x03, /**< Decrement counter by 4. */
    ACCEL_ON_DELAY    = (1 << 4) /**< Add delay before enabling accel. */
} mpu_mot_count_t;

/**
 * @brief Power-on delay settings for motion detection (Register 0x69).
 */
typedef enum {
    MOT_DELAY_0MS     = (0 << 4), /**< 0ms delay before motion detection starts. */
    MOT_DELAY_1MS     = (1 << 4), /**< 1ms power-on delay. */
    MOT_DELAY_2MS     = (2 << 4), /**< 2ms power-on delay. */
    MOT_DELAY_3MS     = (3 << 4)  /**< 3ms power-on delay. */
} mpu_mot_delay_t;

/* --- User Control Register --- */
#define MPU_REG_USER_CTRL          0x6A // Main user control
#define MPU_SIG_COND_RESET         (1 << 0) // Reset sensor signal conditions
#define MPU_I2C_MST_RESET          (1 << 1) // Reset I2C Master logic
#define MPU_FIFO_RESET             (1 << 2) // Reset FIFO buffer
#define MPU_I2C_IF_DIS             (1 << 4) // Disable I2C and use SPI
#define MPU_I2C_MST_EN             (1 << 5) // Enable I2C Master mode
#define MPU_FIFO_EN_BIT            (1 << 6) // Enable FIFO operations

/* --- Power Management 1 --- */
#define MPU_REG_PWR_MGMT_1         0x6B // Power and clock source control
/** @brief Clock source selection for timing and sensor sampling. */
typedef enum {
    MPU_CLK_INTERNAL = (0 << 0), /**< Internal 8MHz oscillator. */
    MPU_CLK_XGYRO    = (1 << 0), /**< PLL with X-Gyro reference. */
    MPU_CLK_YGYRO    = (2 << 0), /**< PLL with Y-Gyro reference. */
    MPU_CLK_ZGYRO    = (3 << 0), /**< PLL with Z-Gyro reference. */
    MPU_CLK_EXT32KHZ = (4 << 0), /**< External 32.768kHz clock. */
    MPU_CLK_EXT19MHZ = (5 << 0), /**< External 19.2MHz clock. */
    MPU_CLK_STOP     = (7 << 0)  /**< Stop clock / reset timing. */
} mpu_clk_sel_t;

#define MPU_TEMP_DIS               (1 << 3) // Disable temperature sensor
#define MPU_CYCLE                  (1 << 5) // Enable cycle mode (low power)
#define MPU_SLEEP                  (1 << 6) // Put device into sleep mode
#define MPU_DEVICE_RESET           (1 << 7) // Trigger full device reset

/* --- Power Management 2 --- */
#define MPU_REG_PWR_MGMT_2         0x6C // Individual axis standby and wake-up
/** @brief Standby settings for individual sensor axes (Register 0x6C). */
typedef enum {
    MPU_STBY_ZG    = (1 << 0), /**< Put Z-Gyro in standby. */
    MPU_STBY_YG    = (1 << 1), /**< Put Y-Gyro in standby. */
    MPU_STBY_XG    = (1 << 2), /**< Put X-Gyro in standby. */
    MPU_STBY_GYRO  = (7 << 0), /**< Put all Gyro axes in standby. */
    MPU_STBY_ACCEL = (7 << 3), /**< Put all Accel axes in standby. */
    MPU_STBY_ALL   = 0x3F      /**< Put all 6 axes in standby. */
} mpu_stby_t;

/** @brief Wake-up frequencies for Low Power Accelerometer mode. */
typedef enum {
    MPU_LP_WAKE_1_25HZ = (0 << 6), /**< Wake-up at 1.25Hz. */
    MPU_LP_WAKE_5HZ    = (1 << 6), /**< Wake-up at 5Hz. */
    MPU_LP_WAKE_20HZ   = (2 << 6), /**< Wake-up at 20Hz. */
    MPU_LP_WAKE_40HZ   = (3 << 6)  /**< Wake-up at 40Hz. */
} mpu_lp_wake_t;

/* --- FIFO Buffer Status & Access --- */
#define MPU_REG_FIFO_COUNTH        0x72 // Number of bytes in FIFO High byte
#define MPU_REG_FIFO_COUNTL        0x73 // Number of bytes in FIFO Low byte
#define MPU_REG_FIFO_R_W           0x74 // FIFO read/write portal

/* --- Identification --- */
#define MPU_REG_WHO_AM_I           0x75 // Device identification register
#define MPU60X0_WHO_AM_I           0x68 // Default chip ID
#define MPU6500_WHO_AM_I           0x70 // Default chip ID
#define MPU9250_WHO_AM_I           0x71 // Default chip ID
#define MPU9255_WHO_AM_I           0x73 // Default chip ID

#endif
