#ifndef mod_esp    
#define mod_esp

#include <WiFi.h>

#include <esp_wifi.h>
#include <esp_now.h>
#include <esp_random.h>

#include "esp_system.h"

#if  !defined(CONFIG_IDF_TARGET_ESP32) && !defined(CONFIG_IDF_TARGET_ESP32S3)
  #error "Modelo de ESP32 no reconocido"
#endif

#if defined(CONFIG_IDF_TARGET_ESP32S3)
  #include <Adafruit_NeoPixel.h>
  extern Adafruit_NeoPixel pixels;
#endif

#if defined(CONFIG_IDF_TARGET_ESP32)
  #ifndef LED_BUILTIN
  #define LED_BUILTIN 2
  #endif
#endif

struct s_peer_info {
  char team[2];
  int id;
  
  uint8_t* mac_addr;
  bool ball_carrier;

  int pos[2];
};

struct s_match_info {
    bool ping_recibido;
    bool ping_enviado;
    
    bool agentes_listos;
    bool partido_activo;
    bool partido_idle;
    
    bool pelota_atajada;
    char ultimo_equipo_carrier[2];

    int pts_A;
    int pts_B;

    int t_actual;
    
    int max_jugadas;
    int max_partidos;

    int filas_cancha;
    int columnas_cancha;
};

struct msg {
    uint8_t cb_id;
    char msg[247];
};

struct msg_coor {
    uint8_t cb_id;
    char team[2];
    int id;
    bool carrier;
    int pos[2];
};
struct msg_rst {
    uint8_t cb_id;
    char carrier_team[2];
    int carrier_id;
};
struct msg_jugada {
    uint8_t cb_id;
    int pos[2];
    char peer_team[2];
    int peer_id;
};
struct msg_coop {
    uint8_t cb_id;
    int peer_id; //Usado para más de un agente
    
    uint32_t val_est_1;
    uint8_t val_est_2;
    int val_est_3;
};

struct msg_coop_end {
    uint8_t cb_id;
    int peer_id; //Usado para más de un agente
    int est_id;
};

extern esp_now_peer_info_t peerInfo_buffer;
extern s_match_info match_info;
extern int len_lista_agentes;

extern uint8_t mac_broadcast[6];

extern uint8_t mac_addr_A1[6];
extern uint8_t mac_addr_A2[6];
extern uint8_t mac_addr_B1[6];
extern uint8_t mac_addr_B2[6];

extern uint8_t mac_addr_M[6];

extern s_peer_info peer_A1;
extern s_peer_info peer_A2;
extern s_peer_info peer_B1;
extern s_peer_info peer_B2;

extern uint8_t* lista_mac[4];

extern s_peer_info* lista_agentes[];
extern s_peer_info* lista_agentes_A[];
extern s_peer_info* lista_agentes_B[];

extern int len_lista_agentes;
extern esp_err_t state;

void PrintDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void imprimirData(const uint8_t* data, size_t len);
void error_pedir_reinicio();
void f_hacer_pase(msg_jugada* pase, s_peer_info* peer);
void print_peerInfo(s_peer_info* peer);
void f_punto(msg_rst* msg, char team_ganador, char* info);

void f_calcular_estrategias(msg_coop* msg, msg_jugada* msg_ball, s_peer_info* peer);
void f_calcular_decision(msg_coop_end* desition_snd, msg_coop* est_p1, msg_coop* est_p2);

void led_conteo();
void led_idle_rgb();
void led_punto_r();
void led_operando_b();
void led_atrapa_g();

#endif