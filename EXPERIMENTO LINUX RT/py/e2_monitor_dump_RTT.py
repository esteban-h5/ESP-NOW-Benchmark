#!/sbin/python

import time, sys
from scapy.all import sniff, wrpcap
import pandas as pd, numpy as np

if len(sys.argv) == 1:
    print(f"Uso: python e2_monitor_dump.py <interfaz> <cantidad de paquetes>")
    interfaz = "wlo1"
    max_count = 2000
else:
    try:   
        interfaz = sys.argv[1]
        max_count = int(sys.argv[2])
    except IndexError:
        interfaz = "wlo1"
        max_count = 3000
        print("Error de argumentos\nUso: python e2_monitor_dump.py <interfaz> <cantidad de paquetes>")
        pass

print(f"Usando interfaz de red \"{interfaz}\" para capturar {max_count} paquetes")

##############################################
#esp = "esp32"
esp = "esp32_s3"
##############################################
#nombre_archivo = f"e2_latencias_{esp}_windows"
#nombre_archivo = f"e2_latencias_{esp}_linux"
nombre_archivo = f"e2_latencias_{esp}_linuxRT"
##############################################

if "s3" in esp:
    mac = "7c:df:a1:ff:2b:30" #E1 s3
else:
    mac = "b8:d6:1a:ab:4d:8c" #E1 esp32

n_actual = 1
data = []

def packet_handler(pkt):
    global data
    global n_actual
    
    print(f"[{n_actual}] Paquete capturado de {pkt.addr2}")
    latency = (time.time() - pkt.time) * 1000

    print(f"Latencia: {latency:.2f} ms")
    print(f"mean: {np.mean(data):.2f} ms")
    data.append(latency)
    n_actual += 1
                    
paquetes = sniff(filter=f"ether src {mac}", iface=interfaz, prn=packet_handler, count=max_count)
wrpcap(f"{nombre_archivo}.pcap", paquetes)

df = pd.DataFrame(data, columns=["latencia [ms]"])
df.to_csv(f"{nombre_archivo}.csv", index=False)
