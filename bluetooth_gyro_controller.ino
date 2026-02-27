// // (c) Michael Schoeffler 2017, http://www.mschoeffler.de

#include "Wire.h"  // This library allows you to communicate with I2C devices.
#include <BleGamepad.h>

BleGamepad bleGamepad("ESP32 BLE Gamepad", "One Man Hit Parade", 100);

float smoothed_accel_y = 0;
const float smoothingFactor = 0.3;

int IDLE_LOWEST = -2300;
int IDLE_HIGHEST = 1200;

int SLIGHT_LEFT_LOWEST = -6000;
int SLIGHT_LEFT_HIGHEST = -2300;

int TOTAL_LEFT_HIGHEST = -6000;

int SLIGHT_RIGHT_LOWEST = 1200;
int SLIGHT_RIGHT_HIGHEST = 5500;

int TOTAL_RIGHT_LOWEST = 5500;

const int buttonW = 32;
const int buttonS = 35;

int xAxis = 32736 / 2;

#define SDA_PIN 26
#define SCL_PIN 25

unsigned long accelStartTime = 0;
bool accelRamping = false;

unsigned long brakeStartTime = 0;
bool brakeRamping = false;

const unsigned long rampDuration = 1000;

const int MPU_ADDR = 0x68;

int16_t accelerometer_x, accelerometer_y, accelerometer_z;
int16_t gyro_x, gyro_y, gyro_z;
int16_t temperature;

char tmp_str[7]; 

char* convert_int16_to_str(int16_t i) { 
  sprintf(tmp_str, "%6d", i);
  return tmp_str;
}

void setup() {
  Serial.begin(115200);
  pinMode(buttonW, INPUT);
  pinMode(buttonS, INPUT);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  Serial.println("Starting BLE...");

  bleGamepad.begin();

  Serial.println("BLE Started");
}

void loop() {
  if (bleGamepad.isConnected()) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, 7 * 2, true);

    accelerometer_x = Wire.read() << 8 | Wire.read();
    accelerometer_y = Wire.read() << 8 | Wire.read();

    smoothed_accel_y = smoothingFactor * accelerometer_y + (1 - smoothingFactor) * smoothed_accel_y;

    int filtered_y = (int)smoothed_accel_y;

    int tilt = 0;
    if (filtered_y > IDLE_LOWEST && filtered_y < IDLE_HIGHEST) {
      tilt = 32736 / 2;
    } else if (filtered_y >= SLIGHT_RIGHT_LOWEST && filtered_y <= SLIGHT_RIGHT_HIGHEST) {
      tilt = map(filtered_y, SLIGHT_RIGHT_LOWEST, SLIGHT_RIGHT_HIGHEST, 32736 / 2, 32736);
    } else if (filtered_y > TOTAL_RIGHT_LOWEST) {
      tilt = 32737;
    } else if (filtered_y >= SLIGHT_LEFT_LOWEST && filtered_y <= SLIGHT_LEFT_HIGHEST) {
      tilt = 32736 / 2 - map(filtered_y, SLIGHT_LEFT_LOWEST, SLIGHT_LEFT_HIGHEST, 32736 / 2, 0);
    } else if (filtered_y <= TOTAL_LEFT_HIGHEST) {
      tilt = 0;
    }

    xAxis = tilt;

    if (digitalRead(buttonW)) {
      bleGamepad.press(8);
    } else {
      bleGamepad.release(8);
    }

    if (digitalRead(buttonS)) {
      bleGamepad.press(7);
    } else {
      bleGamepad.release(7);
    }

    bleGamepad.setAxes(xAxis, 32736 / 2, 32736 / 2, 32736 / 2, 32736 / 2, 32736 / 2, 0, 0);
  }
}
