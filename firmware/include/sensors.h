#pragma once
#include <Arduino.h>

typedef struct {
  float x;
  float y;
  float z;
} Vector3;

// Cola global para enviar los datos del sensor
extern QueueHandle_t display_queue;
extern QueueHandle_t analysis_queue;
extern QueueHandle_t wifi_queue;
extern SemaphoreHandle_t serialMux;

// Prototipos de funciones
void initSensor();
Vector3 getMagneticField();
void TaskSensor(void *pvParameters);
