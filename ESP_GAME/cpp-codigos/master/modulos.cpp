#include "modulos.h"

uint8_t mac_addr_M[6] = {0xEC, 0xDA, 0x3B, 0x45, 0x2C, 0xD0}; //ESP32-S3 M

uint8_t mac_addr_A1[6]  = {0x7C, 0xDF, 0xA1, 0xFF, 0x2B, 0x30}; //[E1] ESP32-S3 A1
uint8_t mac_addr_A2[6] = {0x68, 0xB6, 0xB3, 0x21, 0x09, 0xE8}; //[E2] ESP32-S3 A2

uint8_t mac_addr_B1[6] = {0xB8, 0xD6, 0x1A, 0xAB, 0x4D, 0x8C}; //[E1] ESP32 B1
uint8_t mac_addr_B2[6] = {0xB8, 0xD6, 0x1A, 0xAC, 0x03, 0x60}; //[E2] ESP32 B2

uint8_t mac_broadcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

s_peer_info peer_A1 = {
    .team = "A",
    .id = 1,
    .mac_addr = mac_addr_A1,
    .ball_carrier = false,
    .pos = {-1, -1}
}; 

s_peer_info peer_A2 = {
    .team = "A", 
    .id = 2,
    .mac_addr = mac_addr_A2,
    .ball_carrier = false,
    .pos = {-1, -1}
}; 

s_peer_info peer_B1 = {
    .team = "B",
    .id = 1,
    .mac_addr = mac_addr_B1,
    .ball_carrier = false,
    .pos = {-1, -1}
}; 

s_peer_info peer_B2 = {
    .team = "B",
    .id = 2,
    .mac_addr = mac_addr_B2,
    .ball_carrier = false,
    .pos = {-1, -1}
}; 

s_peer_info*  lista_agentes[]      = {&peer_A1, &peer_A2, &peer_B1, &peer_B2};

s_peer_info*  lista_agentes_A[]    = {&peer_A1, &peer_A2};
s_peer_info*  lista_agentes_B[]    = {&peer_B1, &peer_B2};

int           len_lista_agentes    = sizeof(lista_agentes)/sizeof(lista_agentes[0]);

s_match_info match_info = {
    .ping_recibido = false,
    .ping_enviado  = false,
    
    .agentes_listos = true,
    .partido_activo = false,
    .pelota_atajada = true,
    
    .pts_A = 0,
    .pts_B = 0,

    .t_actual = 0,
    .max_jugadas = -1,
    .max_partidos = -1,

    //GRILLA orden 6x3
    .filas_cancha = 6,
    .columnas_cancha = 3,

};

esp_now_peer_info_t peerInfo_buffer = {
  .channel = 0,
  .encrypt = false
};

esp_err_t state;

void PrintDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    // Mostrar información del mensaje enviado
    Serial.print("Enviado a MAC: ");
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", mac_addr[i]);
        if (i < 5) Serial.print(":");
    }
    // Mostrar estado del envío
    Serial.print(" - Estado: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Éxito" : "Error");
}

//Imprimir Contenido Hexadecimal de un mensaje ESP-NOW
void imprimirData(const uint8_t* data, size_t len) {
    Serial.print("Contenido (Hex): ");
    for (size_t i = 0; i < len; i++) {
        Serial.printf("%02X ", data[i]); // Imprimir en formato hexadecimal
    }
    Serial.println();

    Serial.print("Contenido (Texto): ");
    for (size_t i = 0; i < len; i++) {
        if (isprint(data[i])) { // Si es imprimible, lo muestra
            Serial.print((char)data[i]);
        } else {
            Serial.print("."); // Si no, imprime un punto
        }
    }
    Serial.println();
};

void f_hacer_pase(msg_jugada* pase, s_peer_info* peer) {
  strcpy(pase->peer_team, peer->team);
  pase->peer_id = peer->id;
  pase->pos[0] = esp_random() % match_info.filas_cancha;      // Coordenada X en la grilla (0 a 5)
  pase->pos[1] = esp_random() % match_info.columnas_cancha;   // Coordenada Y en la grilla (0 a 2)
}

