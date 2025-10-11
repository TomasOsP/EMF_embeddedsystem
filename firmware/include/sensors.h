#pragma once
#include <Arduino.h>

typedef struct {
  float x;
  float y;
  float z;
} Vector3;

// Cola global para enviar los datos del sensor
extern QueueHandle_t sensorQueue;

// Prototipos de funciones
void initSensor();
Vector3 getMagneticField();
void TaskSensor(void *pvParameters);
