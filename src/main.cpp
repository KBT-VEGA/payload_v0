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

#define SD_CS      5
#define LORA_CS    17
#define LORA_RST   16
#define LORA_DIO0  14
#define BUZZER_PIN 27
#define GPS_RX     13
#define GPS_TX     15
#define GPS_BAUD   115200
#define I2C_SDA    21
#define I2C_SCL    22


BMP280_Driver bmp;
DHT11_Driver dht;
MPU6050_Driver mpu;
Compass_Driver compass;
GPS_Driver gps;
Buzzer_Driver buzzer(BUZZER_PIN);
SDCard_Driver sdcard(SD_CS);
LoRaDriver lora(LORA_CS, LORA_RST, LORA_DIO0, 433E6);

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);

  pinMode(SD_CS, OUTPUT);
  pinMode(LORA_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  digitalWrite(LORA_CS, HIGH);

  // initialize all sensors
  if (!bmp.begin()) Serial.println("BMP280 init failed");
  dht.begin();
  mpu.begin();
  compass.begin();
  gps.begin(GPS_RX, GPS_TX, GPS_BAUD);
  sdcard.begin();
  lora.begin();

  // test everything
  testAllSensors(bmp, dht, mpu, compass, gps, buzzer, sdcard, lora);

  // stateMachineInit(bmp, dht, mpu, compass, gps, buzzer, sdcard, lora);
}

void loop() {
  // stateMachineUpdate();
  delay(100);
}
