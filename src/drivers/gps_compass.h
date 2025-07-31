#ifndef GPS_COMPASS_DRIVER_H
#define GPS_COMPASS_DRIVER_H

#include <stdint.h>
#include "driver/uart.h"
#include "driver/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

// GPS over UART
#define GPS_UART_NUM           UART_NUM_1
#define GPS_UART_TX_PIN        GPIO_NUM_4  // TX pin to GPS module RX
#define GPS_UART_RX_PIN        GPIO_NUM_5  // RX pin from GPS module TX
#define GPS_UART_BAUD_RATE     9600
#define GPS_UART_BUF_SIZE      1024

/**
 * @brief  Initialize UART for NEO-7M GPS.
 */
void gps_init(void);

/**
 * @brief  Read one NMEA sentence (terminated by '\r\n').
 * @param  out_buf: buffer to receive the sentence
 * @param  buf_len: length of out_buf
 * @param  timeout_ms: timeout waiting for sentence
 * @return length of sentence (excluding terminator), or -1 on timeout
 */
int gps_read_sentence(char* out_buf, size_t buf_len, uint32_t timeout_ms);

// Compass over I2C
#define COMPASS_I2C_PORT       I2C_NUM_0
#define COMPASS_I2C_SDA_PIN    GPIO_NUM_0
#define COMPASS_I2C_SCL_PIN    GPIO_NUM_2
#define COMPASS_I2C_FREQ_HZ    100000
#define COMPASS_I2C_ADDR       0x1E       // default HMC5883L address

/**
 * @brief  Initialize I2C and configure compass (HMC5883L).
 * @return ESP_OK on success
 */
esp_err_t compass_init(void);

/**
 * @brief  Read raw magnetometer data (X, Y, Z).
 * @param  mx, my, mz pointers to int16_t to receive data
 * @return ESP_OK on success
 */
esp_err_t compass_read_raw(int16_t* mx, int16_t* my, int16_t* mz);

#ifdef __cplusplus
}
#endif

#endif // GPS_COMPASS_DRIVER_H