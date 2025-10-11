import serial
import struct
import numpy as np
import matplotlib.pyplot as plt
from collections import deque
from matplotlib.animation import FuncAnimation

# ======== CONFIGURACIÓN ========
PORT = '/dev/tty.usbserial-11340'           # Cambia por tu puerto
BAUDRATE = 921600
FS = 408                # Frecuencia de muestreo (Hz)
PACKET_SIZE = 12        # 3 floats x 4 bytes
BUFFER_SIZE = 2000      # muestras guardadas en el buffer
UPDATE_INTERVAL = 200   # ms entre actualizaciones de la gráfica
CHANNEL = 0             # 0=x, 1=y, 2=z

# ======== CONEXIÓN SERIAL ========
ser = serial.Serial(PORT, BAUDRATE, timeout=0.1)

# ======== BUFFER CIRCULAR ========
buffer = deque(maxlen=BUFFER_SIZE)

# ======== FUNCIÓN DE LECTURA SERIAL ========
def read_serial():
    while ser.in_waiting >= PACKET_SIZE:
        raw = ser.read(PACKET_SIZE)
        if len(raw) == PACKET_SIZE:
            x, y, z = struct.unpack('fff', raw)
            buffer.append((x, y, z))

# ======== CONFIGURAR GRÁFICO ========
plt.ion()
fig, ax = plt.subplots()
line, = ax.plot([], [], lw=1)
ax.set_ylim(-1, 1)
ax.set_xlim(0, BUFFER_SIZE)
ax.set_title("Campo magnético - Eje X")
ax.set_xlabel("Muestra")
ax.set_ylabel("Intensidad")

# ======== FUNCIÓN DE ACTUALIZACIÓN ========
def update(frame):
    read_serial()  # leer los datos nuevos del puerto
    if len(buffer) > 0:
        data = np.array(buffer)
        ydata = data[:, CHANNEL]
        line.set_data(np.arange(len(ydata)), ydata)
        ax.set_ylim(ydata.min() - 0.1, ydata.max() + 0.1)
    return line,

ani = FuncAnimation(fig, update, interval=UPDATE_INTERVAL, blit=True)
plt.show()

try:
    while True:
        plt.pause(0.1)
except KeyboardInterrupt:
    print("Cerrando...")
    ser.close()
