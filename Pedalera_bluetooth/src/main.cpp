#include <Arduino.h>
#include <BleGamepad.h>

BleGamepad bleGamepad;

const int battery = 1;                 // Batería
const int accelPin = 4;                // Acelerador
const int brakePin = 5;                // Freno
const int button1Pin = 2;              // Botón 1
const int button2Pin = 3;              // Botón 2

const int numberOfSamples = 5;         // Número de muestras para suavizar
const int delayBetweenSamples = 4;     // Retardo entre muestras (ms)
const int delayBetweenReports = 5;     // Retardo entre reportes HID (ms)

void setup()
{
    Serial.begin(115200);
    Serial.println("Iniciando BLE Gamepad...");
    pinMode(button1Pin, INPUT_PULLUP);
    pinMode(button2Pin, INPUT_PULLUP);
    bleGamepad.begin();
}

void loop()
{
    if (bleGamepad.isConnected())
    {
        // --- Lectura de la batería ---
        float adc_value = analogRead(battery);
        float vadc = (adc_value / 4095.0) * 3.3;
        float vbattery = vadc * 2; 
        float percentage = ((vbattery - 3.0) / (4.2 - 3.0)) * 100.0;
        if (percentage < 0) percentage = 0;
        if (percentage > 100) percentage = 100;

        bleGamepad.setBatteryLevel((uint8_t)percentage);

        // --- Lectura del acelerador ---
        int accelSum = 0;
        for (int i = 0; i < numberOfSamples; i++)
        {
            accelSum += analogRead(accelPin);
            delay(delayBetweenSamples);
        }
        int accelAvg = accelSum / numberOfSamples;
        int accelMapped = map(accelAvg, 0, 4095, 0, 32767);

        // --- Lectura del freno ---
        int brakeSum = 0;
        for (int i = 0; i < numberOfSamples; i++)
        {
            brakeSum += analogRead(brakePin);
            delay(delayBetweenSamples);
        }
        int brakeAvg = brakeSum / numberOfSamples;
        int brakeMapped = map(brakeAvg, 0, 4095, 0, 32767);

        // --- Lectura de botones ---
        bool btn1 = !digitalRead(button1Pin);
        bool btn2 = !digitalRead(button2Pin);

        // --- Enviar datos al gamepad ---
        bleGamepad.setSlider1(accelMapped);
        bleGamepad.setSlider2(brakeMapped);

        if (btn1) bleGamepad.press(BUTTON_1); else bleGamepad.release(BUTTON_1);
        if (btn2) bleGamepad.press(BUTTON_2); else bleGamepad.release(BUTTON_2);

        delay(delayBetweenReports);
    }
}