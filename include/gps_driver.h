#ifndef GPS_DRIVER_H
#define GPS_DRIVER_H

#include <HardwareSerial.h>
#include <TinyGPSPlus.h>
#include <cstdint>

class GPS_Driver {
public:
  GPS_Driver() : serial_gps(1) {} // UART1

  void begin(int espRx, int espTx, uint32_t baud = 9600) {
    serial_gps.begin(baud, SERIAL_8N1, espRx, espTx);
  }

  bool available() { return serial_gps.available(); }

  void read() {
    while (serial_gps.available()) {
      gps.encode(serial_gps.read());
    }
  }

  bool locationUpdated() { return gps.location.isUpdated(); }

  double latitude() { return gps.location.lat(); }

  double longitude() { return gps.location.lng(); }

  bool hasFix() { return gps.location.isValid(); }

  int satellites() { return gps.satellites.value(); }

  double hdop() { return gps.hdop.hdop(); }

  void powerDown() {
    serial_gps.end(); // Close serial connection
  }

private:
  HardwareSerial serial_gps;
  TinyGPSPlus gps;
};

#endif // !GPS_DRIVER_H
