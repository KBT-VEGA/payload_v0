#include "../include/state_machine.h"
#include <Arduino.h>
#include <math.h>

static BMP280_Driver *bmp_ptr = nullptr;
static DHT11_Driver *dht_ptr = nullptr;
static MPU6050_Driver *mpu_ptr = nullptr;
static Compass_Driver *compass_ptr = nullptr;
static GPS_Driver *gps_ptr = nullptr;
static Buzzer_Driver *buzzer_ptr = nullptr;
static SDCard_Driver *sdcard_ptr = nullptr;
static LoRaDriver *lora_ptr = nullptr;

// current state
static FlightState currentState = PRELAUNCH;

// variables to track state transitions
static float initialAltitude = NAN;
static float lastAltitude = NAN;
static unsigned long landedStableTime = 0;
static const unsigned long LANDING_STABLE_DURATION_MS =
    3000; // 3 seconds for landing confirmation

// thresholds
static const float ASCENT_ACCELERATION_THRESHOLD = 0.5f; // g(gravity)
static const float ALTITUDE_CHANGE_THRESHOLD =
    2.0f; // meters, for "no altitude change" detection

// sensor calibration bool
static bool sensorsCalibrated = false;

// helper to read altitude from BMP280
static float getAltitude() {
  if (bmp_ptr)
    return bmp_ptr->calculateAltitude(1013.25f);
  return NAN;
}

// helper to read acceleration magnitude from MPU6050
static float getAccelGyro() {
  if (mpu_ptr) {
    float ax, ay, az, gx, gy, gz;
    mpu_ptr->readAccelGyro(ax, ay, az, gx, gy, gz);
    return sqrtf(ax * ax + ay * ay + az * az);
  }
  return 0.0f;
}

// helper to check increase in altitude and acceleration in PRELAUNCH
static bool detectLaunch() {
  float alt = getAltitude();
  if (isnan(initialAltitude)) {
    initialAltitude = alt;
    return false;
  }
  float accelMag = getAccelGyro();
  // launch if altitude increase by more than 10m or acceleration significantly
  // above 1g
  if ((alt - initialAltitude) > 10.0f ||
      (accelMag - 1.0f) > ASCENT_ACCELERATION_THRESHOLD) {
    Serial.println("Launch detected");
    return true;
  }
  return false;
}

// helper to detect descent (altitude starting to dcrease)
static bool detectDescent() {
  float alt = getAltitude();
  if (!isnan(lastAltitude) && alt < lastAltitude - 1.0f) { // altitude drop > 1m
    Serial.println("Descent detected");
    return true;
  }
  return false;
}

// helper to detect stable altitude (landing)
static bool detectLanding() {
  float alt = getAltitude();
  if (isnan(lastAltitude)) {
    lastAltitude = alt;
    return false;
  }
  float diff = fabsf(alt - lastAltitude);
  lastAltitude = alt;

  if (diff < ALTITUDE_CHANGE_THRESHOLD) {
    unsigned long now = millis();
    if (landedStableTime == 0) {
      landedStableTime = now;
    } else if (now - landedStableTime >= LANDING_STABLE_DURATION_MS) {
      Serial.println("Landing detected");
      return true;
    }
  } else {
    landedStableTime = 0;
  }
  return false;
}

// helper for calibration sensor condition
static void checkSensorCondition(bool condition, const char *sensorName) {
  if (condition) {
    Serial.print(sensorName);
    Serial.println(" sanity check PASSED.");
  } else {
    Serial.print(sensorName);
    Serial.println(" sanity check FAILED!");
  }
}

