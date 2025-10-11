import serial
import time
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from collections import deque
import re

# Configuración del puerto serial (ajusta el puerto según tu sistema, e.g., 'COM3' en Windows o '/dev/ttyUSB0' en Linux/Mac)
PORT = '/dev/tty.usbserial-10'  # Cambia esto por el puerto de tu ESP32
BAUDRATE = 115200
MAX_POINTS = 500  # Número máximo de puntos en la gráfica

# Inicializar colas para datos
times = deque(maxlen=MAX_POINTS)
x_values = deque(maxlen=MAX_POINTS)
y_values = deque(maxlen=MAX_POINTS)
z_values = deque(maxlen=MAX_POINTS)
magnitudes = deque(maxlen=MAX_POINTS)

# Configurar la figura y el eje
fig, ax = plt.subplots(figsize=(10, 6))
ax.set_title('Flujo Magnético en función del Tiempo (desde ESP32)')
ax.set_xlabel('Tiempo (s)')
ax.set_ylabel('Flujo Magnético (µT)')
ax.grid(True)

# Líneas para cada componente
line_x, = ax.plot([], [], 'r-', label='X', alpha=0.8)
line_y, = ax.plot([], [], 'g-', label='Y', alpha=0.8)
line_z, = ax.plot([], [], 'b-', label='Z', alpha=0.8)
line_mag, = ax.plot([], [], 'k-', label='Magnitud', linewidth=2)
ax.legend()

# Texto para mostrar los datos actuales (en la esquina superior izquierda)
current_text = ax.text(0.05, 0.95, 'Current:\nX: N/A\nY: N/A\nZ: N/A\nMag: N/A', 
                       transform=ax.transAxes, va='top', bbox=dict(facecolor='white', alpha=0.5))

# Límites iniciales
ax.set_xlim(0, 60)
ax.set_ylim(-200, 200)

# Tiempo inicial
start_time = time.time()

# Función para parsear los datos de la línea serial (basado en el formato de main.cpp)
def parse_data(line):
    try:
        # Buscar patrones en la línea (formato: "X: valor µT\tY: valor µT\tZ: valor µT\t" y "Magnitud: valor µT\t")
        x_match = re.search(r'X: ([-+]?\d*\.\d+|\d+) µT', line)
        y_match = re.search(r'Y: ([-+]?\d*\.\d+|\d+) µT', line)
        z_match = re.search(r'Z: ([-+]?\d*\.\d+|\d+) µT', line)
        mag_match = re.search(r'Magnitud: ([-+]?\d*\.\d+|\d+) µT', line)
        
        if x_match and y_match and z_match:
            x = float(x_match.group(1))
            y = float(y_match.group(1))
            z = float(z_match.group(1))
            magnitude = float(mag_match.group(1)) if mag_match else np.sqrt(x**2 + y**2 + z**2)
            return x, y, z, magnitude
    except Exception as e:
        print(f"Error parsing data: {e}")
    return None, None, None, None

# Abrir conexión serial
ser = serial.Serial(PORT, BAUDRATE, timeout=1)

# Función de actualización para la animación
def update(frame):
    try:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if 'X:' in line and 'Y:' in line and 'Z:' in line:
                x, y, z, magnitude = parse_data(line)
                if x is not None:
                    current_time = time.time() - start_time
                    times.append(current_time)
                    x_values.append(x)
                    y_values.append(y)
                    z_values.append(z)
                    magnitudes.append(magnitude)
                    
                    # Actualizar líneas
                    line_x.set_data(times, x_values)
                    line_y.set_data(times, y_values)
                    line_z.set_data(times, z_values)
                    line_mag.set_data(times, magnitudes)
                    
                    # Actualizar texto con datos actuales
                    current_text.set_text(f"Current:\nX: {x:.2f} µT\nY: {y:.2f} µT\nZ: {z:.2f} µT\nMag: {magnitude:.2f} µT")
                    
                    # Ajustar límites X si es necesario
                    if current_time > ax.get_xlim()[1]:
                        ax.set_xlim(0, current_time + 10)
                    
                    # Ajustar límites Y si es necesario
                    current_max = max(max(x_values), max(y_values), max(z_values), max(magnitudes))
                    current_min = min(min(x_values), min(y_values), min(z_values), min(magnitudes))
                    if current_max > ax.get_ylim()[1] or current_min < ax.get_ylim()[0]:
                        margin = 20
                        ax.set_ylim(current_min - margin, current_max + margin)
                    
                    fig.canvas.draw()
    except Exception as e:
        print(f"Serial error: {e}")
    
    return line_x, line_y, line_z, line_mag, current_text

# Crear animación
ani = FuncAnimation(fig, update, interval=100, blit=True)

# Mostrar la gráfica
plt.tight_layout()
plt.show()

# Cerrar serial al finalizar
ser.close()