#ifndef ARDUINO_USB_MODE 
#error This ESP32 SoC has no Native USB interface
#elif ARDUINO_USB_MODE == 1
#warning This sketch should be used when USB is in OTG mode
void setup() {}
void loop() {}
#else

#include <Arduino.h>
#include "USB.h"
#include "USBHIDGamepad.h"

#define ACCEL_PIN     4 
#define BRAKE_PIN     5  
#define BUTTON1_PIN   2
#define BUTTON2_PIN   3

#define SMOOTHING_WINDOW 3

#define ACCEL_MIN 1108 
#define ACCEL_MAX 1975
#define BRAKE_MIN 1127 
#define BRAKE_MAX 1975

int accelBuffer[SMOOTHING_WINDOW] = {0};
int brakeBuffer[SMOOTHING_WINDOW] = {0};
int accelIndex = 0;
int brakeIndex = 0;

USBHIDGamepad Gamepad;

int smooth(int* buffer, int newValue, int& index) {
  buffer[index] = newValue;
  index = (index + 1) % SMOOTHING_WINDOW;
  long sum = 0;
  for (int i = 0; i < SMOOTHING_WINDOW; i++) {
    sum += buffer[i];
  }
  return sum / SMOOTHING_WINDOW;
}

int8_t mapInput(int val, int in_min, int in_max) {
  val = constrain(val, min(in_min, in_max), max(in_min, in_max));
  return map(val, in_min, in_max, -127, 127);
}

void setup() {
  Serial.begin(115200);
  delay(1000);  

  Serial.print("Motivo del reinicio anterior: ");
  esp_reset_reason_t reason = esp_reset_reason();
  Serial.println(reason);

  pinMode(ACCEL_PIN, INPUT);
  pinMode(BRAKE_PIN, INPUT);
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);

  Gamepad.begin();
  USB.begin();

  Serial.println("USB Gamepad activo usando USBHIDGamepad");
}

int8_t lastAccel = 0;
int8_t lastBrake = 0;
bool lastBtn1 = false;
bool lastBtn2 = false;

void loop() {
  int rawAccel = analogRead(ACCEL_PIN);
  int rawBrake = analogRead(BRAKE_PIN);

  Serial.print("Acelerador: ");
  Serial.print(rawAccel);
  Serial.print("   Freno: ");
  Serial.println(rawBrake); 

  int smoothAccel = smooth(accelBuffer, rawAccel, accelIndex);
  int smoothBrake = smooth(brakeBuffer, rawBrake, brakeIndex);

  int8_t accelValue = mapInput(smoothAccel, ACCEL_MIN, ACCEL_MAX);
  int8_t brakeValue = mapInput(smoothBrake, BRAKE_MIN, BRAKE_MAX);

  accelValue = constrain(accelValue, -127, 127);
  brakeValue = constrain(brakeValue, -127, 127);

  bool btn1 = !digitalRead(BUTTON1_PIN);
  bool btn2 = !digitalRead(BUTTON2_PIN);

  bool update = false;

  if (btn1 != lastBtn1) {
    if (btn1) Gamepad.pressButton(0);
    else Gamepad.releaseButton(0);
    lastBtn1 = btn1;
    update = true;
  }

  if (btn2 != lastBtn2) {
    if (btn2) Gamepad.pressButton(1);
    else Gamepad.releaseButton(1);
    lastBtn2 = btn2;
    update = true;
  }

  if (accelValue != lastAccel || brakeValue != lastBrake) {
    Gamepad.leftTrigger(accelValue);
    Gamepad.rightTrigger(brakeValue);
    lastAccel = accelValue;
    lastBrake = brakeValue;
    update = true;
  }

  if (update) {
    Gamepad.leftStick(0, 0);   
    Gamepad.rightStick(0, 0);
    Gamepad.hat(HAT_CENTER);
    Serial.println("Estado HID actualizado.");
  }

  delay(50);
}

#endif