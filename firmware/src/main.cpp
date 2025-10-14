#include <Arduino.h>
#include "sensors.h"
#include "display.h"
#include "data_an.h"


void setup() {
  Serial.begin(115200);
  initSensor();
  initDisplay();

  xTaskCreatePinnedToCore(TaskSensor, "Sensor", 4096, NULL, 3, NULL, 0);
  xTaskCreatePinnedToCore(TaskData, "Data", 4096, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(TaskDisplay, "Display", 4096, NULL, 1, NULL, 0);
}

void loop() {

}