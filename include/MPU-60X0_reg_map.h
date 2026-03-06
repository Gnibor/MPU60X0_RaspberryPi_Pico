#ifndef _GY521_REG_MAP_H_
#define _GY521_REG_MAP_H_

#define GY521_REG_SELF_TEST_X		0x0D
#define GY521_XG_TEST			(0x1F)
#define GY521_XA_TEST			(0xE0)

#define GY521_REG_SELF_TEST_Y		0x0E
#define GY521_YG_TEST			(0x1F)
#define GY521_YA_TEST			(0xE0)

#define GY521_REG_SELF_TEST_Z		0x0F
#define GY521_ZG_TEST			(0x1F)
#define GY521_ZA_TEST			(0xE0)

#define GY521_REG_SELF_TEST_A		0x10
#define GY521_ZA_TEST_A			(3 << 0)
#define GY521_YA_TEST_A			(3 << 2)
#define GY521_XA_TEST_A			(3 << 4)

#define GY521_REG_SMPLRT_DIV		0x19
typedef enum {
	GY521_SMPLRT_8KHZ =		(0 << 0),
	GY521_SMPLRT_1KHZ =		(0x7 << 0),
	GY521_SMPLRT_200HZ =		(0x27 << 0)   // 39 decimal = 0x27
} gy521_smplrt_div_t;

#define GY521_REG_CONFIG		0x1A
typedef enum {
	GY521_DLPF_CFG_260HZ =		(0 << 0),  // DLPF off, BW=260Hz, Fs=8kHz Gyro / 1kHz Accel
	GY521_DLPF_CFG_184HZ =		(1 << 0),
	GY521_DLPF_CFG_94HZ =		(2 << 0),
	GY521_DLPF_CFG_44HZ =		(3 << 0),
	GY521_DLPF_CFG_21HZ =		(4 << 0),
	GY521_DLPF_CFG_10HZ =		(5 << 0),
	GY521_DLPF_CFG_5HZ =		(6 << 0),
	GY521_DLPF_CFG_3600HZ =		(7 << 0)   // Gyro only
} gy521_dlpf_cfg_t;

typedef enum {
	GY521_EXT_SYNC_DISABLED =	(0 << 3),
	GY521_EXT_SYNC_TEMP_OUT =	(1 << 3),
	GY521_EXT_SYNC_XG =		(2 << 3),
	GY521_EXT_SYNC_YG =		(3 << 3),
	GY521_EXT_SYNC_ZG =		(4 << 3),
	GY521_EXT_SYNC_ACCEL_X =	(5 << 3),
	GY521_EXT_SYNC_ACCEL_Y =	(6 << 3),
	GY521_EXT_SYNC_ACCEL_Z =	(7 << 3)
} gy521_ext_sync_set_t;

#define GY521_REG_GYRO_CONFIG		0x1B
typedef enum {
	GY521_FSR_250DPS =		(0 << 3),
	GY521_FSR_500DPS =		(1 << 3),
	GY521_FSR_1000DPS =		(2 << 3),
	GY521_FSR_2000DPS =		(3 << 3)
} gy521_fsr_t;


#define GY521_REG_ACCEL_CONFIG		0x1C
typedef enum {
	GY521_AFSR_2G =			(0 << 3),
	GY521_AFSR_4G =			(1 << 3),
	GY521_AFSR_8G =			(2 << 3),
	GY521_AFSR_16G =		(3 << 3)
} gy521_afsr_t;

#define GY521_ZA_ST			(1 << 5)
#define GY521_YA_ST			(1 << 6)
#define GY521_XA_ST			(1 << 7)

#define GY521_REG_FIFO_EN		0x23
#define GY521_SLV0_FIFO_EN		(1 << 0)
#define GY521_SLV1_FIFO_EN		(1 << 1)
#define GY521_SLV2_FIFO_EN		(1 << 2)
#define GY521_ACCEL_FIFO_EN		(1 << 3)
#define GY521_ZG_FIFO_EN		(1 << 4)
#define GY521_YG_FIFO_EN		(1 << 5)
#define GY521_XG_FIFO_EN		(1 << 6)
#define GY521_TEMP_FIFO_EN		(1 << 7)

