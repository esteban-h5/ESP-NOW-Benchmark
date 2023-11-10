import serial, sys
from time import sleep
import pandas as pd
from datetime import datetime
import os

if len(sys.argv) != 1:
  dev = sys.argv[1]
else:
  dev = "/dev/ttyUSB0"
  #dev = "/dev/ttyUSB1"
  #dev = "/dev/ttyACM0"
  
working_dir = os.path.dirname(os.path.realpath(__file__))

# nombre_csv = f"{working_dir}/{datetime.now().strftime('rtt_resultado_%m-%d-%Y_%H-%M-%S.csv')}"
nombre_csv = f"{working_dir}/{'rtt_data.csv'}"

ser = serial.Serial(dev, 115200)

if not os.path.isfile(nombre_csv):
    print("CSV faltante, creando...")
    with open(nombre_csv, 'w') as archivo_csv:
        archivo_csv.write('RTT,Hora\n')

def add_rtt(rtt,time=True):
    with open(nombre_csv, 'a') as archivo_csv:
        if time: archivo_csv.write(f'{rtt},{datetime.now().strftime("%H:%M:%S")},\n')
        else: archivo_csv.write(f'{rtt},\n')

# buffer = []
for _ in range(500):    
    
    line = ser.readline().decode()
    print(f"{_}) ",end="") 
    if "Fail" in line:
        print("---")

    elif "RTT:" in line:
        rtt = int(line.split(":")[1])
        print(f"RTT:{rtt}[us]")
        add_rtt(rtt)
        #buffer.append(rtt)
    
    else:
        print(f"Lectura incorrecta: {line}")
        
    sleep(.2)

#for data in buffer:
#  add_rtt(data,time=False)
#else:
#  print("Datos Guardados")

ser.close()
print("Ciclo completado")
