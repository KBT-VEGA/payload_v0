#ifndef MPU6050_DRIVER_H
#define MPU6050_DRIVER_H

#include "driver/i2c.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief MPU6050 I2C address options
 */
typedef enum {
    MPU6050_ADDR_HIGH = 0x69,  /**< AD0 pin high */
    MPU6050_ADDR_LOW  = 0x68   /**< AD0 pin low (default) */
} mpu6050_i2c_addr_t;

/**
 * @brief Accelerometer/gyro data structure
 */
typedef struct {
    float ax; /**< Acceleration X in g */
    float ay; /**< Acceleration Y in g */
    float az; /**< Acceleration Z in g */
    float gx; /**< Rotation rate X in °/s */
    float gy; /**< Rotation rate Y in °/s */
    float gz; /**< Rotation rate Z in °/s */
} mpu6050_data_t;

/**
 * @brief MPU6050 device handle
 */
typedef struct {
    i2c_port_t        i2c_port;
    uint8_t           address;
    int16_t           accel_sens;  /**< LSB/g */
    int16_t           gyro_sens;   /**< LSB/(°/s) */
} mpu6050_handle_t;

/**
 * @brief Initialize I2C port for MPU6050
 * @param port I2C port number
 */
void mpu6050_i2c_init(i2c_port_t port);

/**
 * @brief Initialize MPU6050
 * @param handle Device handle
 * @param port I2C port number
 * @param addr I2C address (MPU6050_ADDR_LOW or HIGH)
 * @return ESP_OK on success
 */
esp_err_t mpu6050_init(mpu6050_handle_t *handle, i2c_port_t port, mpu6050_i2c_addr_t addr);

/**
 * @brief Read accelerometer and gyroscope data
 * @param handle Device handle
 * @param[out] data Filled data struct
 * @return ESP_OK on success
 */
esp_err_t mpu6050_read(mpu6050_handle_t *handle, mpu6050_data_t *data);

#ifdef __cplusplus
}
#endif

#endif // MPU6050_DRIVER_H
