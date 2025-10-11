#include <Arduino.h>
#include "fft.h"
#include "sensors.h"

// Cola donde se publican los resultados de la FFT
QueueHandle_t fftQueue;

// ---------------------------------------------------------
// Inicialización del módulo FFT
// ---------------------------------------------------------
void initFFT() {
  fftQueue = xQueueCreate(10, sizeof(FFTData_t));
  if (fftQueue == NULL) {
    Serial.println("Error creando fftQueue");
    while (1);
  }
  Serial.println("FFT inicializada");
}

// ---------------------------------------------------------
// Ejemplo: procesamiento simple (magnitud del campo)
// Puedes reemplazarlo por FFT real si usas una librería FFT
// ---------------------------------------------------------
float computeMagnitude(Vector3 campo) {
  return sqrt(campo.x * campo.x + campo.y * campo.y + campo.z * campo.z);
}

// ---------------------------------------------------------
// Tarea FreeRTOS: lee de sensorQueue y procesa
// ---------------------------------------------------------
void TaskFFT(void *pvParameters) {
  Vector3 campo;
  FFTData_t fftData;

  for (;;) {
    // Espera nuevos datos del sensor
    if (xQueueReceive(sensorQueue, &campo, portMAX_DELAY)) {

      // 🔹 Aquí haces el procesamiento
      // Ejemplo: calcular magnitud instantánea
      fftData.magnitude = computeMagnitude(campo);

      // (Opcional) podrías hacer acumulación para FFT
      // o guardar muestras en un buffer circular antes

      // Enviar a la cola FFT
      xQueueSend(fftQueue, &fftData, 0);
    }
  }
}
