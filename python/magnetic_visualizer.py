import serial
import time
import matplotlib.pyplot as plt
from collections import deque
from matplotlib.animation import FuncAnimation
import numpy as np

# === CONFIGURACIÓN SERIAL ===
PORT = '/dev/tty.usbserial-11340'  # ⚠️ cambia según tu puerto
BAUD = 115200
TIMEOUT = 1.0

# === PARÁMETROS DE GRAFICACIÓN ===
BUFFER_SIZE = 500      # cantidad de muestras visibles
UPDATE_INTERVAL = 50   # ms entre actualizaciones

# === CONEXIÓN SERIAL ===
ser = serial.Serial(PORT, BAUD, timeout=TIMEOUT)
print(f"Conectado a {PORT} a {BAUD} baudios")

# === BUFFERS CIRCULARES ===
timestamps = deque(maxlen=BUFFER_SIZE)
x_data = deque(maxlen=BUFFER_SIZE)
y_data = deque(maxlen=BUFFER_SIZE)
z_data = deque(maxlen=BUFFER_SIZE)
b_data = deque(maxlen=BUFFER_SIZE)  # módulo del campo

# === VARIABLES DE FRECUENCIA DE MUESTREO ===
last_timestamp = None
intervals = deque(maxlen=200)

# === CONFIGURACIÓN DEL PLOT ===
plt.style.use('dark_background')
fig, ax = plt.subplots(figsize=(9, 5))

line_x, = ax.plot([], [], 'r-', label='X (µT)')
line_y, = ax.plot([], [], 'g-', label='Y (µT)')
line_z, = ax.plot([], [], 'b-', label='Z (µT)')
line_b, = ax.plot([], [], 'w--', linewidth=2, label='|B| (µT)')

text_freq = ax.text(0.02, 0.95, '', transform=ax.transAxes, color='white', fontsize=10)

ax.set_title("Magnetic Field (µT) - Real Time")
ax.set_xlabel("Samples")
ax.set_ylabel("Magnetic Field (µT)")
ax.legend(loc="upper right")
ax.grid(True)

# === FUNCIÓN DE LECTURA SERIAL ===
def read_serial_line():
    global last_timestamp
    try:
        line = ser.readline().decode('ascii', errors='ignore').strip()
        if not line:
            return None
        parts = line.split(',')
        if len(parts) >= 4:
            ts = int(parts[0])
            x = float(parts[1])
            y = float(parts[2])
            z = float(parts[3])

            # Calcula módulo
            b = (x**2 + y**2 + z**2) ** 0.5

            # Calcula delta tiempo (para frecuencia)
            if last_timestamp is not None:
                dt = (ts - last_timestamp) / 1e6  # µs -> s
                if dt > 0:
                    intervals.append(dt)
            last_timestamp = ts

            return ts, x, y, z, b
    except Exception:
        return None
    return None

# === FUNCIÓN DE ACTUALIZACIÓN ===
def update(frame):
    data = read_serial_line()
    if data is not None:
        ts, x, y, z, b = data
        timestamps.append(ts)
        x_data.append(x)
        y_data.append(y)
        z_data.append(z)
        b_data.append(b)

        # Actualizar datos de líneas
        line_x.set_data(range(len(x_data)), x_data)
        line_y.set_data(range(len(y_data)), y_data)
        line_z.set_data(range(len(z_data)), z_data)
        line_b.set_data(range(len(b_data)), b_data)

        ax.set_xlim(0, BUFFER_SIZE)
        ymin = min(min(x_data, default=0), min(y_data, default=0), min(z_data, default=0))
        ymax = max(max(x_data, default=0), max(y_data, default=0), max(z_data, default=0))
        ax.set_ylim(ymin - 5, ymax + 5)

        # Frecuencia de muestreo promedio
        if len(intervals) > 5:
            avg_freq = 1.0 / np.mean(intervals)
            text_freq.set_text(f"Sampling rate: {avg_freq:.1f} Hz")
        else:
            text_freq.set_text("Sampling rate: -- Hz")

    return line_x, line_y, line_z, line_b, text_freq

# === INICIAR ANIMACIÓN ===
ani = FuncAnimation(fig, update, interval=UPDATE_INTERVAL, blit=False)

try:
    plt.show()
except KeyboardInterrupt:
    print("\nInterrumpido por el usuario.")
finally:
    ser.close()
    print("Puerto cerrado.")
