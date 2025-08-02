#include "../include/state_machine.h"
#include "../include/bmp280_driver.h"
#include "../include/dht11_driver.h"
#include "../include/buzzer_driver.h"
#include "../include/mpu6050_driver.h"
#include "../include/compass_driver.h"
#include "../include/gps_driver.h"

BMP280_Driver bmp;
DHT11_Driver dht;
MPU6050_Driver imu;
Compass_Driver compass;
GPS_Driver gps;
Buzzer_Driver buzzer;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  if (!bmp.begin()) Serial.println("BMP280 init failed");

  dht.begin();
  imu.begin();
  compass.begin();

  gps.begin(16, 17, 115200);

  buzzer.begin(27);

  stateMachineInit(bmp, dht, imu, compass, gps, buzzer);
}

void loop() {
  stateMachineUpdate();
  delay(100);
}
