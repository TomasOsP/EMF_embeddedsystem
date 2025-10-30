import serial
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from matplotlib.animation import FuncAnimation

# === CONFIGURACIÓN SERIAL ===
PORT = '/dev/tty.usbserial-1340'  # ⚠️ cambia por tu puerto (ej. COM3 o /dev/ttyUSB0)
BAUD = 115200
TIMEOUT = 1.0

# === CONFIGURACIÓN PLOT ===
UPDATE_INTERVAL = 50  # ms
VECTOR_SCALE = 0.01   # factor de escala visual

# === CONEXIÓN SERIAL ===
ser = serial.Serial(PORT, BAUD, timeout=TIMEOUT)
print(f"Conectado a {PORT} a {BAUD} baudios")

# === CONFIGURAR GRÁFICO 3D ===
plt.style.use('dark_background')
fig = plt.figure(figsize=(7, 7))
ax = fig.add_subplot(111, projection='3d')
ax.set_xlim([-1, 1])
ax.set_ylim([-1, 1])
ax.set_zlim([-1, 1])
ax.set_title("Vector del Campo Magnético - Dirección e Intensidad", fontsize=13)
ax.set_xlabel("X (µT)")
ax.set_ylabel("Y (µT)")
ax.set_zlabel("Z (µT)")
ax.view_init(elev=20, azim=40)
ax.grid(True)

# === INICIALIZAR VECTOR ===
origin = np.array([[0, 0, 0]])
vector = np.array([[0, 0, 0]])
quiver = ax.quiver(0, 0, 0, 0, 0, 0, color='cyan', linewidth=2)

# === FUNCIÓN DE LECTURA SERIAL ===
def read_serial_line():
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
            return np.array([x, y, z])
    except:
        return None
    return None

# === ACTUALIZAR VECTOR ===
def update(frame):
    global quiver
    B = read_serial_line()
    if B is not None:
        # Normalizar y escalar
        magnitude = np.linalg.norm(B)
        if magnitude == 0:
            return quiver,
        direction = B / magnitude
        scaled = direction * (magnitude * VECTOR_SCALE)

        # Actualizar vector
        quiver.remove()
        quiver = ax.quiver(
            0, 0, 0,
            scaled[0], scaled[1], scaled[2],
            color='cyan', linewidth=2, arrow_length_ratio=0.15
        )

        # Mostrar info en consola
        print(f"|B|={magnitude:.2f} µT  → dir=({direction[0]:.2f}, {direction[1]:.2f}, {direction[2]:.2f})")

    return quiver,

# === ANIMACIÓN ===
ani = FuncAnimation(fig, update, interval=UPDATE_INTERVAL, blit=False)

try:
    plt.show()
except KeyboardInterrupt:
    print("\nInterrumpido por usuario.")
finally:
    ser.close()
    print("Puerto cerrado.")
