#ifndef DHT11_DRIVER_H
#define DHT11_DRIVER_H

#include "freertos/FreeRTOS.h"  

#include "driver/gpio.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief DHT11 device configuration
 */
typedef struct {
    gpio_num_t pin;           /**< GPIO pin connected to DHT11 data line */
    TickType_t task_delay_ms; /**< Delay between retries in ms */
} dht11_config_t;

/**
 * @brief DHT11 device handle
 */
typedef struct {
    dht11_config_t cfg;
} dht11_handle_t;

/**
 * @brief Initialize DHT11 sensor handle
 * @param handle Pointer to device handle
 * @param config User configuration parameters
 * @return ESP_OK on success, error otherwise
 */
esp_err_t dht11_init(dht11_handle_t *handle, const dht11_config_t *config);

/**
 * @brief Read temperature and humidity from DHT11
 * @param handle Device handle
 * @param[out] temperature in °C (integer)
 * @param[out] humidity in %% (integer)
 * @return ESP_OK on success, ESP_ERR_TIMEOUT or ESP_ERR_INVALID_CRC on failure
 */
esp_err_t dht11_read(dht11_handle_t *handle, int *temperature, int *humidity);

#ifdef __cplusplus
}
#endif

#endif // DHT11_DRIVER_H

