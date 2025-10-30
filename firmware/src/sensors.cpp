#include "sensors.h"
#include <Wire.h>
#include <Adafruit_MLX90393.h>

#define LED_PIN 26
#define BUZZER_PIN 27

const float THRESHOLD_MID = 200.0;   // µT
const float THRESHOLD_HIGH = 1000.0; // µT
const int BLINK_SLOW_MS = 500;
const int BLINK_FAST_MS = 100;

Adafruit_MLX90393 mlx = Adafruit_MLX90393();

// offsets y matriz soft-iron (tus valores)
float offsetX = 36.98;
float offsetY = -122.91;
float offsetZ = 81.51;
float M_soft[3][3] = {
    { 0.981, -0.001, -0.014 },
    { -0.001, 0.976, -0.010 },
    { -0.014, -0.010, 1.045 }
};

// Colas y mutex (extern en header)
QueueHandle_t display_queue;
QueueHandle_t analysis_queue;
QueueHandle_t wifi_queue;

void initSensor() {
  if (!mlx.begin_I2C()) {
    Serial.println("Error: Sensor no encontrado.");
    while (1) delay(10);
  }
  Serial.println("Sensor encontrado!");

  mlx.setGain(MLX90393_GAIN_1X);
  mlx.setResolution(MLX90393_X, MLX90393_RES_16);
  mlx.setResolution(MLX90393_Y, MLX90393_RES_16);
  mlx.setResolution(MLX90393_Z, MLX90393_RES_16);
  mlx.setOversampling(MLX90393_OSR_0);
  mlx.setFilter(MLX90393_FILTER_2);

  display_queue = xQueueCreate(1, sizeof(Vector3));
  analysis_queue = xQueueCreate(10, sizeof(Vector3));
  wifi_queue = xQueueCreate(10, sizeof(Vector3));
  if (display_queue == NULL || analysis_queue == NULL || wifi_queue == NULL) {
    Serial.println("Error creando colas de sensores");
    while (1); 
  }

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
}

Vector3 getMagneticField() {
  float x, y, z;
  Vector3 campo = {0.0, 0.0, 0.0};

  if (mlx.readData(&x, &y, &z)) {
    campo.x = x - offsetX;
    campo.y = y - offsetY;
    campo.z = z - offsetZ;

    // Aplicar matriz soft-iron
    float bx = campo.x;
    float by = campo.y;
    float bz = campo.z;
    campo.x = M_soft[0][0] * bx + M_soft[0][1] * by + M_soft[0][2] * bz;
    campo.y = M_soft[1][0] * bx + M_soft[1][1] * by + M_soft[1][2] * bz;
    campo.z = M_soft[2][0] * bx + M_soft[2][1] * by + M_soft[2][2] * bz;
  } else {
    Serial.println("Error en lectura del sensor.");
  }

  return campo;
}

void TaskSensor(void *pvParameters) {
  Vector3 campo;
  unsigned long lastSampleTime = micros();
  unsigned long currentSampleTime;
  unsigned long deltaTime;
  unsigned long sumDeltaTime = 0;
  int sampleCount = 0;

  static unsigned long lastBlinkTime = 0;
  int currentBlinkDelay = 0;
  bool isAlarmActive = false;
  bool isBufferAlarmActive = false;

  for (;;) {
    campo = getMagneticField();
    float magnitude = sqrt(campo.x * campo.x + campo.y * campo.y + campo.z * campo.z);

    isBufferAlarmActive = (magnitude > THRESHOLD_HIGH);
    isAlarmActive = isBufferAlarmActive || (magnitude > THRESHOLD_MID);

    if (isBufferAlarmActive) {
      currentBlinkDelay = BLINK_FAST_MS;
      digitalWrite(LED_PIN, HIGH);
    } else if (isAlarmActive) {
      currentBlinkDelay = BLINK_SLOW_MS;
      digitalWrite(BUZZER_PIN, LOW);
    } else {
      currentBlinkDelay = 0;
      digitalWrite(LED_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
      lastBlinkTime = 0;
    }

    if (currentBlinkDelay > 0 && (millis() - lastBlinkTime) >= currentBlinkDelay) {
      lastBlinkTime = millis();
      if (!isBufferAlarmActive) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      }
      if (isBufferAlarmActive) {
        digitalWrite(BUZZER_PIN, !digitalRead(BUZZER_PIN));
      }
    }

    // Enviar a colas
    xQueueOverwrite(display_queue, &campo);
    xQueueSend(analysis_queue, &campo, 0);

    // --- SALIDA POR SERIAL EN FORMATO CSV ---
    unsigned long ts = micros(); // timestamp en µs
    if (serialMux != NULL) {
      if (xSemaphoreTake(serialMux, (TickType_t)10) == pdTRUE) {
        // Formato: timestamp_us, X, Y, Z\n  (valores en µT)
        Serial.printf("%lu,%.3f,%.3f,%.3f\n", ts, campo.x, campo.y, campo.z);
        xSemaphoreGive(serialMux);
      }
    } else {
      // Si no hay mutex, igual intenta imprimir (fallback)
      Serial.printf("%lu,%.3f,%.3f,%.3f\n", ts, campo.x, campo.y, campo.z);
    }

    // Calculo de tiempo entre muestras para debug (opcional)
    currentSampleTime = micros();
    deltaTime = currentSampleTime - lastSampleTime;
    lastSampleTime = currentSampleTime;
    sumDeltaTime += deltaTime;
    sampleCount++;

    if (sampleCount >= 256) {
      float avgDeltaMs = sumDeltaTime / 256.0 / 1000.0;
      if (serialMux != NULL) {
        if (xSemaphoreTake(serialMux, (TickType_t)10) == pdTRUE) {
          Serial.print("Tiempo promedio entre muestras: ");
          Serial.print(avgDeltaMs, 3);
          Serial.println(" ms");
          Serial.print("Frecuencia de muestreo: ");
          Serial.print(1.0/avgDeltaMs * 1000.0, 3);
          Serial.println(" Hz");
          xSemaphoreGive(serialMux);
        }
      }
      sumDeltaTime = 0;
      sampleCount = 0;
    }

    // pequeña espera cooperativa para no saturar la CPU
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}
