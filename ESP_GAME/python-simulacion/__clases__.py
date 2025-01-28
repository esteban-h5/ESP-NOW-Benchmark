from random import randint, choice, choices
import random
from dotenv import load_dotenv, dotenv_values 
from time import sleep
import numpy as np, os, sys

wd = os.path.dirname(os.path.realpath(__file__))

env_path = f"{wd}\\var.env"
# env_path = f"{wd}/var.env" #UNIX POSIX

if not load_dotenv(dotenv_path=env_path):
    input(f"Error al importar archivo de configuracion.\n[{wd}\\var.env]\n Enter para cerrar")
    exit(1)

max_pts     = int(os.getenv("MAX_PTS")) # type: ignore
max_partidos = int(os.getenv("MAX_PARTIDOS")) # type: ignore

max_jugadas = int(os.getenv("MAX_JUGADAS")) # type: ignore
min_jugadas = int(os.getenv("MIN_JUGADAS")) # type: ignore

n_est_A = int(os.getenv("ESTRATEGIA_A")) # type: ignore
n_est_B = int(os.getenv("ESTRATEGIA_B")) # type: ignore

prob_A = int(os.getenv("PROB_A")) # type: ignore
prob_B = int(os.getenv("PROB_B")) # type: ignore

log_minimo      = os.getenv("LOG_MINIMO")
sufix           = os.getenv("SUFIJO_NOMBRE")
toggle_print    = os.getenv("TOGGLE_PRINT")
more_info_json   = os.getenv("MORE_INFO_EVENT")
guardar_jugadas = os.getenv("GUARDAR_JUGADAS")

ignorar_errados = os.getenv("IGNORAR_ERRADOS")

prefix = os.getenv("PREFIJO_NOMBRE")
info = os.getenv("INFO_PARTIDO")

if len(sys.argv) != 0:
  args = sys.argv
  if "-h" in args:
     print("Argumentos:\n"+
           "-pA: probabilidad de A 0-100\n"+
           "-pB: probabilidad de B 0-100\n"+
           "-eA: estrategia usada por A {1,2,3}\n"
           "-eB: estrategia usada por B {1,2,3}\n"
      )
     exit(0)

  if "-pA" in args:
    prob_A = int(args[args.index("-pA")+1])
  if "-pB" in args:
    prob_B = int(args[args.index("-pB")+1])
  if "-eA" in args:
    n_est_A = int(args[args.index("-eA")+1])
  if "-eB" in args:
    n_est_B = int(args[args.index("-eB")+1])

evento_name = f"{prefix}estrategia_{n_est_A}{n_est_B}"

if max_pts != -1: 
       evento_name = f"{evento_name}_{max_pts}"

elif max_partidos != -1: 
       evento_name = f"{evento_name}_{max_partidos}"

if prob_A == prob_B:
       evento_name = f"{evento_name}_prob_{prob_A}"
else:
       evento_name = f"{evento_name}_prob_{prob_A}_{prob_B}"

if sufix != "":
    evento_name = f"{evento_name}_{sufix}"

evento_name = f"{evento_name}.json"

evento_path = f"{wd}\\eventos\\{evento_name}"

prob_A = int(prob_A)/100
prob_B = int(prob_B)/100

def eprint(texto,end="\n"):
    if toggle_print=="True": print(texto,end=end)

class MovimientoInvalidoError(Exception):
    def __init__(self, pre_pos, next_pos):
        super().__init__(f"Movimiento invalido, no se puede mover de {pre_pos} a {next_pos}")

class peer_state:
    receiving = "receiving"
    transmitting = "transmitting"
    processing = "processing"
    idle = "idle"


