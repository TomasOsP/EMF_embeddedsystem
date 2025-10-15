#include <Arduino.h>
#include "sensors.h"
#include "display.h"
#include "data_an.h"

// --- Definiciones del Botón ---
#define BUTTON_PIN 33  // Cambio de pantalla

// Semaforo para notiicar el cambio de pantalla
SemaphoreHandle_t buttonSemaphore;

// Función de la interrupcion
void IRAM_ATTR buttonISR() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(buttonSemaphore, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken) {
    portYIELD_FROM_ISR();
  }
}


void setup() {
  Serial.begin(115200);
  initSensor();
  initDisplay();

  // --- Inicializacion del Semáforo y el Botón ---
  buttonSemaphore = xSemaphoreCreateBinary();
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING); // FALLING al presionar

  // --- Creación de Tareas ---
  xTaskCreatePinnedToCore(TaskSensor, "Sensor", 4096, NULL, 3, NULL, 0);
  xTaskCreatePinnedToCore(TaskData, "Data", 4096, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(TaskDisplay, "Display", 4096, NULL, 1, NULL, 0);
}

void loop() {

}