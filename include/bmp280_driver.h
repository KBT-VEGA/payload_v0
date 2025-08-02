#ifndef BMP280_DRIVER_H
#define BMP280_DRIVER_H

#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

class BMP280_Driver {
public:
  bool begin(uint8_t i2c_addr = 0x76) { return bmp.begin(i2c_addr); }

  float returnPressure_hPa() { return bmp.readPressure() / 100.0F; }

  float readTemperature_C() { return bmp.readTemperature(); }

  float calculateAltitude(float seaLevel_hPa = 1013.25) {
    return bmp.readAltitude(seaLevel_hPa);
  }

private:
  Adafruit_BMP280 bmp;
};

#endif // !BMP280_DRIVER.
