#include "bmp280.h"
#include <string.h>
#include "esp_log.h"

static const char *TAG = "bmp280";

// Registers
#define REG_CALIB_START   0x88
#define REG_CTRL_MEAS     0xF4
#define REG_CONFIG        0xF5
#define REG_PRESS_MSB     0xF7
#define REG_TEMP_MSB      0xFA

// I2C read/write
static esp_err_t read_regs(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t *data, size_t len) {
    return i2c_master_write_read_device(port, addr, &reg, 1, data, len, pdMS_TO_TICKS(1000));
}

static esp_err_t write_regs(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t *data, size_t len) {
    uint8_t buf[len+1];
    buf[0] = reg;
    memcpy(buf+1, data, len);
    return i2c_master_write_to_device(port, addr, buf, len+1, pdMS_TO_TICKS(1000));
}

esp_err_t bmp280_init(bmp280_dev_t *dev, bmp280_config_t *config) {
    if (!dev || !config) return ESP_ERR_INVALID_ARG;
    dev->cfg = *config;
    // Read calibration
    uint8_t calib_raw[24];
    esp_err_t ret = read_regs(config->i2c_port, config->i2c_address, REG_CALIB_START, calib_raw, sizeof(calib_raw));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read calibration: %s", esp_err_to_name(ret));
        return ret;
    }
    // Parse calibration
    dev->calib.dig_T1 = (uint16_t)(calib_raw[1] << 8 | calib_raw[0]);
    dev->calib.dig_T2 = (int16_t ) (calib_raw[3] << 8 | calib_raw[2]);
    dev->calib.dig_T3 = (int16_t ) (calib_raw[5] << 8 | calib_raw[4]);
    dev->calib.dig_P1 = (uint16_t)(calib_raw[7] << 8 | calib_raw[6]);
    dev->calib.dig_P2 = (int16_t ) (calib_raw[9] << 8 | calib_raw[8]);
    dev->calib.dig_P3 = (int16_t ) (calib_raw[11]<< 8 | calib_raw[10]);
    dev->calib.dig_P4 = (int16_t ) (calib_raw[13]<< 8 | calib_raw[12]);
    dev->calib.dig_P5 = (int16_t ) (calib_raw[15]<< 8 | calib_raw[14]);
    dev->calib.dig_P6 = (int16_t ) (calib_raw[17]<< 8 | calib_raw[16]);
    dev->calib.dig_P7 = (int16_t ) (calib_raw[19]<< 8 | calib_raw[18]);
    dev->calib.dig_P8 = (int16_t ) (calib_raw[21]<< 8 | calib_raw[20]);
    dev->calib.dig_P9 = (int16_t ) (calib_raw[23]<< 8 | calib_raw[22]);
    // Configure sensor
    uint8_t ctrl = ((config->osrs_t & 0x07) << 5) |
                   ((config->osrs_p & 0x07) << 2) |
                   (config->mode & 0x03);
    ret = write_regs(config->i2c_port, config->i2c_address, REG_CTRL_MEAS, &ctrl, 1);
    if (ret != ESP_OK) return ret;
    uint8_t cfg = ((config->standby_time & 0x07) << 5) |
                  ((config->filter & 0x07) << 2);
    ret = write_regs(config->i2c_port, config->i2c_address, REG_CONFIG, &cfg, 1);
    return ret;
}

static int32_t compensate_temp(bmp280_dev_t *dev, int32_t adc_T) {
    int32_t var1 = (((adc_T >> 3) - ((int32_t)dev->calib.dig_T1 << 1)) * dev->calib.dig_T2) >> 11;
    int32_t var2 = (((((adc_T >> 4) - dev->calib.dig_T1) * ((adc_T >> 4) - dev->calib.dig_T1)) >> 12) * dev->calib.dig_T3) >> 14;
    dev->t_fine = var1 + var2;
    return (dev->t_fine * 5 + 128) >> 8;
}

static uint32_t compensate_pres(bmp280_dev_t *dev, int32_t adc_P) {
    int64_t var1 = (int64_t)dev->t_fine - 128000;
    int64_t var2 = var1 * var1 * dev->calib.dig_P6;
    var2 += (var1 * dev->calib.dig_P5) << 17;
    var2 += ((int64_t)dev->calib.dig_P4) << 35;
    var1 = ((var1 * var1 * dev->calib.dig_P3) >> 8) + ((var1 * dev->calib.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * dev->calib.dig_P1 >> 33;
    if (!var1) return 0;
    int64_t p = 1048576 - adc_P;
    p = ((p << 31) - var2) * 3125 / var1;
    var1 = (dev->calib.dig_P9 * (p >> 13) * (p >> 13)) >> 25;
    var2 = (dev->calib.dig_P8 * p) >> 19;
    p = ((p + var1 + var2) >> 8) + ((int64_t)dev->calib.dig_P7 << 4);
    return (uint32_t)p / 256;
}

esp_err_t bmp280_read_temperature(bmp280_dev_t *dev, int32_t *temperature) {
    if (!dev || !temperature) return ESP_ERR_INVALID_ARG;
    uint8_t buf[3];
    esp_err_t ret = read_regs(dev->cfg.i2c_port, dev->cfg.i2c_address, REG_TEMP_MSB, buf, 3);
    if (ret != ESP_OK) return ret;
    int32_t adc_T = (buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4);
    *temperature = compensate_temp(dev, adc_T);
    return ESP_OK;
}

esp_err_t bmp280_read_pressure(bmp280_dev_t *dev, uint32_t *pressure) {
    if (!dev || !pressure) return ESP_ERR_INVALID_ARG;
    int32_t temp;
    esp_err_t ret = bmp280_read_temperature(dev, &temp);
    if (ret != ESP_OK) return ret;
    uint8_t buf[3];
    ret = read_regs(dev->cfg.i2c_port, dev->cfg.i2c_address, REG_PRESS_MSB, buf, 3);
    if (ret != ESP_OK) return ret;
    int32_t adc_P = (buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4);
    *pressure = compensate_pres(dev, adc_P);
    return ESP_OK;
}
