#ifndef SDCARD_DRIVER_H
#define SDCARD_DRIVER_H

#include <Arduino.h>
#include <SD.h>
#include <cstdint>

class SDCard_Driver {
public:
  // Constructor with default chip select pin (common for many ESP32 dev boards)
  explicit SDCard_Driver(uint8_t csPin = 5)
      : _csPin(csPin), _initialized(false) {}

  // Initialize SD card; return true on success
  bool begin(uint8_t csPin) {
    if (SD.begin(csPin)) {
      _initialized = true;
      return true;
    }
    _initialized = false;
    return false;
  }

  // Append a line (with newline) to a file
  bool writeLine(const String &fileName, const String &data) {
    if (!_initialized)
      return false;
    File file = SD.open(fileName, FILE_APPEND);
    if (!file)
      return false;
    file.println(data);
    file.close();
    return true;
  }

  // Read entire file contents as String
  String readFile(const String &fileName) {
    if (!_initialized)
      return "";
    if (!SD.exists(fileName))
      return "";
    File file = SD.open(fileName, FILE_READ);
    if (!file)
      return "";

    String content;
    while (file.available()) {
      content += (char)file.read();
    }
    file.close();
    return content;
  }

  // Delete all files in root directory
  // Returns number of files deleted or -1 on error
  int deleteAllFiles() {
    if (!_initialized)
      return -1;
    File root = SD.open("/");
    if (!root)
      return -1;
    if (!root.isDirectory()) {
      root.close();
      return -1;
    }

    int deletedCount = 0;
    File file = root.openNextFile();
    while (file) {
      if (!file.isDirectory()) {
        String filename = file.name();
        file.close();
        if (SD.remove(filename)) {
          deletedCount++;
        }
      } else {
        file.close();
      }
      file = root.openNextFile();
    }
    root.close();
    return deletedCount;
  }

  // Prints all filenames in root directory to Serial
  void listFilesToSerial() {
    if (!_initialized) {
      Serial.println("SD not initialized");
      return;
    }
    File root = SD.open("/");
    if (!root) {
      Serial.println("Failed to open root directory");
      return;
    }
    if (!root.isDirectory()) {
      Serial.println("Root is not a directory");
      root.close();
      return;
    }
    Serial.println("Files on SD card:");
    File file = root.openNextFile();
    while (file) {
      if (!file.isDirectory()) {
        Serial.println(file.name());
      }
      file = root.openNextFile();
    }
    root.close();
  }

  // Check if SD is initialized and ready
  bool isInitialized() const { return _initialized; }

private:
  uint8_t _csPin;
  bool _initialized;
};

#endif // SDCARD_DRIVER_H
