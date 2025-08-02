#ifndef DHT11_DRIVER_H
#define DHT11_DRIVER_H

#include <DHT.h>
#include <cmath>

#define DHTPIN 17
#define DHTTYPE DHT11

class DHT11_Driver {
public:
  void begin() { dht.begin(); }

  float readTemperature() {
    float t = dht.readTemperature(); // Celsius
    if (std::isnan(t))
      return NAN;
    return t;
  }

  float readHumidity() {
    float h = dht.readHumidity();
    if (std::isnan(h))
      return NAN;
    return h;
  }

private:
  DHT dht = DHT(DHTPIN, DHTTYPE);
};

#endif
