#ifndef MPU6050_DRIVER_H
#define MPU6050_DRIVER_H

#include <MPU6050.h>
#include <Wire.h>
#include <cstdint>

class MPU6050_Driver {
public:
  void begin() {
    Wire.begin();
    mpu.initialize();
  }

  bool testConnection() { return mpu.testConnection(); }

  void readAccelGyro(float &ax, float &ay, float &az, float &gx, float &gy,
                     float &gz) {
    int16_t accelX, accelY, accelZ, gyroX, gyroY, gyroZ;
    mpu.getMotion6(&accelX, &accelY, &accelZ, &gyroX, &gyroY, &gyroZ);
    ax = accelX / 16384.0f; // assuming default range ±2g
    ay = accelY / 16384.0f;
    az = accelZ / 16384.0f;
    gx = gyroX / 131.0f; // assuming default range ±250°/s
    gy = gyroY / 131.0f;
    gz = gyroZ / 131.0f;
  }

private:
  MPU6050 mpu;
};

#endif // !MPU6050_DRIVER_H
