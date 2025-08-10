#ifndef LORA_DRIVER_H
#define LORA_DRIVER_H

#include <Arduino.h>
#include <LoRa.h>
#include <SPI.h>
#include <cstddef>
#include <cstdint>

class LoRaDriver {
public:
  LoRaDriver(uint8_t csPin, uint8_t rstPin, uint8_t dio0Pin,
             long frequency = 433E6)
      : _csPin(csPin), _rstPin(rstPin), _dio0Pin(dio0Pin),
        _frequency(frequency), _initialized(false) {}

  bool begin() {
    LoRa.setPins(_csPin, _rstPin, _dio0Pin);
    if (!LoRa.begin(_frequency)) {
      Serial.println("LoRa init failed. Check wiring.");
      _initialized = false;
      return false;
    }
    Serial.println("LoRa initialized");
    _initialized = true;
    return true;
  }

  // send string or byte
  bool sendPacket(const String &data) {
    if (!_initialized)
      return false;
    LoRa.beginPacket();
    LoRa.print(data);
    LoRa.endPacket();
    Serial.println("Packet sent: " + data);
    return true;
  }

  // send binary/raw buffer packets
  bool sendPacket(const uint8_t *buffer, size_t length) {
    if (!_initialized)
      return false;
    LoRa.beginPacket();
    LoRa.write(buffer, length);
    LoRa.endPacket();
    Serial.println("Binary packet sent.");
    return true;
  }

  bool isInitialized() const { return _initialized };

private:
  uint8_t _csPin, _rstPin, _dio0Pin;
  long _frequency;
  bool _initialized;
};

#endif // !LORA_DRIVER_H
