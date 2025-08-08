#ifndef COMPASS_DRIVER_H
#define COMPASS_DRIVER_H

#include <Adafruit_HMC5883_U.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

class Compass_Driver {
public:
  Compass_Driver() : compass(12345) {}

  void begin() {
    if (!compass.begin()) {
      Serial.println("Failed to detect HMC5883 sensor");
    }
  }

  float readHeading() {
    sensors_event_t event;
    compass.getEvent(&event);

    float heading = atan2(event.magnetic.y, event.magnetic.x) * 180.0 / PI;
    if (heading < 0) {
      heading += 360.0;
    }
    return heading;
  }

private:
  Adafruit_HMC5883_Unified compass;
};

#endif // !COMPASS_DRIVER_H
