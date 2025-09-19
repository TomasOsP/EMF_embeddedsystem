# Sistema de Monitoreo Magnético con MLX90393

Sistema completo para monitorear y visualizar campos magnéticos usando el sensor MLX90393 con ESP32.

## Estructura del Proyecto
mlx90393-magnetic-sensor/
├── arduino/ # Código para ESP32
├── python-visualization/ # Visualización en Python
└── docs/ # Documentación

## Hardware Requerido

- ESP32
- Sensor MLX90393
- Cables de conexión

## Instalación

### Arduino (ESP32)
1. Abrir `arduino/main.cpp` en Arduino IDE
2. Instalar librería Adafruit MLX90393
3. Subir a la ESP32

### Python Visualization
```bash
cd python-visualization
pip install -r requirements.txt
python magnetic_visualizer.py