void f_punto(msg_rst* msg, char team_ganador, char* info) {
  
  Serial.printf("{t_actual:%d, pts_A:%d, pts_B:%d, ganador:\"%c\", info:\"%s\"},", match_info.t_actual, match_info.pts_A, match_info.pts_B, team_ganador, info);
  Serial.println();
  if (team_ganador == 'A') { 
    match_info.pts_A += 1;
    s_peer_info** lista = lista_agentes_B; //DEFINE QUIEN PARTE EN CASO DE QUE GANE CIERTO EQUIPO

    int len_lista = sizeof(lista) / sizeof(lista[0]);
    int randint = esp_random() % len_lista;
    s_peer_info* peer = lista[randint];

    match_info.ultimo_equipo_carrier[0] = peer->team[0];
    msg->carrier_team[0] = peer->team[0];
    msg->carrier_id = peer->id;
  }

  if (team_ganador == 'B') { 
    match_info.pts_B += 1;
    s_peer_info** lista = lista_agentes_A; //DEFINE QUIEN PARTE EN CASO DE QUE GANE CIERTO EQUIPO

    int len_lista = sizeof(lista) / sizeof(lista[0]);
    int randint = esp_random() % len_lista;
    s_peer_info* peer = lista[randint];

    msg->carrier_team[0] = peer->team[0];
    msg->carrier_id = peer->id;
  }
}

int calcular_espacios_cubiertos(int pos[2]) {
    int espacios_cubiertos = 0;
    int k_radio = 1; //Kernel radio

    // Recorrer la grilla en el rango del radio
    for (int dx = -k_radio; dx <= k_radio; ++dx) {
        for (int dy = -k_radio; dy <= k_radio; ++dy) {
            int x = pos[0] + dx;
            int y = pos[1] + dy;

            // Verificar que el espacio esté dentro de la grilla
            if (x >= 0 && x < match_info.columnas_cancha &&
                y >= 0 && y < match_info.filas_cancha ) {
                espacios_cubiertos++;
            }
        }
    }
    return espacios_cubiertos;
}

//Construir msg_coop a partir de msg_jugada y posicion actual de peer
void f_calcular_estrategias(msg_coop* msg, msg_jugada* msg_ball, s_peer_info* peer){

  int pos_x = peer->pos[0];
  int pos_y = peer->pos[1];

  msg->val_est_3 = calcular_espacios_cubiertos(msg_ball->pos);
  
  msg->val_est_2 = abs(peer->pos[0] - msg_ball->pos[0]) + abs(peer->pos[1] - msg_ball->pos[1]);
  if (msg->val_est_2 > 1) { msg->val_est_2 = -1; } //MOVIMIENTO PROHIBIDO no moverse más de 1 espacio

  msg->val_est_1 = esp_random();
}

void f_calcular_decision(msg_coop_end* desition_snd, msg_coop* est_p1, msg_coop* est_p2) {
    
    //Comparar distancias
    if (est_p1->val_est_2 > est_p2->val_est_2) {
        desition_snd->peer_id = 1;
        desition_snd->est_id = 2;
        return;
    } else if (est_p1->val_est_2 < est_p2->val_est_2) {
        desition_snd->peer_id = 2;
        desition_snd->est_id = 2;
        return;
    } else if(est_p1->val_est_2 == -1 and est_p2->val_est_2  == -1){
      desition_snd->peer_id = -1;
      desition_snd->est_id = 2;
      return;
    }

    //Comparar espacio usado actualmente
    if (est_p1->val_est_3 > est_p2->val_est_3) {
        desition_snd->peer_id = 1;
        desition_snd->est_id = 3;
        return;
    } else if (est_p1->val_est_3 < est_p2->val_est_3) {
        desition_snd->peer_id = 2;
        desition_snd->est_id = 3;
        return;
    }

    //Desempate con variable aleatoria
    if (est_p1->val_est_1 > est_p2->val_est_1) {
        desition_snd->peer_id = 1;
        desition_snd->est_id = 1;
        return;
    } else if (est_p1->val_est_1 < est_p2->val_est_1) {
        desition_snd->peer_id = 2;
        desition_snd->est_id = 1;
        return;
    }

    //Variables iguales, asumir partida perdida
    desition_snd->peer_id = -1;
    desition_snd->est_id = -1;
}

