#ifndef MPU_6050_DEFS_H
#define MPU_6050_DEFS_H


#define MPU6050_TIMEOUT                   (10u)

/* Memory Map */
#define MPU6050_SELF_TEST_X               (0x0D) /* Read Only   */
#define MPU6050_SELF_TEST_Y               (0x0E) /* Read Only   */
#define MPU6050_SELF_TEST_Z               (0x0F) /* Read Only   */
#define MPU6050_SELF_TEST_A               (0x10) /* Read Only   */
#define MPU6050_SMPRT_DIV                 (0x19) /* Read/Write  */
#define MPU6050_CONFIG                    (0x1A) /* Read/Write  */
#define MPU6050_GYRO_CONFIG               (0x1B) /* Read/Write  */
#define MPU6050_ACCEL_CONFIG              (0x1C) /* Read/Write  */
#define MPU6050_FIFO_EN                   (0x23) /* Read/Write  */
#define MPU6050_ACCEL_XOUT_H              (0x3B) /* Read Only   */
#define MPU6050_ACCEL_XOUT_L              (0x3C) /* Read Only   */
#define MPU6050_ACCEL_YOUT_H              (0x3D) /* Read Only   */
#define MPU6050_ACCEL_YOUT_L              (0x3E) /* Read Only   */
#define MPU6050_ACCEL_ZOUT_H              (0x3F) /* Read Only   */
#define MPU6050_ACCEL_ZOUT_L              (0x40) /* Read Only   */
#define MPU6050_TEMP_OUT_H                (0x41) /* Read Only   */
#define MPU6050_TEMP_OUT_L                (0x42) /* Read Only   */
#define MPU6050_GYRO_XOUT_H               (0x43) /* Read Only   */
#define MPU6050_GYRO_XOUT_L               (0x44) /* Read Only   */
#define MPU6050_GYRO_YOUT_H               (0x45) /* Read Only   */
#define MPU6050_GYRO_YOUT_L               (0x46) /* Read Only   */
#define MPU6050_GYRO_ZOUT_H               (0x47) /* Read Only   */
#define MPU6050_GYRO_ZOUT_L               (0x48) /* Read Only   */
#define MPU6050_SIGNAL_PATH_RESET         (0x68) /* Write Only  */
#define MPU6050_USER_CTRL                 (0x6A) /* Read/Write  */
#define MPU6050_PWR_MGMT_1                (0x6B) /* Read/Write  */
#define MPU6050_PWR_MGMT_2                (0x6C) /* Read/Write  */
#define MPU6050_FIFO_COUNT_H              (0x72) /* Read Only   */
#define MPU6050_FIFO_COUNT_L              (0x73) /* Read Only   */
#define MPU6050_WHO_AM_I                  (0x75) /* Read Only   */

/* Bit Masks */
#define MPU6050_SELF_TEST_X_XA_TEST           (7u  << 5)
#define MPU6050_SELF_TEST_X_XG_TEST           (31u << 0)
#define MPU6050_SELF_TEST_Y_YA_TEST           (7u  << 5)
#define MPU6050_SELF_TEST_Y_YG_TEST           (31u << 0)
#define MPU6050_SELF_TEST_Z_ZA_TEST           (7u  << 5)
#define MPU6050_SELF_TEST_Z_ZG_TEST           (31u << 0)
#define MPU6050_SELF_TEST_A_XA_TEST           (3u  << 4)
#define MPU6050_SELF_TEST_A_YA_TEST           (3u  << 2)
#define MPU6050_SELF_TEST_A_ZA_TEST           (3u  << 0)

#define MPU6050_SMPRT_DIV_SMPRT_DIV(x)      (((x)  << 0) & 0xFF)

#define MPU6050_CONFIG_EXT_SYNC_DISABLED      (0u  << 3)
#define MPU6050_CONFIG_EXT_SYNC_TEMP_OUT      (1u  << 3)
#define MPU6050_CONFIG_EXT_SYNC_GYRO_XOUT     (2u  << 3)
#define MPU6050_CONFIG_EXT_SYNC_GYRO_YOUT     (3u  << 3)
#define MPU6050_CONFIG_EXT_SYNC_GYRO_ZOUT     (4u  << 3)
#define MPU6050_CONFIG_EXT_SYNC_ACCEL_XOUT    (5u  << 3)
#define MPU6050_CONFIG_EXT_SYNC_ACCEL_YOUT    (6u  << 3)
#define MPU6050_CONFIG_EXT_SYNC_ACCEL_ZOUT    (7u  << 3)
#define MPU6050_CONFIG_DLPF_CFG(x)          (((x)  << 0) & 0x7)

