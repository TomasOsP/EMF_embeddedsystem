#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MLX90393.h>

// Crea el objeto del sensor (sin DRDY por ahora, usa lecturas timed)
Adafruit_MLX90393 mlx = Adafruit_MLX90393();

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);  // Espera a que se abra el monitor serial

  Serial.println("Adafruit MLX90393 Test!");

  if (!mlx.begin_I2C()) {  // Inicia I2C con dirección por defecto 0x0C
    // Puedes cambiar la dirección: mlx.begin_I2C(0x0C, &Wire, 0x0C);
    Serial.println("¡Error! No se encontró el sensor MLX90393. Revisa conexiones.");
    while (1) delay(10);
  }
  Serial.println("Sensor MLX90393 encontrado!");

  // Configuración opcional: ganancia y resolución (ver datasheet para más opciones)
  // mlx.setGain(MLX90393_GAIN_1X);  // Ganancia 1x (rango ~±50mT)
  // mlx.setResolution(MLX90393_RES_16);  // 16-bit resolución
}

void loop() {
  float x, y, z;   // Valores en µT

  if (mlx.readData(&x, &y, &z)) {
    Serial.print("X: "); Serial.print(x); Serial.print(" µT\t");
    Serial.print("Y: "); Serial.print(y); Serial.print(" µT\t");
    Serial.print("Z: "); Serial.print(z); Serial.print(" µT\t");
    Serial.println();
  } else {
    Serial.println("Error en lectura. Revisa wiring.");
  }

  delay(500);  // Lee cada 500ms
}