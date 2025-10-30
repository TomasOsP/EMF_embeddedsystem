import serial
import numpy as np
import matplotlib.pyplot as plt
from collections import deque
from matplotlib.animation import FuncAnimation

# === CONFIGURACIÓN SERIAL ===
PORT = '/dev/tty.usbserial-11340'   # ⚠️ cambia según tu puerto
BAUD = 115200
TIMEOUT = 1.0

# === CONFIGURACIÓN FFT ===
BUFFER_SIZE = 1024       # número de muestras usadas para FFT
UPDATE_INTERVAL = 200    # ms entre actualizaciones
AXIS = 3                 # 0=X, 1=Y, 2=Z, 3=|B|
SAMPLE_RATE_EST = 400.0  # valor inicial estimado (Hz)
SMOOTHING_ALPHA = 0.4    # factor de suavizado exponencial [0-1]

# === CONEXIÓN SERIAL ===
ser = serial.Serial(PORT, BAUD, timeout=TIMEOUT)
print(f"Conectado a {PORT} a {BAUD} baudios")

# === BUFFER PARA MUESTRAS ===
timestamps = deque(maxlen=BUFFER_SIZE)
x_data = deque(maxlen=BUFFER_SIZE)
y_data = deque(maxlen=BUFFER_SIZE)
z_data = deque(maxlen=BUFFER_SIZE)
b_data = deque(maxlen=BUFFER_SIZE)

# === CONFIGURACIÓN DE PLOT ===
plt.style.use('dark_background')
fig, ax = plt.subplots(figsize=(9, 5))
line_fft, = ax.plot([], [], color='cyan')
peak_dot, = ax.plot([], [], 'ro', markersize=6)
text_info = ax.text(0.02, 0.95, '', transform=ax.transAxes, color='white', fontsize=10)
text_peak = ax.text(0.7, 0.85, '', transform=ax.transAxes, color='orange', fontsize=11, fontweight='bold')
ax.set_title("Espectro FFT en tiempo real")
ax.set_xlabel("Frecuencia (Hz)")
ax.set_ylabel("Amplitud (µT)")
ax.grid(True)

# === VARIABLES GLOBALES ===
last_ts = None
freq_estimate = SAMPLE_RATE_EST
smoothed_spectrum = None

# === LECTURA SERIAL ===
def read_serial_line():
    global last_ts, freq_estimate
    try:
        line = ser.readline().decode('ascii', errors='ignore').strip()
        if not line:
            return
        parts = line.split(',')
        if len(parts) >= 4:
            ts = int(parts[0])
            x = float(parts[1])
            y = float(parts[2])
            z = float(parts[3])
            b = np.sqrt(x**2 + y**2 + z**2)

            # estimar frecuencia de muestreo
            if last_ts is not None:
                dt = (ts - last_ts) / 1e6
                if 0 < dt < 1:
                    freq_estimate = 1.0 / dt
            last_ts = ts

            timestamps.append(ts)
            x_data.append(x)
            y_data.append(y)
            z_data.append(z)
            b_data.append(b)
    except:
        pass

# === ACTUALIZACIÓN DE ANIMACIÓN ===
def update(frame):
    global smoothed_spectrum

    # leer varias muestras antes de actualizar
    for _ in range(20):
        read_serial_line()

    # seleccionar eje
    if AXIS == 0:
        signal = np.array(x_data)
        label = 'X'
    elif AXIS == 1:
        signal = np.array(y_data)
        label = 'Y'
    elif AXIS == 2:
        signal = np.array(z_data)
        label = 'Z'
    else:
        signal = np.array(b_data)
        label = '|B|'

    if len(signal) > 10:
        N = len(signal)
        window = np.hanning(N)
        fft_vals = np.fft.rfft(signal * window)
        fft_freqs = np.fft.rfftfreq(N, 1 / freq_estimate)
        magnitude = np.abs(fft_vals) / N

        # === SUAVIZADO EXPONENCIAL ===
        if smoothed_spectrum is None or len(smoothed_spectrum) != len(magnitude):
            smoothed_spectrum = magnitude
        else:
            smoothed_spectrum = SMOOTHING_ALPHA * magnitude + (1 - SMOOTHING_ALPHA) * smoothed_spectrum

        # === DETECCIÓN DE PICO PRINCIPAL ===
        peak_idx = np.argmax(smoothed_spectrum)
        peak_freq = fft_freqs[peak_idx]
        peak_amp = smoothed_spectrum[peak_idx]

        # === ACTUALIZAR GRÁFICA ===
        line_fft.set_data(fft_freqs, smoothed_spectrum)
        peak_dot.set_data([peak_freq], [peak_amp])

        ax.set_xlim(0, freq_estimate / 2)
        ax.set_ylim(0, np.max(smoothed_spectrum) * 1.2 if np.max(smoothed_spectrum) > 0 else 1)

        text_info.set_text(f"Eje: {label} | Fs ≈ {freq_estimate:.1f} Hz | N={N}")
        text_peak.set_text(f"Pico: {peak_freq:.1f} Hz ({peak_amp:.3f} µT)")

    return line_fft, peak_dot, text_info, text_peak

# === INICIAR ANIMACIÓN ===
ani = FuncAnimation(fig, update, interval=UPDATE_INTERVAL, blit=False)

try:
    plt.show()
except KeyboardInterrupt:
    print("\nInterrumpido por el usuario.")
finally:
    ser.close()
    print("Puerto cerrado.")
