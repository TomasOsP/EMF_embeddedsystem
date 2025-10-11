import serial
import struct
import numpy as np

# ======== CONFIGURACIÓN ========
PORT = '/dev/tty.usbserial-11340'        # Cambia por tu puerto
BAUDRATE = 115200
FS = 408             # Frecuencia de muestreo (Hz)
N = 32               # Solo 32 muestras para prueba preliminar

# ======== CONEXIÓN SERIAL ========
try:
    ser = serial.Serial(PORT, BAUDRATE, timeout=2)
except Exception as e:
    print(f"Error abriendo el puerto serial: {e}")
    exit(1)

print("📡 Recibiendo datos de prueba...")

raw = ser.read(4 * 3 * N)  # N muestras, cada muestra 3 floats de 4 bytes

if len(raw) != 4 * 3 * N:
    print(f"Error: se recibieron {len(raw)} bytes, se esperaban {4*3*N}")
else:
    try:
        data = np.array(struct.unpack(f'{N*3}f', raw)).reshape(N, 3)
        print("Primeras 5 muestras recibidas (X, Y, Z):")
        print(data[:5])
    except struct.error as e:
        print(f"Error al decodificar datos: {e}")

ser.close()