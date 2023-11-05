import serial, sys
from time import sleep
#Imprimir serial por stdout
ser = serial.Serial('/dev/ttyUSB0', 115200)
i = 0
while True:    
    print(f"{i}: {ser.readline().decode()}")
    sleep(.2)
    i+=1
ser.close()