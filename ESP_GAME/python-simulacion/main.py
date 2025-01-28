from __clases__ import *

from datetime import datetime
import time, json

c_good = "#3bff6f"
c_bad = "#fcb3b3"
c_blue = "#78beff"
c_gray = "#a8a8a8"

lista_eventos = []
buffer_lista_eventos = []

def agregar_evento(registro_actual, errado=False, final_partido=False):
    global lista_eventos
    global buffer_lista_eventos

    if errado and ignorar_errados=="True":
        buffer_lista_eventos = []
        return
        
    if log_minimo=="True":
        try:
            del registro_actual["event"]
            del registro_actual["agents_pos"]
            del registro_actual["cantidad_movimientos"]
        except KeyError: pass
            
    if more_info_json=="False":
        try:
            del registro_actual["box_list"]
        except KeyError: pass

    if not final_partido:
        if guardar_jugadas != "False": 
            buffer_lista_eventos.append(registro_actual)
    else:
        if guardar_jugadas != "False":
            [lista_eventos.append(evento) for evento in buffer_lista_eventos+[registro_actual] if not errado]
        else:
            [lista_eventos.append(evento) for evento in [registro_actual] if not errado]

        buffer_lista_eventos = []

#probabilidad = 1 == 100% probabilidad de devolver actor
def devolver_otro_actor(actor, lista_jugadores, probabilidad):

  global otro_actor
  otro_actor = random.random() > probabilidad

  if otro_actor:
    lista_jugadores.remove(actor)
    return choices(lista_jugadores, k=1)[0]

  else:
    return actor

def distancia(pos1, pos2):
    return abs(pos1[0] - pos2[0]) + abs(pos1[1] - pos2[1])

def estrategia_1(lista_jugadores, partido=None, prob_actor = None):
    return choices(lista_jugadores, k=1)[0]

def estrategia_2(lista_jugadores, partido, prob_actor = 1):

    dist_peer_1 = distancia(lista_jugadores[0].posicion_actual, partido.next_ball_pos)
    dist_peer_2 = distancia(lista_jugadores[1].posicion_actual, partido.next_ball_pos)

    #Distancia menor decide quien se mueve
    if dist_peer_1 < dist_peer_2: jugador_actor = lista_jugadores[0]
    if dist_peer_2 < dist_peer_1: jugador_actor = lista_jugadores[1]
    if dist_peer_1 == dist_peer_2: jugador_actor = estrategia_1(lista_jugadores)

    #probabilidad de devolver el otro actor
    return devolver_otro_actor(jugador_actor, lista_jugadores, probabilidad=prob_actor)

def estrategia_3(lista_jugadores, partido, prob_actor = 1):

    #Obtener distancia de cada posicion de los agentes a la pelota
    dist_peer_1 = distancia(lista_jugadores[0].posicion_actual, partido.next_ball_pos)
    dist_peer_2 = distancia(lista_jugadores[1].posicion_actual, partido.next_ball_pos)

    # print(f"distancia: {dist_peer_1} - {dist_peer_2}")
    
    #Distancia menor decide quien se mueve
    if dist_peer_1 < dist_peer_2: jugador_actor = lista_jugadores[0]
    if dist_peer_2 < dist_peer_1: jugador_actor = lista_jugadores[1]

    if dist_peer_1 == dist_peer_2:
        
        #Obtener mejor jugador según espacios vacíos
        espacios_peer_1 = lista_jugadores[0].espacios_cubiertos(partido.next_ball_pos)
        espacios_peer_2 = lista_jugadores[1].espacios_cubiertos(partido.next_ball_pos)

        #Menos espacios cubiertos debe moverse
        if espacios_peer_1 < espacios_peer_2: jugador_actor = lista_jugadores[0]
        if espacios_peer_1 > espacios_peer_2: jugador_actor = lista_jugadores[1]

        if espacios_peer_1 == espacios_peer_2: jugador_actor = estrategia_1(lista_jugadores)
    
    return devolver_otro_actor(jugador_actor, lista_jugadores, probabilidad=prob_actor)