#define GY521_REG_I2C_MST_CTRL		0x24
#define GY521_I2C_MST_CLK_DIV_23	(0 << 0)
#define GY521_I2C_MST_CLK_DIV_24	(1 << 0)
#define GY521_I2C_MST_CLK_DIV_25	(2 << 0)
#define GY521_I2C_MST_CLK_DIV_26	(3 << 0)
#define GY521_I2C_MST_CLK_DIV_27	(4 << 0)
#define GY521_I2C_MST_CLK_DIV_28	(5 << 0)
#define GY521_I2C_MST_CLK_DIV_29	(6 << 0)
#define GY521_I2C_MST_CLK_DIV_30	(7 << 0)
#define GY521_I2C_MST_CLK_DIV_31	(8 << 0)
#define GY521_I2C_MST_CLK_DIV_16	(9 << 0)
#define GY521_I2C_MST_CLK_DIV_17	(10 << 0)
#define GY521_I2C_MST_CLK_DIV_18	(11 << 0)
#define GY521_I2C_MST_CLK_DIV_19	(12 << 0)
#define GY521_I2C_MST_CLK_DIV_20	(13 << 0)
#define GY521_I2C_MST_CLK_DIV_21	(14 << 0)
#define GY521_I2C_MST_CLK_DIV_22	(15 << 0)
#define GY521_I2C_MST_P_NSR		(1 << 4)
#define GY521_SLV_3_FIFO_EN		(1 << 5)
#define GY521_WAIT_FOR_ES		(1 << 6)
#define GY521_MULTI_MST_EN		(1 << 7)

#define GY521_REG_I2C_SLV0_ADDR		0x25
#define GY521_I2C_SLV0_ADDR		(0x7F)
#define GY521_I2C_SLV0_RW		(1 << 7)

#define GY521_REG_I2C_SLV0_REG		0x26

#define GY521_REG_I2C_SLV0_CTRL		0x27
#define GY521_I2C_SLV0_LEN		(7 << 0)
#define GY521_I2C_SLV0_GRP		(1 << 4)
#define GY521_I2C_SLV0_REG_DIS		(1 << 5)
#define GY521_I2C_SLV0_BYTE_SW		(1 << 6)
#define GY521_I2C_SLV0_EN		(1 << 7)

#define GY521_REG_I2C_SLV1_ADDR		0x28
#define GY521_I2C_SLV1_ADDR		(0x7F)
#define GY521_I2C_SLV1_RW		(1 << 7)

#define GY521_REG_I2C_SLV1_REG		0x29

#define GY521_REG_I2C_SLV1_CTRL		0x2A
#define GY521_I2C_SLV1_LEN		(7 << 0)
#define GY521_I2C_SLV1_GRP		(1 << 4)
#define GY521_I2C_SLV1_REG_DIS		(1 << 5)
#define GY521_I2C_SLV1_BYTE_SW		(1 << 6)
#define GY521_I2C_SLV1_EN		(1 << 7)

#define GY521_REG_I2C_SLV2_ADDR		0x2B
#define GY521_I2C_SLV2_ADDR		(0x7F)
#define GY521_I2C_SLV2_RW		(1 << 7)

#define GY521_REG_I2C_SLV2_REG		0x2C

#define GY521_REG_I2C_SLV2_CTRL		0x2D
#define GY521_I2C_SLV2_LEN		(0x0F << 0)
#define GY521_I2C_SLV2_GRP		(1 << 4)
#define GY521_I2C_SLV2_REG_DIS		(1 << 5)
#define GY521_I2C_SLV2_BYTE_SW		(1 << 6)
#define GY521_I2C_SLV2_EN		(1 << 7)

#define GY521_REG_I2C_SLV3_ADDR		0x2E
#define GY521_I2C_SLV3_ADDR		(0x7F)
#define GY521_I2C_SLV3_RW		(1 << 7)

#define GY521_REG_I2C_SLV3_REG		0x2F

#define GY521_REG_I2C_SLV3_CTRL		0x30
#define GY521_I2C_SLV3_LEN		(0xF << 0)
#define GY521_I2C_SLV3_GRP		(1 << 4)
#define GY521_I2C_SLV3_REG_DIS		(1 << 5)
#define GY521_I2C_SLV3_BYTE_SW		(1 << 6)
#define GY521_I2C_SLV3_EN		(1 << 7)

