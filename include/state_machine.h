#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "bmp280_driver.h"
#include "buzzer_driver.h"
#include "compass_driver.h"
#include "dht11_driver.h"
#include "gps_driver.h"
#include "mpu6050_driver.h"
#include "sdcard_driver.h"

enum FlightState { PRELAUNCH, ASCENT, DESCENT, POSTLAND };

void stateMachineInit(BMP280_Driver &bmp, DHT11_Driver &dht,
                      MPU6050_Driver &mpu, Compass_Driver &compass,
                      GPS_Driver &gps, Buzzer_Driver &buzzer,
                      SDCard_Driver &sdcard);
void stateMachineUpdate();

#endif // !STATE_MACHINE_H