class Peer:
    def __init__(self, match, team, player_id, pos, ball_carrier):
        self.team = team
        self.match = match
        self.player_id = player_id

        self.posicion_inicial = pos
        self.posicion_actual = pos
        self.lookup_table = np.array([[0,0,0,0,0] if (_ == 0 or _ == 7) else [0,1,1,1,0] for _ in range(8) ])

        self.ball_carrier = ball_carrier #Definir carrier al constrir objeto
        self.aim_pos = [-1,-1]

        if self.ball_carrier:
            self.match.ball_poss = self.posicion_actual
            self.state = peer_state.processing
            
        else:
            self.state = peer_state.receiving

    def lanzar(self):
        if self.team == "B":
            self.aim_pos = [randint(0,5), randint(0,2)]
        if self.team == "A":
            self.aim_pos = [randint(0,5), randint(6,8)]
            
        self.match.ball_poss = [0,4]
        self.ball_carrier = False
        self.match.next_ball_pos = self.aim_pos

        return self.aim_pos
    
    def mover_atajar(self, next_pos):
        #levantar excepcion si nueva posicion es mayor a casillas adyacentes como el rey de ajedrez
        if (
            next_pos[0] > self.posicion_actual[0] + 1 or
            next_pos[0] < self.posicion_actual[0] - 1 or

            next_pos[1] > self.posicion_actual[1] + 1 or
            next_pos[1] < self.posicion_actual[1] - 1
        ):
            raise MovimientoInvalidoError(self.posicion_actual, next_pos)
        
        else:
            self.posicion_actual = next_pos
            self.ball_carrier = True

    def espacios_cubiertos(self, posicion):
        # lookup table debe tener un contorno rodeado con 0, traslacion conciderada
        x_pos,y_pos = posicion

        if self.team == "B": y_pos = y_pos - 6
        try:
            if self.lookup_table[x_pos+1][y_pos+1] == 0: raise IndexError(f"Fuera de los margenes en tabla de busqueda.")
        except IndexError as e:
            eprint(e)
            return -1

        # Movimiento con kernel cuadrada (Rey de ajedrez)
        kernel = self.lookup_table[ x_pos:x_pos+3, y_pos:y_pos+3 ]
        return np.sum(kernel) 
    

class match_state:
    waiting = "waiting"
    running = "running"
    finished = "finished"
    idle = "idle"


class Match:
    def __init__(self, tick_X_seg=.05 ):
        self.pts_equipo_A = 0
        self.pts_equipo_B = 0

        self.player_list = []

        self.player_list_A = []
        self.player_list_B = []
        
        self.state = match_state.idle
        self.max_partidos = max_partidos
        
        self.next_ball_pos = [-1,-1]
        self.ball_poss = [-1,-1]

        self.tick_actual = 0
        self.tick_X_seg = tick_X_seg
        self.cantidad_jugadas = 0
        self.cantidad_pts_errados = 0
        self.cantidad_partidos = 0
        self.n_jugada = 1
    
    def agregar_jugador(self, jugador):
        self.player_list.append(jugador)
        
        if jugador.team == "A":
                    self.player_list_A.append(jugador)

        if jugador.team == "B":
                    self.player_list_B.append(jugador)
    
    def get_carrier(self):
            for player in self.player_list:
                if player.ball_carrier:
                    return player
            else: 
                return None

    def siguiente(self):
            self.cantidad_jugadas += 1
            self.n_jugada += 1
                            
            if self.state == match_state.running:
                self.tick_actual += 1

            sleep(self.tick_X_seg)
    
    def terminar_partido(self, equipo_ganador, errado=False):

        if not errado:
            
            if equipo_ganador == "A":
                self.pts_equipo_A += 1

                # carrier = choice(self.player_list_B)
                # for p in self.player_list_A: p.carrier = False

                carrier = choice(self.player_list_A)


            if equipo_ganador == "B":
                self.pts_equipo_B += 1

                # carrier = choice(self.player_list_A)
                # for p in self.player_list_B: p.carrier = False

                carrier = choice(self.player_list_B)


            carrier.ball_carrier = True
            self.cantidad_partidos += 1
                
        else:
            self.cantidad_pts_errados += 1

        if self.cantidad_partidos == max_partidos:
            self.state = match_state.finished

        self.cantidad_jugadas = 0
        self.n_jugada = 0
        
        for player in self.player_list:
            player.posicion_actual = player.posicion_inicial



if __name__ == '__main__':
       input("Se debe ejecutar main.py. Enter para cerrar")
       exit(1)