#define MPU6050_GYRO_CONFIG_XG_ST             (1u  << 7)
#define MPU6050_GYRO_CONFIG_YG_ST             (1u  << 6)
#define MPU6050_GYRO_CONFIG_ZG_ST             (1u  << 5)
#define MPU6050_GYRO_CONFIG_FS_SEL_250        (0u  << 3)
#define MPU6050_GYRO_CONFIG_FS_SEL_500        (1u  << 3)
#define MPU6050_GYRO_CONFIG_FS_SEL_1000       (2u  << 3)
#define MPU6050_GYRO_CONFIG_FS_SEL_2000       (3u  << 3)

#define MPU6050_ACCEL_CONFIG_XA_ST            (1u  << 7)
#define MPU6050_ACCEL_CONFIG_YA_ST            (1u  << 6)
#define MPU6050_ACCEL_CONFIG_ZA_ST            (1u  << 5)
#define MPU6050_ACCEL_CONFIG_AFS_SEL_2G       (0u  << 3)
#define MPU6050_ACCEL_CONFIG_AFS_SEL_4G       (1u  << 3)
#define MPU6050_ACCEL_CONFIG_AFS_SEL_8G       (2u  << 3)
#define MPU6050_ACCEL_CONFIG_AFS_SEL_16G      (3u  << 3)

#define MPU6050_FIFO_EN_TEMP_FIFO_EN          (1u  << 7)
#define MPU6050_FIFO_EN_XG_FIFO_EN            (1u  << 6)
#define MPU6050_FIFO_EN_YG_FIFO_EN            (1u  << 5)
#define MPU6050_FIFO_EN_ZG_FIFO_EN            (1u  << 4)
#define MPU6050_FIFO_EN_ACCEL_FIFO_EN         (1u  << 3)
#define MPU6050_FIFO_EN_SLV2_FIFO_EN          (1u  << 2)
#define MPU6050_FIFO_EN_SLV1_FIFO_EN          (1u  << 1)
#define MPU6050_FIFO_EN_SLV0_FIFO_EN          (1u  << 0)

#define MPU6050_SIGNAL_PATH_RESET_GYRO_RESET  (1u  << 2)
#define MPU6050_SIGNAL_PATH_RESET_ACCEL_RESET (1u  << 1)
#define MPU6050_SIGNAL_PATH_RESET_TEMP_RESET  (1u  << 0)

#define MPU6050_USER_CTRL_FIFO_EN             (1u  << 6)
#define MPU6050_USER_CTRL_I2C_MST_EN          (1u  << 5)
#define MPU6050_USER_CTRL_I2C_IF_DIS          (1u  << 4)
#define MPU6050_USER_CTRL_FIFO_RESET          (1u  << 2)
#define MPU6050_USER_CTRL_I2C_MST_RESET       (1u  << 1)
#define MPU6050_USER_CTRL_SIG_COND_RESET      (1u  << 0)

#define MPU6050_PWR_MGMT_1_DEVICE_RESET       (1u  << 7)
#define MPU6050_PWR_MGMT_1_SLEEP              (1u  << 6)
#define MPU6050_PWR_MGMT_1_CYCLE              (1u  << 5)
#define MPU6050_PWR_MGMT_1_TEMP_DIS           (1u  << 3)
#define MPU6050_PWR_MGMT_1_CLKSEL_INT_OSC     (1u  << 0)
#define MPU6050_PWR_MGMT_1_CLKSEL_X_GYRO      (1u  << 0)
#define MPU6050_PWR_MGMT_1_CLKSEL_Y_GYRO      (2u  << 0)
#define MPU6050_PWR_MGMT_1_CLKSEL_Z_GYRO      (3u  << 0)
#define MPU6050_PWR_MGMT_1_CLKSEL_EXT_32KHZ   (4u  << 0)
#define MPU6050_PWR_MGMT_1_CLKSEL_EXT_19MHZ   (5u  << 0)
#define MPU6050_PWR_MGMT_1_CLKSEL_STOP        (7u  << 0)


#endif /* MPU_6050_DEFS_H */
