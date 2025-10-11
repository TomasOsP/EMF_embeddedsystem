#pragma once
#include <Arduino.h>
#include "sensors.h"

// Estructura con resultados del procesamiento
typedef struct {
  float magnitude;   // Por ejemplo, magnitud o potencia espectral
} FFTData_t;

// Cola global para enviar resultados al display / WiFi
extern QueueHandle_t fftQueue;

// Inicialización y tarea
void initFFT();
void TaskFFT(void *pvParameters);
