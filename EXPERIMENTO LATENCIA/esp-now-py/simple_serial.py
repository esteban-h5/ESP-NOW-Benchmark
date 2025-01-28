import serial, sys, os
from time import sleep
import pandas as pd
import numpy as np
from datetime import datetime
import paho.mqtt.publish as publish

try:
  if len(sys.argv) != 1:
    dev = sys.argv[1]
  else:
    dev = "/dev/ttyUSB0"
    #dev = "/dev/ttyUSB1"
    #dev = "/dev/ttyACM0"
    
  ser = serial.Serial(dev, 115200)
  buffer = [0]
  total = 500
  packet_loss = 1
  for _ in range(total):    
      
      line = ser.readline().decode()
      promedio = round(np.mean(buffer),3)
      data = None
      
      if "RTT:" in line:
          data = int(line.split(":")[1])
          buffer.append(data)
      else:
          packet_loss += 1
          sleep(.1)
          continue
          #data = int(promedio)
          #buffer.append(data)

      if data != None:
          loss_per = packet_loss*100/total 
          mensaje = f"{'{:3d}'.format(_)}/{total}) rtt: {'{:3d}'.format(data)}[us]\tmean: {'{:06.3f}'.format(promedio)}[us]\tloss {'{:3d}'.format(int(loss_per))}%"
          print(mensaje)
      #publish.single("prueba/esp/god", mensaje, hostname="broker.hivemq.com", port=1883)
      sleep(.1)

  #for data in buffer:
  #  add_rtt(data,time=False)
  #else:
  #  print("Datos Guardados")

  ser.close()
  print("Ciclo completado")
except KeyboardInterrupt:
   ser.close()
   exit(1)
