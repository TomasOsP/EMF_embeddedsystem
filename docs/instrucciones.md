# Instrucciones Detalladas

## Conexión del Sensor

Conectar el MLX90393 a la ESP32:
- VCC → 3.3V
- GND → GND
- SDA → GPIO21
- SCL → GPIO22

## Configuración

### Calibración
1. Colocar el sensor en campo cero
2. Medir offsets
3. Ajustar offsetX, offsetY, offsetZ en el código

### Puertos Seriales
- Windows: COM3, COM4, etc.
- Linux: /dev/ttyUSB0, /dev/ttyACM0