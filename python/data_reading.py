import serial
import csv
import time
import sys

# Ajusta el puerto y baudrate a tu sistema
PORT = "/dev/tty.usbserial-11340"  # o "COM3" en Windows
BAUD = 115200
OUT_CSV = "datos_sensor.csv"

def main():
    try:
        ser = serial.Serial(PORT, BAUD, timeout=1)
    except Exception as e:
        print("Error abriendo puerto serie:", e)
        sys.exit(1)

    print(f"Abierto {PORT} a {BAUD} baudios. Presiona Ctrl+C para terminar.")
    with open(OUT_CSV, mode='w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(["timestamp_us", "magX_uT", "magY_uT", "magZ_uT"])

        try:
            while True:
                line = ser.readline().decode('ascii', errors='ignore').strip()
                if not line:
                    continue
                # Intentamos parsear CSV: timestamp,x,y,z
                parts = [p.strip() for p in line.split(',')]
                if len(parts) >= 4:
                    try:
                        ts = int(parts[0])
                        x = float(parts[1])
                        y = float(parts[2])
                        z = float(parts[3])
                        writer.writerow([ts, x, y, z])
                        f.flush()  # asegúrate que se escriba en disco
                        print(f"{ts}\t{x:.3f}\t{y:.3f}\t{z:.3f}")
                    except ValueError:
                        # línea no numérica -> ignorar
                        print("Línea inválida (parse):", line)
                else:
                    # Si la línea no tiene 4 campos, la mostramos como info/debug
                    print("Línea inesperada:", line)
        except KeyboardInterrupt:
            print("\nDetenido por usuario.")
        finally:
            ser.close()

if __name__ == "__main__":
    main()
