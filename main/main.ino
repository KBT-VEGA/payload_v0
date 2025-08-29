#include "../include/state_machine.h"
#include "../include/bmp280_driver.h"
#include "../include/dht11_driver.h"
#include "../include/buzzer_driver.h"
#include "../include/mpu6050_driver.h"
#include "../include/compass_driver.h"
#include "../include/gps_driver.h"
#include "../include/sdcard_driver.h"
#include "../include/lora_driver.h"
#include "../include/test_functions.h"

BMP280_Driver bmp;
DHT11_Driver dht;
MPU6050_Driver mpu;
Compass_Driver compass;
GPS_Driver gps;
Buzzer_Driver buzzer(27);
SDCard_Driver sdcard(5);
LoRaDriver lora(17, 16, 14, 433E6);

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  if (!bmp.begin()) Serial.println("BMP280 init failed");

  dht.begin();
  mpu.begin();
  compass.begin();

  gps.begin(13, 15, 115200);

  lora.begin();

  testAllSensors(bmp, dht, mpu, compass, gps, buzzer, sdcard, lora);

  stateMachineInit(bmp, dht, mpu, compass, gps, buzzer, sdcard, lora);
}

void loop() {
  stateMachineUpdate();
  delay(100);
}
