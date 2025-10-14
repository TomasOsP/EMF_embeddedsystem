#include <Arduino.h>
#include "data_an.h"
#include <math.h>

// Si usas ArduinoFFT, incluye la librería:
#include <arduinoFFT.h>

#define FFT_BUFFER_SIZE 256
#define SAMPLING_FREQUENCY 160.0  // Ajusta según tu frecuencia real de muestreo

float magnitudBuffer[FFT_BUFFER_SIZE];
int bufferIndex = 0;



ArduinoFFT<double> FFT = ArduinoFFT<double>();

void TaskData(void *pvParameters) {
    Vector3 campo;
    double vReal[FFT_BUFFER_SIZE];
    double vImag[FFT_BUFFER_SIZE];

    for (;;) {
        // Espera datos de la cola
        if (xQueueReceive(analysis_queue, &campo, portMAX_DELAY) == pdPASS) {
            // Calcula la magnitud            
            float magnitud = sqrt(campo.x * campo.x + campo.y * campo.y + campo.z * campo.z);

            // Guarda en el buffer circular
            magnitudBuffer[bufferIndex] = magnitud;
            bufferIndex++;

            // Cuando el buffer esté lleno, realiza la FFT
            if (bufferIndex >= FFT_BUFFER_SIZE) {
                // Copia los datos al arreglo de la FFT
                for (int i = 0; i < FFT_BUFFER_SIZE; i++) {
                    vReal[i] = magnitudBuffer[i];
                    vImag[i] = 0.0;
                }

                FFT.windowing(vReal, FFT_BUFFER_SIZE, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
                FFT.compute(vReal, vImag, FFT_BUFFER_SIZE, FFT_FORWARD);
                FFT.complexToMagnitude(vReal, vImag, FFT_BUFFER_SIZE);

                // Busca la frecuencia dominante
                double maxAmp = 0.0;
                int maxIndex = 0;
                for (int i = 1; i < FFT_BUFFER_SIZE / 2; i++) { // Ignora DC (i=0)
                    if (vReal[i] > maxAmp) {
                        maxAmp = vReal[i];
                        maxIndex = i;
                    }
                }
                double dominantFreq = (maxIndex * SAMPLING_FREQUENCY) / FFT_BUFFER_SIZE;

                Serial.print("Frecuencia dominante: ");
                Serial.print(dominantFreq, 2);
                Serial.print(" Hz, Amplitud: ");
                Serial.println(maxAmp, 2);

                bufferIndex = 0; // Reinicia el buffer
            }
        }
    }
}