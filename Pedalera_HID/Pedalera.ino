#include <Arduino.h>
#include "Adafruit_TinyUSB.h"

#define ACCEL_PIN     13  
#define BRAKE_PIN     12  
#define BUTTON1_PIN   4
#define BUTTON2_PIN   5

#define SMOOTHING_WINDOW 10

#define ACCEL_MIN 2000
#define ACCEL_MAX 3500
#define BRAKE_MIN 2000
#define BRAKE_MAX 3500

int accelBuffer[SMOOTHING_WINDOW] = {0};
int brakeBuffer[SMOOTHING_WINDOW] = {0};
int accelIndex = 0;
int brakeIndex = 0;

uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_GAMEPAD()
};

Adafruit_USBD_HID usb_hid;
hid_gamepad_report_t gp;

void setup() {
  Serial.begin(115200);

  pinMode(ACCEL_PIN, INPUT);
  pinMode(BRAKE_PIN, INPUT);
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  usb_hid.setPollInterval(2);
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usb_hid.begin();  

  TinyUSBDevice.begin();  
  Serial.println("Funcionando");
}



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

void loop() {
  #ifdef TINYUSB_NEED_POLLING_TASK
  TinyUSBDevice.task();
  #endif

  if (!TinyUSBDevice.mounted()) {
    return;
  }

  if (!usb_hid.ready()) return;

  int rawAccel = analogRead(ACCEL_PIN);
  int rawBrake = analogRead(BRAKE_PIN);

  int smoothAccel = smooth(accelBuffer, rawAccel, accelIndex);
  int smoothBrake = smooth(brakeBuffer, rawBrake, brakeIndex);

  int8_t accelValue = mapInput(smoothAccel, ACCEL_MIN, ACCEL_MAX);
  int8_t brakeValue = mapInput(smoothBrake, BRAKE_MIN, BRAKE_MAX);

  bool btn1 = !digitalRead(BUTTON1_PIN);
  bool btn2 = !digitalRead(BUTTON2_PIN);

  memset(&gp, 0, sizeof(gp));
  gp.x = accelValue;  
  gp.y = brakeValue;  
  gp.buttons = (btn1 << 0) | (btn2 << 1);

  usb_hid.sendReport(0, &gp, sizeof(gp));

  delay(10); 
}