#define GY521_REG_I2C_SLV4_ADDR		0x31
#define GY521_I2C_SLV2_ADDR		(0x7F)
#define GY521_I2C_SLV2_RW		(1 << 7)

#define GY521_REG_I2C_SLV4_REG		0x32
#define GY521_REG_I2C_SLV4_DO		0x33

#define GY521_REG_I2C_SLV4_CTRL		0x34
#define GY521_I2C_MST_DLY		(0xF << 0)
#define GY521_I2C_SLV4_REG_DIS		(1 << 5)
#define GY521_I2C_SLV4_INT_EN		(1 << 6)
#define GY521_I2C_SLV4_EN		(1 << 7)

#define GY521_REG_I2C_SLV4_DI		0x35

#define GY521_REG_I2C_MST_STATUS	0x36
#define GY521_I2C_SLV0_NACK		(1 << 0)
#define GY521_I2C_SLV1_NACK		(1 << 1)
#define GY521_I2C_SLV2_NACK		(1 << 2)
#define GY521_I2C_SLV3_NACK		(1 << 3)
#define GY521_I2C_SLV4_NACK		(1 << 4)
#define GY521_I2C_LOST_ARB		(1 << 5)
#define GY521_I2C_SLV4_DONE		(1 << 6)
#define GY521_PASS_THROUGH		(1 << 7)

#define GY521_REG_INT_PIN_CFG		0x37
typedef enum {
    GY521_I2C_BYPASS_EN   =		(1 << 1),
    GY521_FSYNC_INT_EN    =		(1 << 2),
    GY521_FSYNC_INT_LEVEL =		(1 << 3),
    GY521_INT_RD_CLEAR    =		(1 << 4),
    GY521_LATCH_INT_EN    =		(1 << 5),
    GY521_INT_OPEN_DRAIN  =		(1 << 6),
    GY521_INT_LEVEL_LOW   =		(1 << 7),

    GY521_INT_PIN_CFG_ALL =		0xFE  // alle Bits außer Bit 0 (falls reserviert)
} gy521_int_pin_cfg_t;

#define GY521_REG_INT_ENABLE		0x38
typedef enum{
	GY521_DATA_RDY_EN    =		(1 << 0),
	GY521_I2C_MST_INT_EN =		(1 << 3),
	GY521_FIFO_OFLOW_EN  =		(1 << 4),
	GY521_INT_ENABLE_ALL =		0x19
} gy521_int_enable_t;
#define GY521_REG_INT_STATUS		0x3A
#define GY521_DATA_RDY_INT		(1 << 0)
#define GY521_I2C_MST_INT		(1 << 3)
#define GY521_FIFO_OFLOW_INT		(1 << 4)

#define GY521_REG_ACCEL_XOUT_H		0x3B
#define GY521_REG_ACCEL_XOUT_l		0x3C
#define GY521_REG_ACCEL_YOUT_H		0x3D
#define GY521_REG_ACCEL_YOUT_l		0x3E
#define GY521_REG_ACCEL_ZOUT_H		0x3F
#define GY521_REG_ACCEL_ZOUT_l		0x40
#define GY521_REG_TEMP_OUT_H		0x41
#define GY521_REG_TEMP_OUT_L		0x42
#define GY521_REG_GYRO_XOUT_H		0x43
#define GY521_REG_GYRO_XOUT_l		0x44
#define GY521_REG_GYRO_YOUT_H		0x45
#define GY521_REG_GYRO_YOUT_l		0x46
#define GY521_REG_GYRO_ZOUT_H		0x47
#define GY521_REG_GYRO_ZOUT_l		0x48
#define GY521_REG_EXT_SENS_DATA_00	0x49
#define GY521_REG_EXT_SENS_DATA_01	0x4A
#define GY521_REG_EXT_SENS_DATA_02	0x4B
#define GY521_REG_EXT_SENS_DATA_03	0x4C
#define GY521_REG_EXT_SENS_DATA_04	0x4D
#define GY521_REG_EXT_SENS_DATA_05	0x4E
#define GY521_REG_EXT_SENS_DATA_06	0x4F
#define GY521_REG_EXT_SENS_DATA_07	0x50
#define GY521_REG_EXT_SENS_DATA_08	0x51
#define GY521_REG_EXT_SENS_DATA_09	0x52
#define GY521_REG_EXT_SENS_DATA_10	0x53
#define GY521_REG_EXT_SENS_DATA_11	0x54
#define GY521_REG_EXT_SENS_DATA_12	0x55
#define GY521_REG_EXT_SENS_DATA_13	0x56
#define GY521_REG_EXT_SENS_DATA_14	0x57
#define GY521_REG_EXT_SENS_DATA_15	0x58
#define GY521_REG_EXT_SENS_DATA_16	0x59
#define GY521_REG_EXT_SENS_DATA_17	0x5A
#define GY521_REG_EXT_SENS_DATA_18	0x5B
#define GY521_REG_EXT_SENS_DATA_19	0x5C
#define GY521_REG_EXT_SENS_DATA_20	0x5D
#define GY521_REG_EXT_SENS_DATA_21	0x5E
#define GY521_REG_EXT_SENS_DATA_22	0x5F
#define GY521_REG_EXT_SENS_DATA_23	0x60
#define GY521_REG_I2C_SLV0_DO		0x63
#define GY521_REG_I2C_SLV1_DO		0x64
#define GY521_REG_I2C_SLV2_DO		0x65
#define GY521_REG_I2C_SLV3_DO		0x66

