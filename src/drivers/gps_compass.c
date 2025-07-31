#include "gps_compass.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "gps_compass";

void gps_init(void) {
  uart_config_t uart_config = {.baud_rate = GPS_UART_BAUD_RATE,
                               .data_bits = UART_DATA_8_BITS,
                               .parity = UART_PARITY_DISABLE,
                               .stop_bits = UART_STOP_BITS_1,
                               .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
  uart_param_config(GPS_UART_NUM, &uart_config);
  uart_set_pin(GPS_UART_NUM, GPS_UART_TX_PIN, GPS_UART_RX_PIN,
               UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  uart_driver_install(GPS_UART_NUM, GPS_UART_BUF_SIZE, 0, 0, NULL, 0);
}

int gps_read_sentence(char *out_buf, size_t buf_len, uint32_t timeout_ms) {
  size_t idx = 0;
  uint8_t ch;
  uint32_t start = xTaskGetTickCount() * portTICK_PERIOD_MS;
  while (1) {
    int len = uart_read_bytes(GPS_UART_NUM, &ch, 1, pdMS_TO_TICKS(100));
    if (len <= 0) {
      if ((xTaskGetTickCount() * portTICK_PERIOD_MS - start) > timeout_ms) {
        return -1;
      }
      continue;
    }
    // collect characters
    if (ch == '\r')
      continue;
    if (ch == '\n') {
      out_buf[idx] = '\0';
      return idx;
    }
    if (idx < buf_len - 1) {
      out_buf[idx++] = ch;
    }
  }
}

esp_err_t compass_init(void) {
  // Configure I2C master
  i2c_config_t conf = {.mode = I2C_MODE_MASTER,
                       .sda_io_num = COMPASS_I2C_SDA_PIN,
                       .scl_io_num = COMPASS_I2C_SCL_PIN,
                       .sda_pullup_en = GPIO_PULLUP_ENABLE,
                       .scl_pullup_en = GPIO_PULLUP_ENABLE,
                       .master.clk_speed = COMPASS_I2C_FREQ_HZ};
  esp_err_t ret = i2c_param_config(COMPASS_I2C_PORT, &conf);
  if (ret != ESP_OK)
    return ret;
  ret = i2c_driver_install(COMPASS_I2C_PORT, conf.mode, 0, 0, 0);
  if (ret != ESP_OK)
    return ret;

  // Configure HMC5883L: 8-average, 15 Hz default, normal measurement
  uint8_t cfgA[] = {0x00, 0x70}; // CRA: 0x70=8sample avg,15Hz,normal
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (COMPASS_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd, 0x00, true); // reg CRA
  i2c_master_write(cmd, cfgA, sizeof(cfgA), true);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(COMPASS_I2C_PORT, cmd, pdMS_TO_TICKS(1000));
  i2c_cmd_link_delete(cmd);
  return ret;
}

esp_err_t compass_read_raw(int16_t *mx, int16_t *my, int16_t *mz) {
  uint8_t data[6];
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (COMPASS_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd, 0x03, true); // starting at X MSB
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (COMPASS_I2C_ADDR << 1) | I2C_MASTER_READ, true);
  i2c_master_read(cmd, data, 6, I2C_MASTER_LAST_NACK);
  i2c_master_stop(cmd);
  esp_err_t ret =
      i2c_master_cmd_begin(COMPASS_I2C_PORT, cmd, pdMS_TO_TICKS(1000));
  i2c_cmd_link_delete(cmd);
  if (ret != ESP_OK)
    return ret;

  // combine MSB and LSB
  *mx = (int16_t)((data[0] << 8) | data[1]);
  *mz = (int16_t)((data[2] << 8) | data[3]);
  *my = (int16_t)((data[4] << 8) | data[5]);
  return ESP_OK;
}