if n_est_A == 1: 
   estrategia_A = estrategia_1
if n_est_A == 2: 
   estrategia_A = estrategia_2
if n_est_A == 3: 
   estrategia_A = estrategia_3

if n_est_B == 1: 
   estrategia_B = estrategia_1
if n_est_B == 2: 
   estrategia_B = estrategia_2
if n_est_B == 3: 
   estrategia_B = estrategia_3


########################################################################################################################################################################
#VARIAR ESTRATEGIA POR EQUIPO

def ejecutar_estrategia(peer_team, peer_list, partido):

    if peer_team == "A":
        jugador_actor = estrategia_A(peer_list, partido, prob_actor=prob_A)

    if peer_team == "B":
        jugador_actor = estrategia_B(peer_list, partido, prob_actor=prob_B)
        
    return jugador_actor

########################################################################################################################################################################

partido = Match(tick_X_seg=0)
visualizar = True

if max_pts != -1: eprint(f"Buscando el mejor de {max_pts} puntos")
if max_partidos != -1: eprint(f"Realizando {max_partidos} partidos")

peer_A1 = Peer(partido, "A", 1, [1,1], ball_carrier=True)
peer_A2 = Peer(partido, "A", 2, [4,1], ball_carrier=False)

peer_B1 = Peer(partido, "B", 1, [1,7], ball_carrier=False)
peer_B2 = Peer(partido, "B", 2, [4,7], ball_carrier=False)

[ partido.agregar_jugador(peer) for peer in [peer_A1, peer_A2, peer_B1, peer_B2] ]

t_inicio = time.time()

#############################################################################################
#INICIO DE EJECUCION
print("Comenzando partido con la siguiente configuración:",end="")
print(f"\n\t - nombre_archivo:{evento_name}\n\t - estr_A:{n_est_A}\n\t - estr_B:{n_est_B}\n\t - prob_A:{int(prob_A*100)}\n\t - prob_B:{int(prob_B*100)}\n\t - max_pts:{max_pts}\n\t - max_partidos:{max_partidos}\n\t - max_jugadas:{max_jugadas}\n\t - min_jugadas:{min_jugadas}\n\t - toggle_print:{toggle_print}")
sleep(3)

