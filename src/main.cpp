#include "../include/state_machine.h"
#include "../include/bmp280_driver.h"
#include "../include/dht11_driver.h"
#include "../include/buzzer_driver.h"
#include "../include/mpu6050_driver.h"
#include "../include/compass_driver.h"
#include "../include/gps_driver.h"
#include "../include/sdcard_driver.h"
// #include "../include/lora_driver.h"  // Commented out LoRa for now
#include "../include/test_functions.h"

#define SD_CS 5
// #define LORA_CS    17      // Commented out LoRa pins
// #define LORA_RST   16
// #define LORA_DIO0  14
#define BUZZER_PIN 27
#define GPS_RX 13
#define GPS_TX 15
#define GPS_BAUD 115200
#define I2C_SDA 21
#define I2C_SCL 22

BMP280_Driver bmp;
DHT11_Driver dht;
MPU6050_Driver mpu;
Compass_Driver compass;
GPS_Driver gps;
Buzzer_Driver buzzer(BUZZER_PIN);
SDCard_Driver sdcard(SD_CS);
// LoRaDriver lora(LORA_CS, LORA_RST, LORA_DIO0, 433E6);  // Commented out LoRa

void setup()
{
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);

  pinMode(SD_CS, OUTPUT);
  // pinMode(LORA_CS, OUTPUT);  // Commented out LoRa pin setup
  digitalWrite(SD_CS, HIGH);
  // digitalWrite(LORA_CS, HIGH);

  // initialize all sensors
  if (!bmp.begin())
    Serial.println("BMP280 init failed");
  dht.begin();
  mpu.begin();
  compass.begin();
  gps.begin(GPS_RX, GPS_TX, GPS_BAUD);
  sdcard.begin();
  // lora.begin();  // Commented out LoRa initialization

  // test everything (without LoRa for now)
  // testAllSensors(bmp, dht, mpu, compass, gps, buzzer, sdcard, lora);

  // Initialize state machine (commented out for now)
  // stateMachineInit(bmp, dht, mpu, compass, gps, buzzer, sdcard);
}

void loop()
{
  // Read and print sensor data every 1 seconds
  static unsigned long lastPrint = 0;
  unsigned long now = millis();

  if (now - lastPrint >= 1000)
  {
    lastPrint = now;

    Serial.println("========== Sensor Data ==========");
    Serial.print("Timestamp: ");
    Serial.println(now);

    // BMP280 Data
    float temp_bmp = bmp.readTemperature_C();
    float pressure = bmp.returnPressure_hPa();
    float altitude = bmp.calculateAltitude(1013.25f);
    Serial.printf("BMP280 - Temp: %.2f°C, Pressure: %.2f hPa, Altitude: %.2f m\n",
                  temp_bmp, pressure, altitude);

    // DHT11 Data
    float temp_dht = dht.readTemperature();
    float humidity = dht.readHumidity();
    Serial.printf("DHT11 - Temp: %.2f°C, Humidity: %.2f%%\n", temp_dht, humidity);

    // MPU6050 Data
    float ax, ay, az, gx, gy, gz;
    mpu.readAccelGyro(ax, ay, az, gx, gy, gz);
    Serial.printf("MPU6050 - Accel: X=%.2f Y=%.2f Z=%.2f | Gyro: X=%.2f Y=%.2f Z=%.2f\n",
                  ax, ay, az, gx, gy, gz);

    // Compass Data
    float heading = compass.readHeading();
    Serial.printf("Compass - Heading: %.2f°\n", heading);

    // GPS Data
    gps.read();
    if (gps.hasFix())
    {
      float lat = gps.latitude();
      float lon = gps.longitude();
      Serial.printf("GPS - Lat: %.6f, Lon: %.6f\n", lat, lon);
    }
    else
    {
      Serial.println("GPS - No fix");
    }

    Serial.println("================================");
    Serial.println();
  }

  // stateMachineUpdate();  // Commented out for now
  delay(100);
}
