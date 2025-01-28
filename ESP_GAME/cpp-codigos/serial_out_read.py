import serial
import serial.tools.list_ports
from time import sleep
from datetime import datetime 

def list_active_ports():
    """Listar todos los puertos seriales activos."""
    ports = serial.tools.list_ports.comports()
    active_ports = []
    print("Puertos seriales activos:")
    for i, port in enumerate(ports, start=1):
        print(f"{i}: {port.device} - {port.description}")
        active_ports.append(port.device)
        
    if not active_ports:
        input("No se encontraron puertos seriales activos. Enter para cerrar...")
        
    return active_ports

def read_serial_to_file(port, baud_rate, output_file):
    """Leer datos del puerto serial y guardarlos en un archivo."""
    try:
        with serial.Serial(port, baud_rate, timeout=1) as ser:
            print(f"Conectado al puerto {port} a {baud_rate} baudios.\nLineas con # al comienzo no son guardadas en archivo de salida")
            
            with open(output_file, "w") as file:
                
                print(f"Guardando datos en {output_file}. Presiona Ctrl+C para detener.")
                print(f"Esperando keyword de inicio (serial_begin).")
                
                while True:
                    try:
                        #imprimir e ignorar hasta keyword serial_begin
                        while True:
                            if ser.in_waiting > 0:
                                line = ser.readline().decode('utf-8').strip()
                                print(f"# {line}")
                                if "serial_begin" in line:
                                    print("Leyendo puerto serial")
                                    break
                        
                        while True:
                            if ser.in_waiting > 0:
                                line = ser.readline().decode('utf-8').strip()
                                print(f"{line}")
                                file.write(line + "\n")
                                file.flush()
                    except UnicodeDecodeError:
                        pass
                        
    except KeyboardInterrupt:
        print("\nLectura interrumpida por el usuario.")
    except serial.SerialException as e:
        print(f"Error al acceder al puerto serial: {e}")

if __name__ == "__main__":
    # Listar puertos seriales activos
    active_ports = list_active_ports()
    if not active_ports:
        exit()
    
    # Solicitar al usuario que elija un puerto
    while True:
        try:
            choice = int(input("\nSelecciona el número del puerto para usar: ")) - 1
            if 0 <= choice < len(active_ports):
                selected_port = active_ports[choice]
                break
            else:
                print("Selección inválida. Intenta nuevamente.")
        except ValueError:
            print("Por favor, ingresa un número válido.")
    
    # Configuración para lectura
    baud_rate = 115200  # Cambia esto si necesitas otra velocidad
    output_file = datetime.now().strftime("output %Y-%m-%d_%H-%M-%S.txt")
    
    # Leer datos del puerto serial seleccionado y guardarlos
    read_serial_to_file(selected_port, baud_rate, output_file)
