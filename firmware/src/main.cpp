#include <Arduino.h>
#include "sensors.h"
#include "display.h"
#include "fft.h"


void setup() {
  Serial.begin(115200);
  initSensor();
  initDisplay();

  xTaskCreatePinnedToCore(TaskSensor, "Sensor", 4096, NULL, 3, NULL, 0);
  xTaskCreatePinnedToCore(TaskFFT, "FFT", 4096, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(TaskDisplay, "Display", 4096, NULL, 1, NULL, 1);
}

void loop() {

}