#include "mpu6050.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"

static const char *TAG = "MPU6050_DRV";

// Registers
#define REG_PWR_MGMT_1     0x6B
#define REG_ACCEL_CONFIG   0x1C
#define REG_GYRO_CONFIG    0x1B
#define REG_ACCEL_XOUT_H   0x3B

// Sensitivity settings
#define ACCEL_RANGE_2G     0x00
#define GYRO_RANGE_250DPS  0x00

// LSB per unit
#define ACCEL_SENS_2G      16384  // LSB/g
#define GYRO_SENS_250DPS   131    // LSB/(°/s)

// Write single register
static esp_err_t write_reg(mpu6050_handle_t *h, uint8_t reg, uint8_t val) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (h->address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, val, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(h->i2c_port, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}

// Read multiple bytes
static esp_err_t read_regs(mpu6050_handle_t *h, uint8_t reg, uint8_t *buf, size_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (h->address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (h->address << 1) | I2C_MASTER_READ, true);
    for (size_t i = 0; i < len - 1; i++) {
        i2c_master_read_byte(cmd, &buf[i], I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, &buf[len-1], I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(h->i2c_port, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}

void mpu6050_i2c_init(i2c_port_t port) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GPIO_NUM_21,
        .scl_io_num = GPIO_NUM_22,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000
    };
    i2c_param_config(port, &conf);
    i2c_driver_install(port, conf.mode, 0, 0, 0);
}

esp_err_t mpu6050_init(mpu6050_handle_t *handle, i2c_port_t port, mpu6050_i2c_addr_t addr) {
    if (!handle) return ESP_ERR_INVALID_ARG;
    handle->i2c_port  = port;
    handle->address   = addr;
    handle->accel_sens = ACCEL_SENS_2G;
    handle->gyro_sens  = GYRO_SENS_250DPS;

    // Wake up sensor
    esp_err_t ret = write_reg(handle, REG_PWR_MGMT_1, 0x00);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to wake MPU6050");
        return ret;
    }
    // Set ranges
    ret = write_reg(handle, REG_ACCEL_CONFIG, ACCEL_RANGE_2G << 3);
    ret |= write_reg(handle, REG_GYRO_CONFIG, GYRO_RANGE_250DPS << 3);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure ranges");
    }
    return ret;
}

esp_err_t mpu6050_read(mpu6050_handle_t *handle, mpu6050_data_t *data) {
    if (!handle || !data) return ESP_ERR_INVALID_ARG;
    uint8_t buf[14];
    esp_err_t ret = read_regs(handle, REG_ACCEL_XOUT_H, buf, sizeof(buf));
    if (ret != ESP_OK) return ret;

    int16_t ax = (buf[0] << 8) | buf[1];
    int16_t ay = (buf[2] << 8) | buf[3];
    int16_t az = (buf[4] << 8) | buf[5];
    // skip temp bytes buf[6], buf[7]
    int16_t gx = (buf[8] << 8) | buf[9];
    int16_t gy = (buf[10] << 8)| buf[11];
    int16_t gz = (buf[12] << 8)| buf[13];

    data->ax = ax / (float)handle->accel_sens;
    data->ay = ay / (float)handle->accel_sens;
    data->az = az / (float)handle->accel_sens;
    data->gx = gx / (float)handle->gyro_sens;
    data->gy = gy / (float)handle->gyro_sens;
    data->gz = gz / (float)handle->gyro_sens;

    return ESP_OK;
}
