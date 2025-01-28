import os

def sort_listdir(directory):
    def get_creation_time(item):
        item_path = os.path.join(directory, item)
        return os.path.getctime(item_path)

    items = os.listdir(directory)
    sorted_items = sorted(items, key=get_creation_time, reverse=True)
    return sorted_items

wd = os.path.dirname(os.path.realpath(__file__))
# wd = os.path.abspath("")

eventos_dir = wd+"\\eventos\\"
graficos_dir = wd+"\\graficos\\"

#_=[print(_) for _ in sort_listdir(eventos_dir)[:6]]
evento_list = [_ for _ in os.listdir(eventos_dir)]

import sys, json 
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.ticker import FixedLocator, FixedFormatter

total=len(evento_list)
for idx,evento_name in enumerate(evento_list):
  print(f"EVENTOS RESTANTES: {idx}/{total}")
  evento_name = evento_name.replace("\n","")
  evento_path = eventos_dir+evento_name

  if not os.path.isfile(evento_path):
      print(f"Evento {evento_name} no encontrado.")
      archivos = os.listdir(eventos_dir)
      if len(archivos) != 0:
          print("Existen los siguientes archivos en directorio eventos:")
          [print("\t- "+_) for _ in archivos]

      raise Exception("archivo no encontrado")

  print(f"leyendo archivo {evento_name}")
  with open(evento_path, 'r') as file: 
      
      fstring = json.load(file)
      secuencia = fstring["secuencia"]

      fstring.pop("secuencia")
      info = fstring
      
  print(f"creando dataframe con {len(secuencia)} datos")

  print("Información de json \n")
  # n_partidos = int([_ for _ in info["config"].split(",") if "max_partidos" in _][0].split(":")[1])

  for llave,valor in info.items():
    print(f"{llave}: {valor}")
  # print(f"n_partidos: {n_partidos}")

  df = pd.DataFrame(secuencia)
  df_pts = df[["time","pts_A","pts_B"]].drop_duplicates().reset_index().rename(columns={'index': 'indice'})
  df_pts["diff"] = df_pts["pts_A"] - df_pts["pts_B"]

  m_A_list = []
  m_B_list = []
  m_diff_list = []

  paso = 100

  x_b,y_b = 0,0
  for idx,(x,y) in enumerate(df_pts[["indice","pts_A"]].values):
    if idx%paso == 0 and idx != 0:
      m = (y-y_b)/(x-x_b)
      m_A_list.append(m)

  x_b,y_b = 0,0
  for idx,(x,y) in enumerate(df_pts[["indice","pts_B"]].values):
    if idx%paso == 0 and idx != 0:
      m = (y-y_b)/(x-x_b)
      m_B_list.append(m)

  x_b,y_b = 0,0
  for idx,(x,y) in enumerate(df_pts[["indice","diff"]].values):
    if idx%paso == 0 and idx != 0:
      m = (y-y_b)/(x-x_b)
      m_diff_list.append(m)

  m_A = np.mean(m_A_list).round(3)
  m_B = np.mean(m_B_list).round(3)

  m_diff = np.mean(m_diff_list).round(3)

  print(f"A:{m_A}\nB:{m_B}\nDiff:{m_diff}")

  fig, ax1 = plt.subplots()
  pts_A,pts_B,pts_err = info["final"].split(" - ")

  print("graficando variable 1")
  sns.lineplot(data=df_pts, x="indice", y="pts_A", label=f"Puntos A", color='blue', ax=ax1)

  print("graficando variable 2")
  sns.lineplot(data=df_pts, x="indice", y="pts_B", label=f"Puntos B", color='red', ax=ax1)
  ax1.set_xlabel("Número Partido")
  ax1.set_ylabel("Puntos")

  ax1.set_title(f"{evento_name}")

  ax1.xaxis.get_major_formatter().set_scientific(False)
  ax1.yaxis.get_major_formatter().set_scientific(False)

  ax1.text(-.2, -.25,  f'Puntos {pts_A.replace(":",": "): <10} -  mx: {m_A}\nPuntos {pts_B.replace(":",": "): <10} -  mx: {m_B}', ha='left', va='bottom', fontsize=12, transform=ax1.transAxes, fontfamily='monospace')

  ax1.grid(True)
  plt.savefig(graficos_dir+evento_name.replace(".json", "_g1.jpg"), bbox_inches='tight')

  fig, ax1 = plt.subplots()
  plt.margins()

  print("graficando diferencia")
  sns.lineplot(data=df_pts, x="indice", y="diff", color='blue', ax=ax1)

  ax1.set_xlabel("Numero de Partido")
  ax1.set_ylabel("Equipo B                       Equipo A")

  max_val = np.max(np.abs(df_pts["diff"]))

  ax1.axhline(0, color='black', linewidth=1, linestyle='--')
  ax1.set_ylim(-max_val-max_val*0.2, max_val+max_val*0.2)

  yticks = plt.gca().get_yticklabels()

  yticks[:2][1].set_fontsize(15)
  yticks[-2:][0].set_fontsize(15)

  ax1.xaxis.get_major_formatter().set_scientific(False)
  ax1.yaxis.get_major_formatter().set_scientific(False)

  ax1.set_title(f"DIFERENCIA DE PUNTOS")
  ax1.text(-.3, -.25, f'archivo: {evento_name}'             ,fontsize=10, fontfamily='monospace', ha='left', va='bottom', transform=ax1.transAxes)
  ax1.text(0.78, -.25, f'max_val: {max_val}\nmx: {m_diff}'  ,fontsize=12, fontfamily='monospace', ha="left", va='bottom', transform=ax1.transAxes)
  ax1.grid(True)

  plt.savefig(graficos_dir+evento_name.replace(".json", "_g2.jpg"), bbox_inches='tight')