void print_peerInfo(s_peer_info* peer){
  Serial.printf("Equipo: %s | ID: %d | MAC: %02X:%02X:%02X:%02X:%02X:%02X | Portador: %s | Pos: (%d, %d)\n", peer->team, peer->id, peer->mac_addr[0], peer->mac_addr[1], peer->mac_addr[2], peer->mac_addr[3], peer->mac_addr[4], peer->mac_addr[5], peer->ball_carrier ? "Sí" : "No", peer->pos[0], peer->pos[1]);
  Serial.println();
}

//Interrupcion para detener programa
void error_pedir_reinicio() {
    esp_now_deinit();
    
    led_punto_r();
    Serial.println("Problemas en la ejecución, por favor reinicie manualmente el dispositivo para reintentar.");

    while (true) {
        delay(1000);  // Reduce consumo innecesario en el bucle
    }
};


#if defined(CONFIG_IDF_TARGET_ESP32S3)
  Adafruit_NeoPixel pixels(1, 48, NEO_GRB + NEO_KHZ800);
  int intencidad = 10;

  void led_conteo() {
    pixels.setPixelColor(0, pixels.Color(0, 0, 0)); pixels.show();

    Serial.println("Inicio en:");
    delay(1000);

    Serial.println("3");
    delay(1000);
    
    Serial.println("2");
    pixels.setPixelColor(0, pixels.Color(intencidad, 0, 0)); pixels.show();
    delay(1000);
    
    Serial.println("1");
    pixels.setPixelColor(0, pixels.Color(intencidad, intencidad, 0)); pixels.show();
    delay(1000);

    pixels.setPixelColor(0, pixels.Color(0, intencidad, 0)); pixels.show();
    Serial.println("Anunciando Inicio");
  };

  void led_coop_y()     { pixels.setPixelColor(0, pixels.Color(intencidad, intencidad, 0) ); pixels.show(); };
  void led_idle_rgb()   { pixels.setPixelColor(0, pixels.Color(intencidad, intencidad, intencidad) ); pixels.show(); };
  void led_punto_r()    { pixels.setPixelColor(0, pixels.Color(intencidad, 0, 0) ); pixels.show(); };
  void led_operando_b() { pixels.setPixelColor(0, pixels.Color(0, 0, intencidad) ); pixels.show(); };
  void led_atrapa_g()   { pixels.setPixelColor(0, pixels.Color(0, intencidad, 0) ); pixels.show(); };
#endif


#if defined(CONFIG_IDF_TARGET_ESP32)
  void led_conteo() {
    
    digitalWrite(LED_BUILTIN, HIGH);  
    Serial.println("Inicio en:");
    delay(1000);

    Serial.println("3");
    digitalWrite(LED_BUILTIN, LOW);   
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);    
    delay(900);
    
    Serial.println("2");
    digitalWrite(LED_BUILTIN, LOW);   
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);    
    delay(900);
    
    Serial.println("1");
    digitalWrite(LED_BUILTIN, LOW);   
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);    
    delay(900);

    digitalWrite(LED_BUILTIN, LOW);  
    Serial.println("Anunciando Inicio");
  };

  void led_idle_rgb() { digitalWrite(LED_BUILTIN, LOW); };
  void led_punto_r()
  {
    digitalWrite(LED_BUILTIN, LOW); 
    delay(100); 
    digitalWrite(LED_BUILTIN, HIGH); 
    delay(100); 
    digitalWrite(LED_BUILTIN, LOW);
    delay(100); 
    digitalWrite(LED_BUILTIN, HIGH);    
    delay(100); 
    digitalWrite(LED_BUILTIN, LOW);
    delay(100); 
    digitalWrite(LED_BUILTIN, HIGH);    
    delay(100); 
    digitalWrite(LED_BUILTIN, LOW); 
  };

  void led_operando_b() { digitalWrite(LED_BUILTIN, HIGH); };

  void led_atrapa_g() {   
    digitalWrite(LED_BUILTIN, LOW);   
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);    
    delay(100); 
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);  
  };

  void led_coop_y() {     
    digitalWrite(LED_BUILTIN, LOW);   
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);    
    delay(100); 
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);   
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);  
  };


#endif