// // (c) Michael Schoeffler 2017, http://www.mschoeffler.de

#include "Wire.h"  // This library allows you to communicate with I2C devices.
#include <BleGamepad.h>

#define numOfButtons 13
#define numOfHatSwitches 0
#define enableX true
#define enableY true
#define enableZ true
#define enableRX true
#define enableRY true
#define enableRZ true
#define enableSlider1 true
#define enableSlider2 true
#define enableRudder false
#define enableThrottle false
#define enableAccelerator false
#define enableBrake false
#define enableSteering false

#define R2_BUTTON 32
#define L2_BUTTON 35

#define NUM_OF_BUTTONS 13


BleGamepad bleGamepad("ESP32 BLE Gamepad", "One Man Hit Parade", 100);

int16_t simMin = 0x8000;
int16_t axesCenter = 0x00;
int16_t simMax = 0x7FFF;

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

#define SDA_PIN 26
#define SCL_PIN 25

int DEADZONE_LOW = -2300;
int DEADZONE_HIGH = 1200;

int MAX_LEFT = -6000;
int MAX_RIGHT = 5500;

const int MPU_ADDR = 0x68;

int16_t accelerometer_x, accelerometer_y, accelerometer_z;
int16_t gyro_x, gyro_y, gyro_z;
int16_t temperature;

char tmp_str[7];  // temporary variable used in convert function

char* convert_int16_to_str(int16_t i) {  // converts int16 to string. Moreover, resulting strings will have the same length in the debug monitor.
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
  BleGamepadConfiguration bleGamepadConfig;
  bleGamepadConfig.setAutoReport(false);
  bleGamepadConfig.setControllerType(CONTROLLER_TYPE_GAMEPAD);
  bleGamepadConfig.setButtonCount(numOfButtons);
  bleGamepadConfig.setWhichAxes(enableX, enableY, enableZ, enableRX, enableRY, enableRZ, enableSlider1, enableSlider2);
  bleGamepadConfig.setWhichSimulationControls(enableRudder, enableThrottle, enableAccelerator, enableBrake, enableSteering);
  bleGamepadConfig.setHatSwitchCount(numOfHatSwitches);
  bleGamepadConfig.setSimulationMin(simMin);
  bleGamepadConfig.setSimulationMax(simMax);

  bleGamepad.begin(&bleGamepadConfig);

  bleGamepad.setBrake(simMin);
  bleGamepad.setAccelerator(simMax);

  Serial.println("BLE Started");
}

void loop() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 7 * 2, true);

  accelerometer_x = Wire.read() << 8 | Wire.read();
  accelerometer_y = Wire.read() << 8 | Wire.read();

  if (bleGamepad.isConnected()) {
    int xAxisValue = 0;
    if (accelerometer_y > DEADZONE_LOW && accelerometer_y < DEADZONE_HIGH) {
      xAxisValue = 0;
    } else {
      xAxisValue = map(accelerometer_y, MAX_LEFT, MAX_RIGHT, -32767, 32767);

      xAxisValue = constrain(xAxisValue, -32767, 32767);
    }

    int rightTrigger = 0;
    int leftTrigger = 0;

    Serial.println(xAxisValue);

    if (digitalRead(buttonW)) {
      bleGamepad.press(8);
      Serial.println("Accer");
    }

    if (digitalRead(buttonS)) {
      bleGamepad.press(7);
      Serial.println("Brake");
    }

    bleGamepad.setX(xAxisValue);
    bleGamepad.setY(0);
    bleGamepad.setZ(0);
    bleGamepad.setRX(0);

    delay(10);
  }

  // if (accelerometer_y > IDLE_LOWEST && accelerometer_y < IDLE_HIGHEST) {
  //   tiltR = 0;
  //   tiltL = 0;
  // } else if (accelerometer_y >= SLIGHT_RIGHT_LOWEST && accelerometer_y <= SLIGHT_RIGHT_HIGHEST) {
  //   tiltR = map(accelerometer_y, SLIGHT_RIGHT_LOWEST, SLIGHT_RIGHT_HIGHEST, 0, 32736);
  //   tiltL = 0;
  // } else if (accelerometer_y > TOTAL_RIGHT_LOWEST) {
  //   tiltR = 32737;
  //   tiltL = 0;
  // } else if (accelerometer_y >= SLIGHT_LEFT_LOWEST && accelerometer_y <= SLIGHT_LEFT_HIGHEST) {
  //   tiltR = 0;
  //   tiltL = map(accelerometer_y, SLIGHT_LEFT_LOWEST, SLIGHT_LEFT_HIGHEST, 0, 32736);
  // } else if (accelerometer_y <= TOTAL_LEFT_HIGHEST) {
  //   tiltR = 0;
  //   tiltL = 32737;
  // }
}
