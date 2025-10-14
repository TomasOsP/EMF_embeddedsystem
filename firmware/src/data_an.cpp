#include <Arduino.h>
#include "data_an.h"
#include "sensors.h"

// Si usas ArduinoFFT, incluye la librería:
#include <arduinoFFT.h>

AnalysisResults_t analysisResults;

// --- Parámetros de Tiempo ---
// Para un muestreo a ~6ms (166 Hz), ~166 muestras por segundo.
const int SAMPLES_PER_SECOND = 166; 

// Período para calcular el RMS (Ej: 1 segundo)
const int RMS_PERIOD_SECONDS = 1;
const int RMS_SAMPLES_COUNT = SAMPLES_PER_SECOND * RMS_PERIOD_SECONDS; 

// Período de Promedio (6 minutos)
const int AVG_PERIOD_SECONDS = 6 * 60;
const int AVG_SAMPLES_COUNT = SAMPLES_PER_SECOND * AVG_PERIOD_SECONDS;


// ArduinoFFT<double> FFT = ArduinoFFT<double>();

void TaskData(void *pvParameters) {
    Vector3 campo;
    // Variables de Acumulación para el RMS
    float sum_sq_x = 0.0;
    float sum_sq_y = 0.0;
    float sum_sq_z = 0.0;
    int rms_sample_counter = 0;

    // Variables de Acumulación para el Promedio de 6 minutos
    float sum_avg_x = 0.0;
    float sum_avg_y = 0.0;
    float sum_avg_z = 0.0;
    int avg_sample_counter = 0;

    for (;;) {
        // Espera datos de la cola
        if (xQueueReceive(analysis_queue, &campo, portMAX_DELAY) == pdPASS) {
            // --- 1. Cálculo del RMS (Se ejecuta con cada muestra) ---
            // Acumular el cuadrado de los valores para el cálculo RMS
            sum_sq_x += campo.x * campo.x;
            sum_sq_y += campo.y * campo.y;
            sum_sq_z += campo.z * campo.z;
            rms_sample_counter++;
            
            // --- 2. Acumulación para el Promedio de 6 min ---
            sum_avg_x += campo.x;
            sum_avg_y += campo.y;
            sum_avg_z += campo.z;
            avg_sample_counter++;

            // --- 3. Finalización del Período RMS (Ej: cada 1 segundo) ---
            if (rms_sample_counter >= RMS_SAMPLES_COUNT) {
                    
                // Calcular el RMS (Root Mean Square) actual
                analysisResults.vrms_actual.x = sqrt(sum_sq_x / rms_sample_counter);
                analysisResults.vrms_actual.y = sqrt(sum_sq_y / rms_sample_counter);
                analysisResults.vrms_actual.z = sqrt(sum_sq_z / rms_sample_counter);
                
                // --- Determinar el RMS Máximo ---
                if (analysisResults.vrms_actual.x > analysisResults.vrms_max.x) {
                    analysisResults.vrms_max.x = analysisResults.vrms_actual.x;
                }
                if (analysisResults.vrms_actual.y > analysisResults.vrms_max.y) {
                    analysisResults.vrms_max.y = analysisResults.vrms_actual.y;
                }
                if (analysisResults.vrms_actual.z > analysisResults.vrms_max.z) {
                    analysisResults.vrms_max.z = analysisResults.vrms_actual.z;
                }

                // Reiniciar los acumuladores del RMS
                sum_sq_x = 0.0;
                sum_sq_y = 0.0;
                sum_sq_z = 0.0;
                rms_sample_counter = 0;
                Serial.printf("RMS Actual: X=%.3f, Y=%.3f, Z=%.3f | RMS Máx: X=%.3f, Y=%.3f, Z=%.3f\n", 
                              analysisResults.vrms_actual.x, 
                              analysisResults.vrms_actual.y, 
                              analysisResults.vrms_actual.z,
                              analysisResults.vrms_max.x, 
                              analysisResults.vrms_max.y, 
                              analysisResults.vrms_max.z);
            }
        // --- 4. Finalización del Período de Promedio (cada 6 minutos) ---
            if (avg_sample_counter >= AVG_SAMPLES_COUNT) {
                
                // Calcular el Promedio (Suma / Cantidad)
                analysisResults.avg_6min.x = sum_avg_x / avg_sample_counter;
                analysisResults.avg_6min.y = sum_avg_y / avg_sample_counter;
                analysisResults.avg_6min.z = sum_avg_z / avg_sample_counter;
                
                Serial.printf("Nuevo Promedio de 6 min: X=%.3f, Y=%.3f, Z=%.3f\n", 
                              analysisResults.avg_6min.x, 
                              analysisResults.avg_6min.y, 
                              analysisResults.avg_6min.z);

                // Reiniciar los acumuladores del Promedio
                sum_avg_x = 0.0;
                sum_avg_y = 0.0;
                sum_avg_z = 0.0;
                avg_sample_counter = 0;
            }
        }
    }
}