// sensor sanity check function
static void verifySensorSanity() {
  bool result;

  // BMP280 temperature range -40 to +85 °C &
  // pressure range ~300 to ~1100 hPa
  if (bmp_ptr) {
    float temp = bmp_ptr->readTemperature_C();
    result = (!isnan(temp) && temp >= -40.0f && temp <= 85.0f);
    checkSensorCondition(result, "BMP280 Temperature");

    float pres = bmp_ptr->returnPressure_hPa();
    result = (!isnan(pres) && pres >= 300.0f && pres <= 1100.0f);
    checkSensorCondition(result, "BMP280 Pressure");
  }

  // DHT11 temperature typically 0 to 50 °C &
  // humidity typically 20% to 90%
  if (dht_ptr) {
    float temp = dht_ptr->readTemperature();
    result = (!isnan(temp) && temp >= 0.0f && temp <= 50.0f);
    checkSensorCondition(result, "DHT11 Temperature");

    float hum = dht_ptr->readHumidity();
    result = (!isnan(hum) && hum >= 20.0f && hum <= 90.0f);
    checkSensorCondition(result, "DHT11 Humidity");
  }

  // MPU6050 accel: raw accelerations in g; we check magnitude reasonable range
  // ~0 to 16g
  if (mpu_ptr) {
    float ax, ay, az, gx, gy, gz;
    mpu_ptr->readAccelGyro(ax, ay, az, gx, gy, gz);
    float accelMag = sqrtf(ax * ax + ay * ay + az * az);
    result = (accelMag >= 0.0f &&
              accelMag <= 16.0f); // MPU6050 max ±16g configurable
    checkSensorCondition(result, "MPU6050 Acceleration");
  }

  // Compass heading check: should be between 0 and 360 degrees
  if (compass_ptr) {
    float heading = compass_ptr->readHeading();
    result = (heading >= 0.0f && heading <= 360.0f);
    checkSensorCondition(result, "Compass Heading");
  }

  // GPS fix presence - simple valid fix (boolean) check
  if (gps_ptr) {
    result = gps_ptr->hasFix();
    checkSensorCondition(result, "GPS Fix");
  }
}

void stateMachineInit(BMP280_Driver &bmp, DHT11_Driver &dht,
                      MPU6050_Driver &mpu, Compass_Driver &compass,
                      GPS_Driver &gps, Buzzer_Driver &buzzer,
                      SDCard_Driver &sdcard, LoRaDriver &lora) {
  bmp_ptr = &bmp;
  dht_ptr = &dht;
  mpu_ptr = &mpu;
  compass_ptr = &compass;
  gps_ptr = &gps;
  buzzer_ptr = &buzzer;
  sdcard_ptr = &sdcard;
  lora_ptr = &lora;

  currentState = PRELAUNCH;

  initialAltitude = NAN;
  lastAltitude = NAN;
  landedStableTime = 0;

  Serial.println("State machine initialized: PRELAUNCH");
}

void stateMachineUpdate() {
  switch (currentState) {
  case PRELAUNCH:
    if (!sensorsCalibrated) {
      verifySensorSanity();
      sensorsCalibrated = true;
    }

    // check for launch condition (altitude increase or accelration)
    if (detectLaunch()) {
      currentState = ASCENT;
      Serial.println("Transition to ASCENT");
    }
    break;
  case ASCENT:
    // TODO: monitor sensors, log and send data over telemetry

    // transition on altitude decrease inidicating start of descent
    if (detectDescent()) {
      currentState = DESCENT;
      Serial.println("Transition to DESCENT");
    }
    break;

  case DESCENT:
    // TODO: track descent, position, keep logging and sending data over
    // telemetry

    // check if landed (no altitude change for given duration)
    if (detectLanding()) {
      currentState = POSTLAND;
      Serial.println("Transition to POSTLAND");
      // Power down heavy sensors (do this once)
      static bool powerDownComplete = false;
      if (!powerDownComplete) {
        Serial.println("Powering down sensors...");
        if (mpu_ptr)
          mpu_ptr->powerDown();
        if (compass_ptr)
          compass_ptr->powerDown(); // Add this method
        if (bmp_ptr)
          bmp_ptr->powerDown();
        // Keep GPS and SD card active for recovery data logging
        powerDownComplete = true;
      }

      // Keep GPS active for location reporting
      // if (gps_ptr) {
      //   gps_ptr->read();
      //   if (gps_ptr->locationUpdated()) {
      //     // Log GPS coordinates to SD for recovery
      //     String gpsData = String(gps_ptr->latitude(), 6) + "," +
      //                      String(gps_ptr->longitude(), 6);
      //     if (sd_ptr)
      //       sd_ptr->writeLine("/recovery.txt", gpsData);
      //   }
      // }

      if (buzzer_ptr) {
        // TODO: beeping pattern?
        buzzer_ptr->startTone(2000); // 2kHz tone
      }
    }
    break;

  case POSTLAND:
    // TODO: keep sending gps data, beacon is active
    if (gps_ptr) {
      gps_ptr->read();

      if (gps_ptr->locationUpdated()) {
        // TODO: send gps data over telem
      }
    }
    break;
  default:
    Serial.println("Unknown state");
    break;
  }
}
