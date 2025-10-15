#include <Arduino.h>
#include "sensors.h"
#include <Wire.h>
#include <Adafruit_MLX90393.h>

// Vector3 struct is defined in sensors.h

#define LED_PIN 26
#define BUZZER_PIN 27

// Umbrales de magnitud 
const float THRESHOLD_MID = 200.0;   // µT
const float THRESHOLD_HIGH = 1000.0; // µT

// Tiempo de Parpadeo
const int BLINK_SLOW_MS = 500; // Duración del parpadeo
const int BLINK_FAST_MS = 100; // Intervalo entre parpadeos
//#define MLX90393_HALL_CONF (0x0C)
//#define MLX90393_REG_SB (0x10)

Adafruit_MLX90393 mlx = Adafruit_MLX90393();

// --- Calibración simple (offsets) ---
// Falta mejorar con calibración completa
float offsetX = 0.0;  
float offsetY = 0.0;
float offsetZ = 0.0;


// --- Colas globales para cada tarea consumidora ---
QueueHandle_t display_queue;
QueueHandle_t analysis_queue;
QueueHandle_t wifi_queue;

// Inicializa el sensor MLX90393 con configuración óptima
void initSensor() {

  if (!mlx.begin_I2C()) {
    Serial.println("Error: Sensor no encontrado.");
    while (1) delay(10);
  }
  Serial.println("Sensor encontrado!");

  // Configuración para alta precisión (bajo ruido, para DC/baja freq)
  mlx.setGain(MLX90393_GAIN_1X);       // Rango amplio: ±50 mT (elige 5X para ±5 mT si campos débiles)
  mlx.setResolution(MLX90393_X, MLX90393_RES_16);  // Máxima resolución (~0.15 µT/LSB en alta ganancia)
  mlx.setResolution(MLX90393_Y, MLX90393_RES_16);  // Máxima resolución (~0.15 µT/LSB en alta ganancia)
  mlx.setResolution(MLX90393_Z, MLX90393_RES_16);  // Máxima resolución (~0.15 µT/LSB en alta ganancia)
  mlx.setOversampling(MLX90393_OSR_0); // Máximo sobremuestreo (reduce ruido, aumenta tiempo)
  mlx.setFilter(MLX90393_FILTER_1);    // Máximo filtro (reduce ruido, ODR ~5 Hz)

  // --- Creación de las Colas Individuales ---
  display_queue = xQueueCreate(1, sizeof(Vector3));
  analysis_queue = xQueueCreate(10, sizeof(Vector3));
  wifi_queue = xQueueCreate(10, sizeof(Vector3));
  if (display_queue == NULL || analysis_queue == NULL || wifi_queue == NULL) {
    Serial.println("Error creando colas de sensores");
    while (1);
  }

  // --- Inicialización de pines ---
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
}
// ---------------------------------------------------------
// Lectura de datos del sensor (una sola muestra)
// ---------------------------------------------------------
Vector3 getMagneticField() {
  float x, y, z;  // En µT
  Vector3 campo = {0.0, 0.0, 0.0};
  
  if (mlx.readData(&x, &y, &z)) {
    // Aplica calibración simple
    campo.x = x - offsetX;
    campo.y = y - offsetY;
    campo.z = z - offsetZ;
  } else {
    Serial.println("Error en lectura del sensor.");
  }

  return campo;
}

// ---------------------------------------------------------
// Tarea FreeRTOS: lectura periódica del sensor
// ---------------------------------------------------------
void TaskSensor(void *pvParameters) {
  Vector3 campo;
  unsigned long lastSampleTime = micros();
  unsigned long currentSampleTime;
  unsigned long deltaTime;
  unsigned long sumDeltaTime = 0;
  int sampleCount = 0;

  // Variables para la lógica de parpadeo no bloqueante
  static unsigned long lastBlinkTime = 0;
  int currentBlinkDelay = 0;
  bool isAlarmActive = false; // Indica si alguna alarma está activa
  bool isBufferAlarmActive = false; // Indica si la alarma de buffer está activa

  for (;;) {
    campo = getMagneticField();
    float magnitude = sqrt(campo.x * campo.x + campo.y * campo.y + campo.z * campo.z);

    // 2. Determinar el estado de las alarmas
    isBufferAlarmActive = (magnitude > THRESHOLD_HIGH);
    isAlarmActive = isBufferAlarmActive || (magnitude > THRESHOLD_MID);

    if (isBufferAlarmActive) {
      // Prioridad 1: Alarma de Buffer/Búfer (Señal Sonora Intermitente y LED Rápido)
      currentBlinkDelay = BLINK_FAST_MS;
      digitalWrite(LED_PIN, HIGH); // El LED principal se enciende fijo
    } else if (isAlarmActive) {
      // Prioridad 2: Alarma de Magnitud 200 (LED Lento)
      currentBlinkDelay = BLINK_SLOW_MS;
      digitalWrite(BUZZER_PIN, LOW); // Apaga el buzzer
    } else {
      // Sin Alarma
      currentBlinkDelay = 0;
      digitalWrite(LED_PIN, LOW); // Apaga el LED principal
      digitalWrite(BUZZER_PIN, LOW); // Apaga el buzzer
      lastBlinkTime = 0; // Reinicia el tiempo de parpadeo
    }
    
    // 3. Lógica de Parpadeo y Sonido No Bloqueante
    if (currentBlinkDelay > 0 && (millis() - lastBlinkTime) >= currentBlinkDelay) {
      lastBlinkTime = millis();
      
      // Control del LED de Alarma
      if (!isBufferAlarmActive) {
         // Parpadeo Lento para THRESHOLD_MID
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      }

      // Control de la Señal Sonora Intermitente (y LED Rápido)
      if (isBufferAlarmActive) {
        // Parpadeo Rápido y Sonido Intermitente para THRESHOLD_HIGH
        digitalWrite(BUZZER_PIN, !digitalRead(BUZZER_PIN));
      }
    }
    // Envía los datos a las colas (no bloqueante)
    xQueueOverwrite(display_queue, &campo);
    xQueueSend(analysis_queue, &campo, 0);
    // Calcular tiempo entre muestras
    currentSampleTime = micros();
    deltaTime = currentSampleTime - lastSampleTime;
    lastSampleTime = currentSampleTime;
    sumDeltaTime += deltaTime;
    sampleCount++;

    // Cada 256 muestras, imprime el tiempo promedio entre muestras
    if (sampleCount >= 256) {
      float avgDeltaMs = sumDeltaTime / 256.0 / 1000.0; // en ms
      Serial.print("Tiempo promedio entre muestras: ");
      Serial.print(avgDeltaMs, 3);
      Serial.println(" ms");
      Serial.print("Frecuencia de muestreo: ");
      Serial.print(1.0/avgDeltaMs * 1000.0, 3);
      Serial.println(" Hz");
      sumDeltaTime = 0;
      sampleCount = 0;
    }
  }
}