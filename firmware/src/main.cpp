#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MLX90393.h>

Adafruit_MLX90393 mlx = Adafruit_MLX90393();

float offsetX = 0.0;  // Ajusta estos offsets midiendo en campo cero
float offsetY = 0.0;
float offsetZ = 0.0;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  if (!mlx.begin_I2C()) {
    Serial.println("Error: Sensor no encontrado.");
    while (1) delay(10);
  }
  Serial.println("Sensor encontrado!");

  // Configuración para alta precisión (bajo ruido, para DC/baja freq)
  mlx.setGain(MLX90393_GAIN_1X);       // Rango amplio: ±50 mT (elige 5X para ±5 mT si campos débiles)
  mlx.setResolution(MLX90393_X, MLX90393_RES_19);  // Máxima resolución (~0.15 µT/LSB en alta ganancia)
  mlx.setResolution(MLX90393_Y, MLX90393_RES_19);  // Máxima resolución (~0.15 µT/LSB en alta ganancia)
  mlx.setResolution(MLX90393_Z, MLX90393_RES_19);  // Máxima resolución (~0.15 µT/LSB en alta ganancia)
  mlx.setOversampling(MLX90393_OSR_3); // Máximo sobremuestreo (reduce ruido, aumenta tiempo)
  mlx.setFilter(MLX90393_FILTER_7);    // Máximo filtro (reduce ruido, ODR ~5 Hz)

  // Para AC rápida: Comenta arriba y usa:
  // mlx.setOversampling(MLX90393_OSR_1);
  // mlx.setFilter(MLX90393_FILTER_1);  // ODR >700 Hz

  // Modo Burst para monitoreo continuo (opcional, ajusta intervalo)
  // mlx.startBurstMode(MLX90393_X | MLX90393_Y | MLX90393_Z | MLX90393_TEMP);
}

void loop() {
  float x, y, z;  // En µT

  if (mlx.readData(&x, &y, &z)) {
    // Aplica calibración simple
    x -= offsetX;
    y -= offsetY;
    z -= offsetZ;

    // Calcula magnitud isotrópica (para cumplimiento)
    float magnitude = sqrt(x*x + y*y + z*z);

    if (magnitude > 200.0) Serial.println("Alerta: Excede 200 µT (público)!");
    if (magnitude > 1000.0) Serial.println("Alerta: Excede 1000 µT (ocupacional)!");

    Serial.print("X: "); Serial.print(x, 2); Serial.print(" µT\t");
    Serial.print("Y: "); Serial.print(y, 2); Serial.print(" µT\t");
    Serial.print("Z: "); Serial.print(z, 2); Serial.print(" µT\t");
    Serial.println();
    Serial.print("Magnitud: "); Serial.print(magnitude, 2); Serial.print(" µT\t");
    Serial.println();
  } else {
    Serial.println("Error en lectura.");
  }

  delay(500);  // Ajusta según ODR (más rápido para AC)
}