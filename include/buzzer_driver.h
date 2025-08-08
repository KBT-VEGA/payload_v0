#ifndef BUZZER_DRIVER_H
#define BUZZER_DRIVER_H

#include <Arduino.h>
#include <cstdint>

class Buzzer_Driver {
public:
  Buzzer_Driver(uint8_t pin) : buzzerPin(pin) { pinMode(buzzerPin, OUTPUT); }

  void startTone(unsigned int frequency = 2000) { tone(buzzerPin, frequency); }

  void stopTone() { noTone(buzzerPin); }

  void beep(unsigned int frequency = 2000, unsigned long duration_ms = 1000) {
    tone(buzzerPin, frequency);
    delay(duration_ms);
    noTone(buzzerPin);
    delay(duration_ms);
  }

private:
  uint8_t buzzerPin;
};

#endif // !BUZZER_DRIVER_H
