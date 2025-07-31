#ifndef BMP280_H
#define BMP280_H

#include "driver/i2c.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BMP280_ADDRESS_DEFAULT 0x76

// Oversampling settings
typedef enum {
    BMP280_OSRS_SKIPPED = 0x00,
    BMP280_OSRS_1       = 0x01,
    BMP280_OSRS_2       = 0x02,
    BMP280_OSRS_4       = 0x03,
    BMP280_OSRS_8       = 0x04,
    BMP280_OSRS_16      = 0x05,
} bmp280_oversample_t;

// Sensor mode
typedef enum {
    BMP280_MODE_SLEEP  = 0x00,
    BMP280_MODE_FORCED = 0x01,
    BMP280_MODE_NORMAL = 0x03,
} bmp280_mode_t;

// IIR filter coefficient
typedef enum {
    BMP280_FILTER_OFF = 0x00,
    BMP280_FILTER_2   = 0x01,
    BMP280_FILTER_4   = 0x02,
    BMP280_FILTER_8   = 0x03,
    BMP280_FILTER_16  = 0x04,
} bmp280_filter_t;

// Configuration structure
typedef struct {
    i2c_port_t          i2c_port;
    uint8_t             i2c_address;
    bmp280_oversample_t osrs_t;
    bmp280_oversample_t osrs_p;
    bmp280_mode_t       mode;
    bmp280_filter_t     filter;
    uint8_t             standby_time;
} bmp280_config_t;

// Calibration parameters
typedef struct {
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
} bmp280_calib_t;

// Device handle
typedef struct {
    bmp280_config_t  cfg;
    bmp280_calib_t   calib;
    int32_t          t_fine;
} bmp280_dev_t;

/**
 * @brief Initialize BMP280 on given I2C bus
 * @param dev Pointer to device handle
 * @param config Configuration parameters
 * @return esp_err_t
 */
esp_err_t bmp280_init(bmp280_dev_t *dev, bmp280_config_t *config);

/**
 * @brief Read temperature in degC x100
 * @param dev Device handle
 * @param[out] temperature Temperature in centi-degrees Celsius
 * @return esp_err_t
 */
esp_err_t bmp280_read_temperature(bmp280_dev_t *dev, int32_t *temperature);

/**
 * @brief Read pressure in Pa
 * @param dev Device handle
 * @param[out] pressure Pressure in Pascals
 * @return esp_err_t
 */
esp_err_t bmp280_read_pressure(bmp280_dev_t *dev, uint32_t *pressure);

#ifdef __cplusplus
}
#endif

#endif // BMP280_H
