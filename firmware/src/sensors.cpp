#include <Arduino.h>
#include "sensors.h"
#include <Wire.h>
#include <Adafruit_MLX90393.h>

// Vector3 struct is defined in sensors.h

//#define MLX90393_HALL_CONF (0x0C)
//#define MLX90393_REG_SB (0x10)

Adafruit_MLX90393 mlx = Adafruit_MLX90393();

float offsetX = 0.0;  // Ajusta estos offsets midiendo en campo cero
float offsetY = 0.0;
float offsetZ = 0.0;

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
  mlx.setOversampling(MLX90393_OSR_1); // Máximo sobremuestreo (reduce ruido, aumenta tiempo)
  mlx.setFilter(MLX90393_FILTER_1);    // Máximo filtro (reduce ruido, ODR ~5 Hz)

  // Para AC rápida: Comenta arriba y usa:
  // mlx.setOversampling(MLX90393_OSR_1);
  // mlx.setFilter(MLX90393_FILTER_1);  // ODR >700 Hz

  // Modo Burst para monitoreo continuo (opcional, ajusta intervalo)
  // mlx.startBurstMode(MLX90393_X | MLX90393_Y | MLX90393_Z | MLX90393_TEMP);

}

Vector3 getMagneticField() {
  float x, y, z;  // En µT
  Vector3 campo = {0.0, 0.0, 0.0};

  if (mlx.readData(&x, &y, &z)) {
    // Aplica calibración simple
    x -= offsetX;
    y -= offsetY;
    z -= offsetZ;
    campo = {x, y, z};
  } else {
    Serial.println("Error en lectura del sensor.");
  }

  return campo;
}