#pragma once
#include "sensors.h"
#include <Arduino.h>


// Estructura para almacenar los resultados del análisis
typedef struct {
    Vector3 vrms_max;   // El valor RMS máximo detectado para X, Y, Z
    Vector3 vrms_actual; // El valor RMS del último período
    Vector3 avg_6min;   // El promedio del último período de 6 minutos
} AnalysisResults_t;

// Variable global para que todas las tareas puedan leer los resultados
extern AnalysisResults_t analysisResults;


void TaskData(void *pvParameters);

