#ifndef COMPASS_DRIVER_H
#define COMPASS_DRIVER_H

#include <QMC5883L.h>
#include <Wire.h>

class Compass_Driver {
public:
  void begin() {
    compass.init();
    compass.setMode(QMC5883L_CONTINOUS);
    compass.setRange(QMC5883L_RANGE_2G);
    compass.setOutputDataRate(QMC5883L_ODR_50HZ);
    compass.setMeasurementMode(QMC5883L_CONTINOUS);
  }

  float readHeading() {
    compass.read();
    float heading = atan2(compass.getY(), compass.getX()) * 180.0 / PI;
    if (heading < 0)
      heading += 360.0;
    return heading;
  }

private:
  QMC5883L compass;
};

#endif // !COMPASS_DRIVER_H
