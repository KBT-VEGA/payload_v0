#ifndef MPU6050_DRIVER_H
#define MPU6050_DRIVER_H

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <cstdint>

class MPU6050_Driver {
public:
  void begin() {
    Wire.begin();
    if (!mpu.begin()) {
      Serial.println("Failed to find MPU6050 chip");
      while (1) {
        delay(10);
      }
    }
    mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
    mpu.setGyroRange(MPU6050_RANGE_250_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  }

  void readAccelGyro(float &ax, float &ay, float &az, float &gx, float &gy,
                     float &gz) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // Convert accelerometer readings from m/s² to g by dividing by 9.80665
    ax = a.acceleration.x / 9.80665f;
    ay = a.acceleration.y / 9.80665f;
    az = a.acceleration.z / 9.80665f;

    gx = g.gyro.x;
    gy = g.gyro.y;
    gz = g.gyro.z;
  }

  void powerDown() { Serial.println("Stop using MPU6050"); }

private:
  Adafruit_MPU6050 mpu;
};

#endif // !MPU6050_DRIVER_H
