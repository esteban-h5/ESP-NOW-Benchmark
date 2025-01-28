import os, json, random, cv2
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import matplotlib.image as img

from time import sleep
from PIL import Image
from datetime import datetime

pwd = os.path.dirname(os.path.realpath(__file__))

eventos_dir = pwd+"\\eventos"
archivos = [_ for _ in os.listdir(eventos_dir) if ".json" in _]

if len(archivos) == 0:
   print("no existen archivos en directorio eventos.")
   exit(1)

if len(archivos) == 1:
  print(f"leyendo {archivos[0]}")
  json_dir = f"{eventos_dir}\\{archivos[0]}"

else:
  for idx, js in enumerate(archivos) :
    print(f"{idx+1}. {js}")

  x = int(input("numero de archivo a leer: ")) -1
  json_dir = f"{eventos_dir}\\{archivos[x]}"

def grid_pos(pos):
  x,y = pos
  cell_size = 1  # Asumiendo que cada celda tiene tamaño 1x1

  xmin = x * cell_size
  xmax = (x + 1) * cell_size
  ymin = y * cell_size
  ymax = (y + 1) * cell_size
  return [xmin, xmax, ymin, ymax] 
  

def ensamblar_imagen(indice_excena, t_stamp, agents_pos, puntaje_a, puntaje_b, estado_partido=None, box_list=None):
  
  fig, ax = plt.subplots(figsize=(10, 8))
  rows, cols = 8, 10

  pos = ax.get_position()
  pos.x0 = 0.2      
  pos.y0 = 0.2
  pos.x1 = 0.85     
  pos.y1 = 0.9      

  ax.set_position(pos)

  ax.set_xlim(0, cols)
  ax.set_ylim(0, rows)

  ax.invert_yaxis()
  ax.axis('off')

  #LINEAS
  for i in range(6 + 1):
      ax.plot([0, 3], [i, i], color='black', linestyle='-', linewidth=1)

  for i in range(3 + 1):
      ax.plot([i, i], [0, 6], color='black', linestyle='-', linewidth=1)

  for i in range(6 + 1):
      ax.plot([6, 9], [i, i], color='black', linestyle='-', linewidth=1)

  for i in range(6, 9 + 1):
      ax.plot([i, i], [0, 6], color='black', linestyle='-', linewidth=1)

  #CUADRADO
  ax.plot([4, 4], [1, 2], color='black', linestyle='-', linewidth=1)
  ax.plot([5, 5], [1, 2], color='black', linestyle='-', linewidth=1)
  ax.plot([4, 5], [1, 1], color='black', linestyle='-', linewidth=1)
  ax.plot([4, 5], [2, 2], color='black', linestyle='-', linewidth=1)

  #TEXTO
  sep = -0.06
  plt.text(0, 0,        'n_escena :',    horizontalalignment='right', verticalalignment='center', transform=ax.transAxes, fontsize="large")
  plt.text(0, 0+sep*1,  't :',                horizontalalignment='right', verticalalignment='center', transform=ax.transAxes, fontsize="large")
  plt.text(0, 0+sep*2,  'puntaje_A :',        horizontalalignment='right', verticalalignment='center', transform=ax.transAxes, fontsize="large")
  plt.text(0, 0+sep*3,  'puntaje_B :',        horizontalalignment='right', verticalalignment='center', transform=ax.transAxes, fontsize="large")
  plt.text(0, 0+sep*4,  'estado :',           horizontalalignment='right', verticalalignment='center', transform=ax.transAxes, fontsize="large")

  plt.text(0.005, 0,        str(indice_excena),   horizontalalignment='left', verticalalignment='center', transform=ax.transAxes, fontsize="large")
  plt.text(0.005, 0+sep*1,  str(t_stamp),         horizontalalignment='left', verticalalignment='center', transform=ax.transAxes, fontsize="large")
  plt.text(0.005, 0+sep*2,  str(puntaje_a),       horizontalalignment='left', verticalalignment='center', transform=ax.transAxes, fontsize="large")
  plt.text(0.005, 0+sep*3,  str(puntaje_b),       horizontalalignment='left', verticalalignment='center', transform=ax.transAxes, fontsize="large")
  plt.text(0.005, 0+sep*4,  str(estado_partido),  horizontalalignment='left', verticalalignment='center', transform=ax.transAxes, fontsize="large")
  
  #[pos, color] center_pos:4,1 a_min:0,0 a_max:2,5 b_min:6,0 b_max:8,5    [ [(1,2),"#color"], [(2,2),"#color"] ]
  if box_list != None:      
    # print(f"ESTADO:{estado_partido} CAJA: {box_list}")
    for box in box_list:
      pos,color = box
      pos = [pos[1], pos[0]]
      ax.add_patch(patches.Rectangle(pos, 1, 1, linewidth=1, edgecolor='b', facecolor=color))
  
  #MASTER
  ax.imshow(player, extent=grid_pos((4,1)), aspect='equal', zorder=1)

  for position in agents_pos:
    position = [position[1],position[0]]
    ax.imshow(player, extent=grid_pos(position), aspect='equal', zorder=1)

  cadena = "abcde1234567890"
  name = ''.join(random.choices(cadena, k=8))

  png_path = f"{pwd}\\tmp\\t{t_stamp}-{name}.png"
  plt.savefig(png_path)
  plt.close()

  return png_path

player = img.imread(pwd+'\\assets\\esp32.png')

with open(json_dir, 'r') as file: diccionario = json.load(file)

imagenes_directorios = []
img_path = []

total = len(diccionario["secuencia"])
more_info = bool(diccionario["more_info"])

for idx, escena in enumerate(diccionario["secuencia"]):
  
  #RENDERIZAR ESCENAS
  print(f"{idx+1} / {total}")

  if more_info:
    __path = ensamblar_imagen(idx+1,
                              escena['time'], 
                              escena['agents_pos'], 
                              escena["pts_A"], 
                              escena["pts_B"], 
                              escena["event"], 
                              escena['box_list']
                              )
  else:
    __path = ensamblar_imagen(idx+1,
                              escena['time'], 
                              escena['agents_pos'], 
                              escena["pts_A"], 
                              escena["pts_B"]
                              )
  img_path.append(__path)
  # imagenes_directorios.append( Image.open(__path) )

print("Creando animacion")

video_name = datetime.now().strftime(f"{pwd}\\output %Y-%m-%d_%H-%M-%S.mp4")
  
frame = cv2.imread(img_path[0]) 
height, width, layers = frame.shape   

video = cv2.VideoWriter(video_name, 0, 1, (width, height))  

for image in img_path:  
    video.write(cv2.imread(image))  
  
cv2.destroyAllWindows()  
video.release()

# Guarda las imágenes como un archivo GIF
# imagenes_directorios[0].save(
#     datetime.now().strftime(f"{pwd}\\output %Y-%m-%d_%H-%M-%S.gif"),
#     save_all=True,
#     append_images=imagenes_directorios[1:],
#     optimize=False,
#     duration=750,
#     loop=0
# )

sleep(.5)
print("Borrando imagenes en tmp")

for __path in img_path:
  try:
    os.remove(__path)
  except PermissionError:
    print(f"imagen {os.path.basename(__path)} se encuentra abierta")
    pass

print("Imagenes borradas")