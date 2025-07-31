#include "dht11.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "rom/ets_sys.h"

static const char *TAG = "dht11";

/**
 * @brief Microsecond delay using ROM function
 */
static void dht11_delay_us(uint32_t us) {
    ets_delay_us(us);
}

/**
 * @brief Send start signal to DHT11
 */
static void dht11_send_start(dht11_handle_t *handle) {
    gpio_set_direction(handle->cfg.pin, GPIO_MODE_OUTPUT);
    gpio_set_level(handle->cfg.pin, 0);
    vTaskDelay(pdMS_TO_TICKS(20)); // hold low for >18ms
    gpio_set_level(handle->cfg.pin, 1);
    dht11_delay_us(30);
    gpio_set_direction(handle->cfg.pin, GPIO_MODE_INPUT);
}

/**
 * @brief Wait transition on the bus with timeout
 */
static esp_err_t dht11_wait_level(dht11_handle_t *handle, int level, uint32_t timeout_us) {
    uint32_t start = esp_log_timestamp();
    while (gpio_get_level(handle->cfg.pin) != level) {
        if ((esp_log_timestamp() - start) > timeout_us) {
            return ESP_ERR_TIMEOUT;
        }
    }
    return ESP_OK;
}

/**
 * @brief Read a single bit from DHT11
 */
static esp_err_t dht11_read_bit(dht11_handle_t *handle, uint8_t *bit) {
    // Wait for line to go low (50us)
    esp_err_t ret = dht11_wait_level(handle, 0, 100);
    if (ret != ESP_OK) return ret;
    // Wait for line to go high (start of bit)
    ret = dht11_wait_level(handle, 1, 100);
    if (ret != ESP_OK) return ret;
    // Measure length of high pulse
    dht11_delay_us(40);
    *bit = gpio_get_level(handle->cfg.pin);
    // Wait for end of bit
    ret = dht11_wait_level(handle, 0, 100);
    return ret;
}

/**
 * @brief Read a byte (8 bits)
 */
static esp_err_t dht11_read_byte(dht11_handle_t *handle, uint8_t *byte) {
    uint8_t val = 0;
    for (int i = 0; i < 8; ++i) {
        uint8_t bit;
        esp_err_t ret = dht11_read_bit(handle, &bit);
        if (ret != ESP_OK) return ret;
        val = (val << 1) | bit;
    }
    *byte = val;
    return ESP_OK;
}

esp_err_t dht11_init(dht11_handle_t *handle, const dht11_config_t *config) {
    if (!handle || !config) return ESP_ERR_INVALID_ARG;
    handle->cfg = *config;
    // Configure pull-up
    gpio_set_direction(handle->cfg.pin, GPIO_MODE_INPUT);
    gpio_set_pull_mode(handle->cfg.pin, GPIO_PULLUP_ONLY);
    return ESP_OK;
}

esp_err_t dht11_read(dht11_handle_t *handle, int *temperature, int *humidity) {
    if (!handle || !temperature || !humidity) return ESP_ERR_INVALID_ARG;

    dht11_send_start(handle);
    // DHT11 response: low for ~80us, high for ~80us
    esp_err_t ret = dht11_wait_level(handle, 0, 100);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "No response (LOW) from sensor");
        return ret;
    }
    ret = dht11_wait_level(handle, 1, 100);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "No response (HIGH) from sensor");
        return ret;
    }

    uint8_t data[5];
    for (int i = 0; i < 5; ++i) {
        ret = dht11_read_byte(handle, &data[i]);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed reading byte %d", i);
            return ret;
        }
    }
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) {
        ESP_LOGE(TAG, "CRC mismatch: expected %02X got %02X", checksum, data[4]);
        return ESP_ERR_INVALID_CRC;
    }

    *humidity = data[0];
    *temperature = data[2];
    return ESP_OK;
}
