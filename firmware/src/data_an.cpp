#include <Arduino.h>
#include "data_an.h"
#include "sensors.h"

// Si usas ArduinoFFT, incluye la librería:
#include <arduinoFFT.h>

AnalysisResults_t analysisResults;

// --- Parámetros de Tiempo ---
// Para un muestreo a ~6ms (166 Hz), ~166 muestras por segundo.
const int SAMPLES_PER_SECOND = 160; 

// Período para calcular el RMS (Ej: 1 segundo)
const int RMS_PERIOD_SECONDS = 1;
const int RMS_SAMPLES_COUNT = SAMPLES_PER_SECOND * RMS_PERIOD_SECONDS; 

// Período de Promedio (6 minutos)
const int AVG_PERIOD_SECONDS = 1 * 30;
const int AVG_SAMPLES_COUNT = SAMPLES_PER_SECOND * AVG_PERIOD_SECONDS;

Vector3 avg_buffer[AVG_SAMPLES_COUNT];
// Indice para escribir la nueva muestra
int avg_buffer_index = 0;

// Suma total.
float total_sum_x = 0.0;
float total_sum_y = 0.0;
float total_sum_z = 0.0;

// Bandera para ver si el buffer está lleno
bool is_avg_buffer_full = false;

// ArduinoFFT<double> FFT = ArduinoFFT<double>();

void TaskData(void *pvParameters) {
    Vector3 campo;
    // Variables de Acumulación para el RMS
    float sum_sq_x = 0.0;
    float sum_sq_y = 0.0;
    float sum_sq_z = 0.0;
    int rms_sample_counter = 0;

    // // Variables de Acumulación para el Promedio de 6 minutos
    // float sum_avg_x = 0.0;
    // float sum_avg_y = 0.0;
    // float sum_avg_z = 0.0;
    // int avg_sample_counter = 0;

    for (;;) {
        // Espera datos de la cola
        if (xQueueReceive(analysis_queue, &campo, portMAX_DELAY) == pdPASS) {
            // --- 1. Cálculo del RMS (Se ejecuta con cada muestra) ---
            // Acumular el cuadrado de los valores para el cálculo RMS
            sum_sq_x += campo.x * campo.x;
            sum_sq_y += campo.y * campo.y;
            sum_sq_z += campo.z * campo.z;
            rms_sample_counter++;
            
            // --- 2. Cálculo de promedio móvil ---
            Vector3 oldest_sample = avg_buffer[avg_buffer_index];
            // Restar la muestra más antigua de la suma total
            total_sum_x -= oldest_sample.x;
            total_sum_y -= oldest_sample.y;
            total_sum_z -= oldest_sample.z;
            // Almacenar nueva muestra en el buffer
            avg_buffer[avg_buffer_index] = campo;
            // Añadir la nueva muestra a la suma total
            total_sum_x += campo.x;
            total_sum_y += campo.y;
            total_sum_z += campo.z;
            // Mover el índice del buffer (circular)
            avg_buffer_index++;
            if (avg_buffer_index >= AVG_SAMPLES_COUNT) {
                avg_buffer_index = 0;
                is_avg_buffer_full = true; // El buffer ya está lleno al menos una vez
            }
            // Calcular el promedio solo si el buffer está lleno
            int current_count = is_avg_buffer_full ? AVG_SAMPLES_COUNT : avg_buffer_index;

            if (current_count > 0) {
                analysisResults.avg_6min.x = total_sum_x / current_count;
                analysisResults.avg_6min.y = total_sum_y / current_count;
                analysisResults.avg_6min.z = total_sum_z / current_count;
            } else {
                analysisResults.avg_6min.x = 0.0;
                analysisResults.avg_6min.y = 0.0;
                analysisResults.avg_6min.z = 0.0;
            }


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
                Serial.printf("Promedio Móvil de 6 min: X=%.3f, Y=%.3f, Z=%.3f\n", 
                              analysisResults.avg_6min.x, 
                              analysisResults.avg_6min.y, 
                              analysisResults.avg_6min.z);
            }
        }
    }
}
