#!/sbin/python

import serial, sys, os
from time import sleep
import pandas as pd, numpy as np

if len(sys.argv) != 1:
  dev = sys.argv[1]
  
else:
  print("Uso: python e1_serial_RTT.py <dispositivo> \nUsando dispositivo \"/dev/ttyUSB0\"")
  
  dev = "/dev/ttyUSB0"
  #dev = "/dev/ttyUSB1"
  #dev = "/dev/ttyACM0"

##############################################
serial_esp = "esp32"
#serial_esp = "esp32_s3"
##############################################
#slave_esp = "esp32"
slave_esp = "esp32_s3"
##############################################

nombre_archivo = f"e1_latencias_{serial_esp}_{slave_esp}"
max_count = 2000
n_actual = 1
data = []

try:
  ser = serial.Serial(dev, 115200)
  
  while n_actual <= max_count:
      
      line = ser.readline().decode()
      promedio = round(np.mean(data),3)
      rtt = None
      
      if "RTT:" in line:
          rtt = int(line.split(":")[1])
          data.append(rtt)
          n_actual += 1

      else:
          sleep(.1)
          continue

      if rtt != None:
          print(f"[{n_actual}] Paquete capturado")
          print(f"RTT: {rtt:.2f} ms")

      sleep(.1)

  ser.close()
  print("Ciclo completado")
  
  df = pd.DataFrame(data, columns=["latencia [ms]"])
  df.to_csv(f"{nombre_archivo}.csv", index=False)

except KeyboardInterrupt:
   ser.close()
   exit(1)
