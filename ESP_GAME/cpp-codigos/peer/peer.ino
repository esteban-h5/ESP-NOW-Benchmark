#include "modulos.h"

s_peer_info* peer_actual;
s_peer_info* peer_aliado; //Utilizar como lista para más de dos peers

msg ping_rcv;
msg ping_snd = { .cb_id = 201 };

msg_coor coor_rcv;

msg_rst expt_coor = { .cb_id = 104 };

msg_rst expt_tiempo;
msg_rst expt_pierde;

msg_jugada pase = { .cb_id = 10 };
msg_jugada atajada = { .cb_id = 11 };
msg_jugada atajada_rcv;
msg_jugada pase_rcv;

msg_coop val_est_snd;
msg_coop val_est_rcv;

msg_coop_end val_desition_snd = { .cb_id = 52 };
msg_coop_end val_desition_rcv;

msg_coop_end val_desition_local;

msg_rst expt_coop_inalcanzable = { .cb_id = 60 };
msg_rst expt_no_coop = { .cb_id = 61 };

void cb_list(const esp_now_recv_info* recv_info, const uint8_t* data, int len) {
  if (len < 1) { Serial.println("Mensaje inválido"); } 
  else {
    uint8_t cb_id = data[0];
    Serial.printf("(cb_list recibe %d)", cb_id); Serial.println(); //DEBUG

    switch (cb_id) {
      //msg         PING DE RESPUESTA 
      case 200:
        if (len >= sizeof(msg)) {
          memcpy(&ping_rcv, data, sizeof(msg));

          if (strncmp(ping_rcv.msg, "estado", sizeof(ping_rcv.msg)) == 0) {

            Serial.println("Ping recibido");
            match_info.ping_recibido = true;

            strcpy(ping_snd.msg, "respuesta");
            state = esp_now_send(mac_addr_M, (uint8_t*)&ping_snd, sizeof(ping_snd));

            if (state == ESP_ERR_ESPNOW_NOT_FOUND) {
              Serial.println("Maestro Desconectado");
            } else if (state != OK) {
              Serial.printf("ERROR al enviar [%s]", esp_err_to_name(state));
              Serial.println();
            } else if (state == OK) {
              Serial.println("Repuesta enviada");
              match_info.ping_enviado = true;
            }
          }
        } else {
          Serial.printf("Largo incorrecto. cb_id: %d", cb_id);
          imprimirData(data, len);
          Serial.println();
        }
        break;

      //msg_coor    COORDINACION DE POSICION 
      case 202:
        //BROADCAST
        if (len >= sizeof(msg_coor)) {
          
          memcpy(&coor_rcv, data, sizeof(msg_coor));
          for (int i = 0; i < len_lista_agentes; i++) {
            s_peer_info* peer = lista_agentes[i];
            if (peer->team[0] == coor_rcv.team[0] && peer->id == coor_rcv.id) {

              peer->ball_carrier = coor_rcv.carrier;
              peer->pos[0] = coor_rcv.pos[0];
              peer->pos[1] = coor_rcv.pos[1];

              Serial.printf("Asignando ball Carrier: %s | Pos: (%d, %d)\n", peer->ball_carrier ? "Yes" : "No", peer->pos[0], peer->pos[1]);
              Serial.println();
            }
          }


        } else { Serial.printf("Largo incorrecto. cb_id: %d", cb_id); imprimirData(data, len); Serial.println(); }
        break;

      //msg         FIN DE COORDINACION 
      case 203:
        if (!match_info.partido_activo) {
          for (int i = 0; i < len_lista_agentes; i++) {
            s_peer_info* peer = lista_agentes[i];

            if (peer->pos[0] == -1 || peer->pos[1] == -1) {
              Serial.printf("Peer %s%d no se asignó correctamente", peer->team, peer->id);
              esp_now_send(mac_broadcast, (uint8_t*)&expt_coor, sizeof(msg_rst));

              error_pedir_reinicio();
              ///////////////////////
            }
          }
          Serial.println("Coordinacion realizada, esperando inicio de partido");
        } else {
          Serial.println("Coordinacion ya realizada, partido en curso...");
        }
        led_atrapa_g();


        break;

      //msg         INICIO DE PARTIDO 
      case 0:
        //BROADCAST
        match_info.partido_activo = true;
        break;

      //msg         FINAL DE PARTIDO 
      case 1:
        //BROADCAST
        match_info.partido_activo = false;
        break;

      //msg_jugada  PASE 
      case 10:
        //BROADCAST
        match_info.pelota_atajada = false;
        //Revisar equipo donde caerá la pelota y empezar coordinacion
        if (len >= sizeof(msg_jugada)) {
          memcpy(&pase_rcv, data, sizeof(msg_jugada));
          if (pase_rcv.peer_team[0] != peer_actual->team[0]){

            esp_now_unregister_recv_cb();
            esp_now_register_recv_cb(cb_coop);
            delay(500);

            //COOP PEER 1 COMIENZA COOPERACION
            if(peer_actual->id == 1){ 
              val_est_snd.cb_id = 50;
              f_calcular_estrategias(&val_est_snd, &pase_rcv, peer_actual);
              
              Serial.printf("Enviando mensaje: cb_id=%u, peer_id=%d, val_est_1=%u, val_est_2=%u, val_est_3=%d\n", 
                  val_est_snd.cb_id, val_est_snd.peer_id, val_est_snd.val_est_1, 
                  val_est_snd.val_est_2, val_est_snd.val_est_3);
              esp_now_send(peer_aliado->mac_addr, (uint8_t*)&val_est_snd, sizeof(msg_jugada)); 
            }
          }
          
        }
        break;

      //msg_jugada  ATAJADA 
      case 11:
        //BROADCAST
        // uint8_t pos[2]; //Nueva posicion
        // char peer_team[2]; //Jugador que ataja
        // int peer_id; //Jugador que ataja

        if (len >= sizeof(msg_jugada)) {
          memcpy(&atajada_rcv, data, sizeof(msg_jugada));
          for (int i = 0; i < len_lista_agentes; i++) {
            s_peer_info* peer = lista_agentes[i];
            if (atajada_rcv.peer_team[0] == peer->team[0] && atajada_rcv.peer_id == peer->id){
              peer->pos[0] = atajada_rcv.pos[0];
              peer->pos[1] = atajada_rcv.pos[1];
              peer->ball_carrier = true;
              match_info.pelota_atajada = true;
            }
          }
        }

        break;
      
      //msg_rst     EXCEPCION DE PERDIDA 
      case 100:
        //BROADCAST
        led_punto_r();
        if (len >= sizeof(msg_rst)) {
          memcpy(&expt_pierde, data, sizeof(msg_rst));
          for (int i = 0; i < len_lista_agentes; i++) {
            s_peer_info* peer = lista_agentes[i];
            if (expt_pierde.carrier_team[0] == peer->team[0] && expt_pierde.carrier_id == peer->id){ peer->ball_carrier = true; }
          }
        }
        delay(500);
        led_idle_rgb();

        break;

      //msg_rst     EXCEPCION DE TIEMPO 
      case 101:
        //broadcast
        Serial.println("Punto");
        led_punto_r();
        if (len >= sizeof(msg_rst)) {
          memcpy(&expt_tiempo, data, sizeof(msg_rst));
          for (int i = 0; i < len_lista_agentes; i++) {
            s_peer_info* peer = lista_agentes[i];
            if (expt_tiempo.carrier_team[0] == peer->team[0] && expt_tiempo.carrier_id == peer->id){ peer->ball_carrier = true; }
          }
        }
        delay(500);
        led_idle_rgb();
        break;

      //cb_id no registrado
      default:
        Serial.printf("cb_id %d no registrado", cb_id);
        Serial.println();
        imprimirData(data, len);
        break;
    }
  }
}

