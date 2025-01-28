#!/bin/bash

# Verifica si el script se está ejecutando como root
if [ "$EUID" -ne 0 ]; then
    echo "Este script debe ejecutarse como root. Usa 'sudo $0'."
    exit 1
fi

# Verifica si se proporcionó un argumento
if [ -z "$1" ]; then
    echo -e "Usage: $0 <interface>\nLas interfaces disponibles son:"
    ip -o link show | awk '{print $2}' | sed 's\:\\g'
    exit 1
fi

interface="$1"

ip link set "$interface" down

iw dev "$interface" set type monitor

ip link set "$interface" up

echo "La interfaz $interface se ha cambiado a modo monitor."
