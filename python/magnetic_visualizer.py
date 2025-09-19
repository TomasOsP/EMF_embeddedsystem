import serial
import time
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.quiver as quiver
import re
from collections import deque

class MLX90393Visualizer:
    def __init__(self, port='/dev/tty.usbserial-10', baudrate=115200, max_points=500):
        self.ser = serial.Serial(port, baudrate, timeout=1)
        self.max_points = max_points
        
        # Datos para la gráfica temporal
        self.times = deque(maxlen=max_points)
        self.x_values = deque(maxlen=max_points)
        self.y_values = deque(maxlen=max_points)
        self.z_values = deque(maxlen=max_points)
        self.magnitudes = deque(maxlen=max_points)
        
        # Datos para la gráfica 3D (últimos puntos)
        self.positions_3d = deque(maxlen=100)
        self.vectors_3d = deque(maxlen=100)
        
        # Configurar las gráficas
        self.setup_plots()
        
        # Tiempo inicial
        self.start_time = time.time()
        
    def setup_plots(self):
        # Configurar la figura con dos subplots
        self.fig = plt.figure(figsize=(15, 10))
        
        # Gráfica temporal (2D)
        self.ax1 = self.fig.add_subplot(2, 1, 1)
        self.ax1.set_title('Flujo Magnético en función del Tiempo')
        self.ax1.set_xlabel('Tiempo (s)')
        self.ax1.set_ylabel('Flujo Magnético (µT)')
        self.ax1.grid(True)
        
        # Líneas para cada componente
        self.line_x, = self.ax1.plot([], [], 'r-', label='X', alpha=0.8)
        self.line_y, = self.ax1.plot([], [], 'g-', label='Y', alpha=0.8)
        self.line_z, = self.ax1.plot([], [], 'b-', label='Z', alpha=0.8)
        self.line_mag, = self.ax1.plot([], [], 'k-', label='Magnitud', linewidth=2)
        self.ax1.legend()
        
        # Límites iniciales
        self.ax1.set_xlim(0, 60)
        self.ax1.set_ylim(-200, 200)
        
        # Gráfica 3D
        self.ax2 = self.fig.add_subplot(2, 1, 2, projection='3d')
        self.ax2.set_title('Dirección del Campo Magnético 3D')
        self.ax2.set_xlabel('X (µT)')
        self.ax2.set_ylabel('Y (µT)')
        self.ax2.set_zlabel('Z (µT)')
        
        # Configurar límites del espacio 3D
        self.ax2.set_xlim(-200, 200)
        self.ax2.set_ylim(-200, 200)
        self.ax2.set_zlim(-200, 200)
        
        # Quiver plot para vectores
        self.quiver_plot = None
        
        # Ajustar layout
        plt.tight_layout()
    
    def parse_data(self, line):
        """Parsear los datos del serial"""
        try:
            # Buscar patrones en la línea
            x_match = re.search(r'X: ([-+]?\d*\.\d+|\d+) µT', line)
            y_match = re.search(r'Y: ([-+]?\d*\.\d+|\d+) µT', line)
            z_match = re.search(r'Z: ([-+]?\d*\.\d+|\d+) µT', line)
            mag_match = re.search(r'Magnitud: ([-+]?\d*\.\d+|\d+) µT', line)
            
            if x_match and y_match and z_match:
                x = float(x_match.group(1))
                y = float(y_match.group(1))
                z = float(z_match.group(1))
                
                # Calcular magnitud si no está en el log
                if mag_match:
                    magnitude = float(mag_match.group(1))
                else:
                    magnitude = np.sqrt(x**2 + y**2 + z**2)
                
                return x, y, z, magnitude
        except (ValueError, AttributeError) as e:
            print(f"Error parsing data: {e}")
        
        return None, None, None, None
    
    def read_serial(self):
        """Leer datos del puerto serial"""
        try:
            if self.ser.in_waiting > 0:
                line = self.ser.readline().decode('utf-8', errors='ignore').strip()
                if line and ('X:' in line and 'Y:' in line and 'Z:' in line):
                    return self.parse_data(line)
        except serial.SerialException as e:
            print(f"Serial error: {e}")
            return None, None, None, None
        
        return None, None, None, None
    
    def update_plots(self, frame):
        """Actualizar las gráficas"""
        # Leer datos del serial
        x, y, z, magnitude = self.read_serial()
        
        if x is not None:
            current_time = time.time() - self.start_time
            
            # Actualizar datos temporales
            self.times.append(current_time)
            self.x_values.append(x)
            self.y_values.append(y)
            self.z_values.append(z)
            self.magnitudes.append(magnitude)
            
            # Actualizar datos 3D (solo cada cierto tiempo para no saturar)
            if len(self.times) % 5 == 0:
                self.positions_3d.append([0, 0, 0])  # Origen
                self.vectors_3d.append([x, y, z])
            
            # Actualizar gráfica temporal
            if len(self.times) > 0:
                self.line_x.set_data(self.times, self.x_values)
                self.line_y.set_data(self.times, self.y_values)
                self.line_z.set_data(self.times, self.z_values)
                self.line_mag.set_data(self.times, self.magnitudes)
                
                # Ajustar límites del tiempo
                if current_time > self.ax1.get_xlim()[1]:
                    self.ax1.set_xlim(0, current_time + 10)
                
                # Ajustar límites Y si es necesario
                current_max = max(max(self.x_values), max(self.y_values), 
                                 max(self.z_values), max(self.magnitudes))
                current_min = min(min(self.x_values), min(self.y_values), 
                                 min(self.z_values), min(self.magnitudes))
                
                if current_max > self.ax1.get_ylim()[1] or current_min < self.ax1.get_ylim()[0]:
                    margin = 20
                    self.ax1.set_ylim(current_min - margin, current_max + margin)
            
            # Actualizar gráfica 3D
            if len(self.positions_3d) > 0:
                # Limpiar gráfica anterior
                if self.quiver_plot is not None:
                    self.quiver_plot.remove()
                
                # Convertir a arrays numpy
                positions = np.array(self.positions_3d)
                vectors = np.array(self.vectors_3d)
                
                # Crear quiver plot
                self.quiver_plot = self.ax2.quiver(
                    positions[:, 0], positions[:, 1], positions[:, 2],
                    vectors[:, 0], vectors[:, 1], vectors[:, 2],
                    length=100, normalize=True, color='red', alpha=0.6,
                    arrow_length_ratio=0.3
                )
        
        return self.line_x, self.line_y, self.line_z, self.line_mag
    
    def run(self):
        """Ejecutar la visualización"""
        # Esperar a que lleguen datos
        print("Esperando datos del sensor...")
        time.sleep(2)
        
        # Crear animación
        ani = FuncAnimation(self.fig, self.update_plots, interval=100, blit=True)
        plt.show()
    
    def close(self):
        """Cerrar conexión serial"""
        if self.ser.is_open:
            self.ser.close()

# Función principal
def main():
    # Configuración - ajusta el puerto COM según tu sistema
    # Windows: 'COM3', 'COM4', etc.
    # Linux/Mac: '/dev/ttyUSB0', '/dev/ttyACM0', etc.
    port = 'COM3'  # Cambia esto por tu puerto
    
    try:
        visualizer = MLX90393Visualizer(port=port)
        visualizer.run()
    except serial.SerialException as e:
        print(f"No se pudo abrir el puerto {port}: {e}")
        print("Por favor, verifica:")
        print("1. Que el puerto COM sea correcto")
        print("2. Que el Arduino esté conectado")
        print("3. Que no haya otro programa usando el puerto")
    except KeyboardInterrupt:
        print("\nCerrando visualización...")
    finally:
        if 'visualizer' in locals():
            visualizer.close()

if __name__ == "__main__":
    main()