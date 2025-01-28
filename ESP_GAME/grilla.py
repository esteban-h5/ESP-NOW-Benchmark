"""
0   0   0   0   0
0   1   1   1   0
0   1   1   1   0
0   1   1   1   0
0   1   1   1   0
0   1   1   1   0
0   1   1   1   0
0   0   0   0   0
"""
# [0,0,0,0,0]
# [0,1,1,1,0]

def espacios_cubiertos(posicion, lookup_table):

    # lookup table debe tener un contorno rodeado con 0, traslacion conciderada
    x_pos,y_pos = posicion
    
    try:
        if lookup_table[x_pos+1][y_pos+1] == 0: raise IndexError(f"Fuera de los margenes en tabla de busqueda.")
    except IndexError as e:
        print(e)
        return -1

    # Movimiento con kernel cuadrada (Rey de ajedrez)
    kernel = lookup_table[ x_pos:x_pos+3, y_pos:y_pos+3 ]

    print(kernel)
    return np.sum(kernel) 

import numpy as np
lookup_table = np.array([[0,0,0,0,0] if (_ == 0 or _ == 7) else [0,1,1,1,0] for _ in range(8) ])

print(lookup_table,end="\n\n")

x = espacios_cubiertos([0,1], lookup_table)
print(x)