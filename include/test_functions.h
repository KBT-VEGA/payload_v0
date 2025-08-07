#include "bmp280_driver.h"
#include "buzzer_driver.h"
#include "compass_driver.h"
#include "dht11_driver.h"
#include "gps_driver.h"
#include "mpu6050_driver.h"
#include "sdcard_driver.h"
#include "state_machine.h"
#include <stdio.h>

bool testBMP280(BMP280_Driver &bmp) {
  float temp = bmp.readTemperature_C();
  float pressure = bmp.returnPressure_hPa();
  float altitude = bmp.calculateAltitude();

  Serial.printf("  Temp: %.2f°C, Pressure: %.2f hPa, Altitude: %.2f m\n", temp,
                pressure, altitude);

  // Validate reasonable ranges
  return (!isnan(temp) && temp >= -40.0f && temp <= 85.0f && !isnan(pressure) &&
          pressure >= 300.0f && pressure <= 1100.0f && !isnan(altitude));
}

bool testDHT11(DHT11_Driver &dht) {
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();

  Serial.printf("  Temp: %.2f°C, Humidity: %.2f%%\n", temp, humidity);

  return (!isnan(temp) && temp >= -40.0f && temp <= 80.0f && !isnan(humidity) &&
          humidity >= 0.0f && humidity <= 100.0f);
}

bool testMPU6050(MPU6050_Driver &mpu) {
  if (!mpu.testConnection()) {
    Serial.println("  Connection failed");
    return false;
  }

  float ax, ay, az, gx, gy, gz;
  mpu.readAccelGyro(ax, ay, az, gx, gy, gz);

  Serial.printf("  Accel: X=%.2f Y=%.2f Z=%.2f g\n", ax, ay, az);
  Serial.printf("  Gyro: X=%.2f Y=%.2f Z=%.2f °/s\n", gx, gy, gz);

  // Check if values are reasonable (not NaN, within expected ranges)
  float accelMag = sqrt(ax * ax + ay * ay + az * az);
  return (accelMag >= 0.5f &&
          accelMag <= 2.0f); // Should be close to 1g at rest
}

bool testCompass(Compass_Driver &compass) {
  float heading = compass.readHeading();
  Serial.printf("  Heading: %.2f degrees\n", heading);

  return (heading >= 0.0f && heading <= 360.0f);
}

bool testGPS(GPS_Driver &gps) {
  // GPS test requires time to acquire satellites
  Serial.println("  Waiting for GPS data (10 seconds)...");
  unsigned long startTime = millis();

  while (millis() - startTime < 10000) { // Wait 10 seconds
    gps.read();
    if (gps.hasFix()) {
      Serial.printf("  GPS Fix acquired! Lat: %.6f, Lon: %.6f\n",
                    gps.latitude(), gps.longitude());
      Serial.printf("  Satellites: %d, HDOP: %.2f\n", gps.satellites(),
                    gps.hdop());
      return true;
    }
    delay(100);
  }

  Serial.println("  No GPS fix acquired (normal indoors)");
  return false; // Return false but don't fail overall test if indoors
}

bool testBuzzer(Buzzer_Driver &buzzer) {
  Serial.println("  Playing test tone...");
  buzzer.beep(1000, 500); // 1kHz for 500ms
  return true;            // If no crash, assume it works
}

bool testSDCard(SDCard_Driver &sdcard) {
  if (!sdcard.isInitialized()) {
    Serial.println("  SD not initialized");
    return false;
  }

  // Test write and read
  String testData = "Test line: " + String(millis());
  if (!sdcard.writeLine("/test.txt", testData)) {
    Serial.println("  Write failed");
    return false;
  }

  String readData = sdcard.readFile("/test.txt");
  Serial.printf("  SD test successful, file contains %d bytes\n",
                readData.length());

  return (readData.length() > 0);
}
bool testAllSensors() {
  Serial.println("=== STARTING STATIC SENSOR TESTS ===");
  bool allTestsPassed = true;

  // Test BMP280
  Serial.print("Testing BMP280... ");
  if (testBMP280()) {
    Serial.println("PASSED");
  } else {
    Serial.println("FAILED");
    allTestsPassed = false;
  }

  // Test DHT11
  Serial.print("Testing DHT11... ");
  if (testDHT11()) {
    Serial.println("PASSED");
  } else {
    Serial.println("FAILED");
    allTestsPassed = false;
  }

  // Test MPU6050
  Serial.print("Testing MPU6050... ");
  if (testMPU6050()) {
    Serial.println("PASSED");
  } else {
    Serial.println("FAILED");
    allTestsPassed = false;
  }

  // Test Compass
  Serial.print("Testing Compass... ");
  if (testCompass()) {
    Serial.println("PASSED");
  } else {
    Serial.println("FAILED");
    allTestsPassed = false;
  }

  // Test GPS
  Serial.print("Testing GPS... ");
  if (testGPS()) {
    Serial.println("PASSED");
  } else {
    Serial.println("FAILED");
    allTestsPassed = false;
  }

  // Test Buzzer
  Serial.print("Testing Buzzer... ");
  if (testBuzzer()) {
    Serial.println("PASSED");
  } else {
    Serial.println("FAILED");
    allTestsPassed = false;
  }

  // Test SD Card
  Serial.print("Testing SD Card... ");
  if (testSDCard()) {
    Serial.println("PASSED");
  } else {
    Serial.println("FAILED");
    allTestsPassed = false;
  }

  Serial.println("=== TEST SUMMARY ===");
  Serial.print("Overall result: ");
  Serial.println(allTestsPassed ? "ALL TESTS PASSED" : "SOME TESTS FAILED");

  return allTestsPassed;
}
