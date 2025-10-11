#include <Arduino.h>
#include "sensors.h"
#include "display.h"


void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  Serial.println("Iniciando sensor...");
  initSensor();
  displayInit();
  displayPrint("Sensor listo");
  delay(1000);
}

void loop() {
  static unsigned long lastMicros = 0;
  unsigned long now = micros();

  if (now - lastMicros >= 2450) {  // 408 Hz
    lastMicros = now;

    Vector3 campo = getMagneticField();
    // Serial.printf("%f,%f,%f\n", campo.x, campo.y, campo.z);
    uint8_t buffer[12];
    memcpy(buffer,     &campo.x, 4);
    memcpy(buffer + 4, &campo.y, 4);
    memcpy(buffer + 8, &campo.z, 4);

    Serial.write(buffer, 12);
    // delay(1);  // Pequeña pausa para evitar saturación del buffer serial
  }
}