void cb_coop(const esp_now_recv_info* recv_info, const uint8_t* data, int len) {
  if (len < 1) { Serial.println("Mensaje inválido"); } 
  else {

    uint8_t cb_id = data[0];
    Serial.printf("(cb_coop recibe %d)", cb_id); Serial.println(); //DEBUG

    switch (cb_id) {
      //msg_coop      COORDINACION DE ESTRATEGIA PARA ID 2
      case 50:
        
        //COOP PEER 2 CALCULA ESTRATEGIAS CON PASE
        val_est_snd.cb_id = 51;
        f_calcular_estrategias(&val_est_snd, &pase_rcv, peer_actual);
        esp_now_send(peer_aliado->mac_addr, (uint8_t*)&val_est_snd, sizeof(msg_coop));
      
        //CALCULAR DESICION PARA ENVIARSELO A PEER 1
        if (len >= sizeof(msg_coop)) {
          memcpy(&val_est_rcv, data, sizeof(msg_coop));

          Serial.printf("Recibe mensaje: cb_id=%u, peer_id=%d, val_est_1=%u, val_est_2=%u, val_est_3=%d\n", 
              val_est_rcv.cb_id, val_est_rcv.peer_id, val_est_rcv.val_est_1, 
              val_est_rcv.val_est_2, val_est_rcv.val_est_3);

          f_calcular_decision(&val_desition_snd, &val_est_rcv, &val_est_snd);
          delay(500); //tiempo extra para que peer 1 calcule decisión
          Serial.printf("Enviando Decision: cb_id=%u, peer_id=%d, est_id=%d", val_desition_snd.cb_id, val_desition_snd.peer_id, val_desition_snd.est_id); Serial.println();

          esp_now_send(peer_aliado->mac_addr, (uint8_t*)&val_desition_snd, sizeof(msg_coop_end));
        }

        esp_now_unregister_recv_cb();
        esp_now_register_recv_cb(cb_list);
        led_operando_b();

        break;

      //msg_coop      PEER 1 CALCULA SU DECISION
      case 51:
        if (len >= sizeof(msg_coop)) {
          memcpy(&val_est_rcv, data, sizeof(msg_coop));
          f_calcular_decision(&val_desition_local, &val_est_snd, &val_est_rcv);
        }
        break;

      //msg_coop_end  COORDINAR COMPARACION CON DESISION DE ID 2
      case 52:
        if (len >= sizeof(msg_coop_end)) {
          memcpy(&val_desition_rcv, data, sizeof(msg_coop_end));

          if (val_desition_rcv.peer_id == -1 && val_desition_local.peer_id == -1) {
            esp_now_send(mac_addr_M, (uint8_t*)&expt_coop_inalcanzable, sizeof(msg_rst));
          }
          if (val_desition_rcv.peer_id != val_desition_local.peer_id) {
            esp_now_send(mac_addr_M, (uint8_t*)&expt_no_coop, sizeof(msg_rst));
          }
          
          //ANUNCIO OFICIAL DE ATAJADA 
          if (val_desition_rcv.peer_id == val_desition_local.peer_id) {
            esp_now_send(mac_broadcast, (uint8_t*)&atajada, sizeof(msg_jugada));
          }
        }
        esp_now_unregister_recv_cb();
        esp_now_register_recv_cb(cb_list);
        led_operando_b();

        break;

      //msg_rst       TIMEOUT PARA COORDINAR
      case 101:
        //broadcast
        Serial.println("Punto");
        led_punto_r();
        if (len >= sizeof(msg_rst)) {
          memcpy(&expt_tiempo, data, sizeof(msg_rst));
          for (int i = 0; i < len_lista_agentes; i++) {
            s_peer_info* peer = lista_agentes[i];
            if (expt_tiempo.carrier_team[0] == peer->team[0] && expt_tiempo.carrier_id == peer->id){ peer->ball_carrier = true; }
          }
        }
        esp_now_unregister_recv_cb();
        esp_now_register_recv_cb(cb_list);

        delay(500);
        led_idle_rgb();
        break;

      //cb_id no registrado
      default:
        Serial.printf("cb_id %d no registrado", cb_id);
        Serial.println();
        imprimirData(data, len);
        break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error al inicializar ESP-NOW");

    error_pedir_reinicio();
    ///////////////////////
  };

  #if defined(CONFIG_IDF_TARGET_ESP32S3)
    Serial.println("Configuración para ESP32-S3");
    pixels.begin();

  #elif defined(CONFIG_IDF_TARGET_ESP32)
    Serial.println("Configuración para ESP32");
    pinMode(LED_BUILTIN, OUTPUT);
  #endif

  led_idle_rgb();

  bool peer_actual_registrado = false;
  uint8_t peer_actual_Mac[6];  //Usado solo para comparar y encontrar peer_actual

  state = esp_wifi_get_mac(WIFI_IF_STA, peer_actual_Mac);
  if (state != ESP_OK) {
    Serial.println("No se pudo reconocer la Mac del peer actual");
    error_pedir_reinicio();
    ///////////////////////
  }

  Serial.println();
  Serial.println("Lista Jugadores:");
  for (int i = 0; i < len_lista_agentes; i++) {
    s_peer_info* peer = lista_agentes[i];

    Serial.printf("- %s%d: %02x:%02x:%02x:%02x:%02x:%02x",
                  peer->team, peer->id,
                  peer->mac_addr[0], peer->mac_addr[1], peer->mac_addr[2],
                  peer->mac_addr[3], peer->mac_addr[4], peer->mac_addr[5]);

    if (memcmp(peer->mac_addr, peer_actual_Mac, 6) == 0) {

      pase.peer_team[0] = peer->team[0];
      pase.peer_id = peer->id;

      peer_actual = peer;
      peer_actual_registrado = true;

      Serial.print(" - PEER ACTUAL");
    }

    if (peer->ball_carrier) { Serial.print(" - CARRIER INICIAL"); }
    Serial.println();
  };

  memcpy(mac_addr_M, mac_addr_M, sizeof(mac_addr_M));
  Serial.printf("- M0: %02x:%02x:%02x:%02x:%02x:%02x - MAESTRO",
                mac_addr_M[0], mac_addr_M[1], mac_addr_M[2],
                mac_addr_M[3], mac_addr_M[4], mac_addr_M[5]);

  Serial.println();
  delay(500);

  if (!peer_actual_registrado) {
    Serial.println("No encontró el peer actual en la lista de direcciones mac");
    error_pedir_reinicio();
    ///////////////////////
  }

  ////////////////////////////////////////
  //REGISTRAR INFORAMCION DE LOS AGENTES//
  ////////////////////////////////////////

  Serial.println();
  for (int i = 0; i < len_lista_agentes; i++) {

    Serial.printf("Agregando jugador %i...", i + 1);
    s_peer_info* peer = lista_agentes[i];
    
    memcpy(peerInfo_buffer.peer_addr, peer->mac_addr, 6);
    state = esp_now_add_peer(&peerInfo_buffer);

    if (state == ESP_OK) {
      Serial.println("OK");
    } else {
      Serial.printf("ERROR [%s]", esp_err_to_name(state));
      Serial.println();

      error_pedir_reinicio();
      ///////////////////////
    }
    
    //Agregar a lista en caso de ser varios agentes aliados
    if (peer->team[0] == peer_actual->team[0] && peer->id != peer_actual->id ) { peer_aliado = peer; }
    delay(100);
  };

  Serial.print("Agregando Broadcast...");
  memcpy(peerInfo_buffer.peer_addr, mac_broadcast, 6);
  state = esp_now_add_peer(&peerInfo_buffer);
  if (state == ESP_OK) {
    Serial.println("OK");
  } else {
    Serial.printf("ERROR [%s]", esp_err_to_name(state));
    Serial.println();
    error_pedir_reinicio();
    ///////////////////////
  } Serial.println();

  Serial.print("Agregando Maestro...");
  memcpy(peerInfo_buffer.peer_addr, mac_addr_M, 6);
  state = esp_now_add_peer(&peerInfo_buffer);  
  if (state == ESP_OK) {
    Serial.println("OK");
  } else {
    Serial.printf("ERROR [%s]", esp_err_to_name(state));
    Serial.println();
    error_pedir_reinicio();
    ///////////////////////
  } Serial.println();
  

  ///////////////////////////
  //ESPERAR PING DE MAESTRO//
  ///////////////////////////
  led_operando_b();

  esp_now_register_recv_cb(cb_list);
  esp_now_register_send_cb(PrintDataSent); //DEBUG

  Serial.println("Esperando ping de maestro");

  while (!match_info.ping_recibido) { delay(500); } //Callback envía mensaje
  delay(100);

  if (!match_info.ping_enviado) {
    Serial.println("No se pudo enviar mensaje");
    error_pedir_reinicio();
    ///////////////////////
  }

  while (!match_info.partido_activo) { delay(100); }

  Serial.println("Se inició el partido");
  Serial.println("serial_begin");

  led_idle_rgb();
}


void loop() {
  
  //LOOP PRINCIPAL DE PARTIDO
  while (match_info.partido_activo) 
  {
    //Revisar si peer actual es carrier para arrojar pelota
    if (match_info.pelota_atajada && peer_actual->ball_carrier) { 
      
      led_operando_b(); delay(500);

      f_hacer_pase(&pase, peer_actual);
      esp_now_send(mac_broadcast, (uint8_t*)&pase, sizeof(msg_jugada));
      peer_actual->ball_carrier = false;
      match_info.pelota_atajada = false;
      
      led_idle_rgb();
    }
    delay(100);
  }

  led_atrapa_g();
  esp_now_deinit();  
  while (true) { Serial.println("Partido terminado"); delay(1000); }
}