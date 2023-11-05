# ESP-NOW-Benchmark
Programa y firmware para comparar el protocolo de comunicación ESP-NOW para diferentes microcontroladores de la familia ESP32.
El fimware incluye librerías para ESP32-WROOM-32 y ESP32-S3-WROOM-1 por separado.
Mediante comunicación seríal, la ejecución de python recibirá el tiempo de respuesta en el envío de un paquete mediante el protocolo ESP-NOW entre 2 microcontroladores del mismo tipo (unicast). Ese tiempo de respuesta tipo Round-trip Time, luego será exportado a un archivo csv para su posterior anáisis.