while partido.state != match_state.finished:

  registro_actual = {
                      "time":partido.tick_actual,
                      "n_jugada":partido.n_jugada,
                      "agents_pos":[player.posicion_actual for player in partido.player_list],
                      "pts_A":partido.pts_equipo_A,
                      "pts_B":partido.pts_equipo_B,
                     }
  
  #Comenzar partido
  partido.state = match_state.running

  #Obtener el jugador con la pelota
  carrier = partido.get_carrier()

  #Alguien tiene la pelota
  if carrier != None:
    
    #Registro
    subreg = registro_actual.copy()
    subreg["event"] = "decidiendo"
    subreg["box_list"] = [ 
                          [carrier.posicion_actual, c_good],
                        ]
    agregar_evento(subreg)

    #Lanzarla a posición aleatoria de bando contrario
    next_pos = carrier.lanzar() #ANUNCIAR POSICION
    eprint(f"carrier {carrier.team}{carrier.player_id} lanza la pelota")
    
    #Registro
    subreg = registro_actual.copy()
    subreg["event"] = "lanzar"
    subreg["box_list"] = [ 
                          [carrier.posicion_actual, c_good],
                          [[1,4], c_blue],
                          [partido.next_ball_pos, c_gray] 
                        ]
    agregar_evento(subreg)
  
  #Pelota en el aire
  else:

    #Registro
    subreg = registro_actual.copy()
    subreg["event"] = "decidiendo"
    subreg["box_list"] = [ 
                                    [[1,4], c_good],
                                    [partido.next_ball_pos, c_blue]
                                  ]
    agregar_evento(subreg)

    #Pelota cae en cancha equipo A
    if partido.next_ball_pos[1] in [0,1,2]:
      peer_list = [peer_A1, peer_A2]
      peers_team = "A"

    #Pelota cae en cancha equipo B
    if partido.next_ball_pos[1] in [6,7,8]:
      peer_list = [peer_B1, peer_B2]
      peers_team = "B"

    otro_actor = None
    jugador_actor = ejecutar_estrategia(peers_team, peer_list, partido)
       
    #Registro
    subreg = registro_actual.copy()

    if otro_actor:
      subreg["event"] = "otro actor se mueve"
    else: 
      subreg["event"] = "actor se mueve"
    
    subreg["box_list"] = [ 
                          [jugador_actor.posicion_actual, c_blue],
                          [partido.next_ball_pos, c_blue]
                        ]    
    agregar_evento(subreg)

    try:
      #Intenar de mover jugador actor
      jugador_actor.mover_atajar(partido.next_ball_pos)
      eprint(f"jugador {jugador_actor.team}{jugador_actor.player_id} ataja")

    except MovimientoInvalidoError:

      punto_errado = (max_jugadas != -1 and partido.cantidad_jugadas > max_jugadas) or (min_jugadas != -1 and partido.cantidad_jugadas < min_jugadas)

      if jugador_actor.team == "A":
        print(f"punto para B en {partido.cantidad_jugadas} movimientos")
        registro_actual["ganador"] = "B"
        partido.terminar_partido(equipo_ganador="B", errado=punto_errado)

      if jugador_actor.team == "B":
        print(f"punto para A en {partido.cantidad_jugadas} movimientos")
        registro_actual["ganador"] = "A"
        partido.terminar_partido(equipo_ganador="A", errado=punto_errado)

      eprint(f"jugador {jugador_actor.team}{jugador_actor.player_id} perdio la pelota")

      if not punto_errado:
        print(f"[{partido.cantidad_partidos}] A:{partido.pts_equipo_A} - B:{partido.pts_equipo_B}\n")
      else:
        print(f"[PUNTO ERRADO][{partido.cantidad_partidos}]\n A:{partido.pts_equipo_A} - B:{partido.pts_equipo_B}\n")

      registro_actual["cantidad_movimientos"] = partido.cantidad_jugadas
      registro_actual["event"] = "punto"
      registro_actual["box_list"] = [ [jugador_actor.posicion_actual, c_bad], [partido.next_ball_pos, c_bad]]
      registro_actual["pts_A"] = partido.pts_equipo_A
      registro_actual["pts_B"] = partido.pts_equipo_B

      agregar_evento(registro_actual, errado=punto_errado, final_partido=True)
  
  if partido.state == match_state.finished or partido.pts_equipo_A == max_pts or partido.pts_equipo_B == max_pts:
    
    partido.state = match_state.finished
    
    subreg = registro_actual.copy()
    subreg["event"] = "final"
    subreg["cantidad_movimientos"] = partido.cantidad_jugadas
    subreg["box_list"] = [ 
                                [[1,4], c_bad],
                              ]    
    agregar_evento(subreg, final_partido=True)

    print(f"PARTIDO TERMINADO - A:{partido.pts_equipo_A} - B:{partido.pts_equipo_B}\n")
  else:
    partido.siguiente()
  
else:
  t_final = time.time()
  minutos_de_ejecucion = (t_final - t_inicio)/60

  registro = {"info":info,
              "config":f"estr_A:{n_est_A},estr_B:{n_est_B},prob_A:{int(prob_A*100)},prob_B:{int(prob_B*100)},max_pts:{max_pts},max_partidos:{max_partidos},max_jugadas:{max_jugadas},min_jugadas:{min_jugadas},toggle_print:{toggle_print}",
              "final":f"A:{partido.pts_equipo_A} - B:{partido.pts_equipo_B} - pts_errados:{partido.cantidad_pts_errados}",
              "tiempo_total":minutos_de_ejecucion,
              "more_info":more_info_json,
              "secuencia":lista_eventos
              }
  
  if os.path.exists(evento_path):
    input(f"archivo {evento_name} existente, enter para continuar y sobreescribir...")
    
  with open(evento_path, "w") as f: 
    print("Guardando archivo de eventos")
    json.dump(registro, f)

print("Programa terminado")
print(f"nombre json: {evento_name}")