#define GY521_REG_I2C_MST_DELAY_CTRL	0x67
#define GY521_I2C_SLV0_DLY_EN		(1 << 0)
#define GY521_I2C_SLV1_DLY_EN		(1 << 1)
#define GY521_I2C_SLV2_DLY_EN		(1 << 2)
#define GY521_I2C_SLV3_DLY_EN		(1 << 3)
#define GY521_I2C_SLV4_DLY_EN		(1 << 4)
#define GY521_DELAY_ES_SHADOW		(1 << 7)

#define GY521_REG_SIGNAL_PATH_RESET	0x68
#define GY521_TEMP_RESET		(1 << 0)
#define GY521_ACCEL_RESET		(1 << 1)
#define GY521_GYRO_RESET		(1 << 2)

#define GY521_REG_USER_CTRL		0x6A
#define GY521_SIG_COND_RESET		(1 << 0)
#define GY521_I2C_MST_RESET		(1 << 1)
#define GY521_FIFO_RESET		(1 << 2)
#define GY521_I2C_IF_DIS		(1 << 4)
#define GY521_I2C_MST_EN		(1 << 5)
#define GY521_FIFO_EN			(1 << 6)

#define GY521_REG_PWR_MGMT_1		0x6B
typedef enum {
	GY521_CLK_INTERNAL =		(0 << 0),
	GY521_CLK_XGYRO =		(1 << 0),
	GY521_CLK_YGYRO =		(2 << 0),
	GY521_CLK_ZGYRO =		(3 << 0),
	GY521_CLK_EXT32KHZ =		(4 << 0),
	GY521_CLK_EXT19MHZ =		(5 << 0),
	GY521_CLK_STOP =		(7 << 0)
} gy521_clk_sel_t;
#define GY521_TEMP_DIS			(1 << 3)
#define GY521_CYCLE			(1 << 5)
typedef enum{
	GY521_CYCLE_LP  =		2,
	GY521_CYCLE_ON  =		1,
	GY521_CYCLE_OFF =		0
} gy521_cycle_t;
#define GY521_SLEEP			(1 << 6)
#define GY521_DEVICE_RESET		(1 << 7)


#define GY521_REG_PWR_MGMT_2		0x6C
typedef enum{
	GY521_STBY_ZG =			(1 << 0),
	GY521_STBY_YG =			(1 << 1),
	GY521_STBY_XG =			(1 << 2),
	GY521_STBY_GYRO =		(7 << 0),
	GY521_STBY_ACCEL =		(7 << 3),
	GY521_STBY_ALL =		(0x3F)
} gy521_stby_t;

typedef enum {
	GY521_LP_WAKE_1_25HZ =		(0 << 6),
	GY521_LP_WAKE_5HZ  =		(1 << 6),
	GY521_LP_WAKE_20HZ =		(2 << 6),
	GY521_LP_WAKE_40HZ =		(3 << 6)
} gy521_lp_wake_t;

#define GY521_REG_FIFO_COUNTH		0x72
#define GY521_REG_FIFO_COUNTL		0x73
#define GY521_REG_FIFO_R_W		0x74

#define GY521_REG_WHO_AM_I		0x75
#define GY521_WHO_AM_I			0x68

#endif
