#include <modulos.h>

//PING INICIAL
msg ping_rcv;
msg ping_snd = { .cb_id = 200 };

//COORDINACION DE POSICION INICIAL Y CARRIER
msg_coor coor_snd = { .cb_id = 202 };
msg fin_coor = { .cb_id = 203 };

//FIN DE PARTIDO
msg inicio_partido = { .cb_id = 0 };
msg final_partido = { .cb_id = 1 };

//JUGADAS
msg_jugada pase;
msg_jugada atajada;

//FINALES DE JUGADA
msg_rst expt_pierde = { .cb_id = 100 };
msg_rst expt_tiempo = { .cb_id = 101 };

void cb_list(const esp_now_recv_info *recv_info, const uint8_t *data, int len) {
  
  if (len < 1) { Serial.printf("Mensaje inválido de largo %d", len); Serial.println(); } 
  else {

    uint8_t cb_id = data[0];
    Serial.printf("(cb_list recibe %d)", cb_id); Serial.println(); //DEBUG
    
    switch (cb_id) {
      
      //PING DE RESPUESTA DE ESCLAVO
      case 201:
        //UNICAST
        if (len >= sizeof(msg)) {
          memcpy(&ping_rcv, data, sizeof(msg));
          if (strncmp(ping_rcv.msg, "respuesta", sizeof(ping_rcv.msg)) == 0) { match_info.ping_recibido = true; }
        } else { Serial.printf("Largo incorrecto. cb_id: %d", cb_id); imprimirData(data, len); Serial.println(); }
        break;

      //PASE
      case 10:
        //BROADCAST
        if (len >= sizeof(msg_jugada)) {
          memcpy(&pase, data, sizeof(msg_jugada));
          match_info.ultimo_equipo_carrier[0] = pase.peer_team[0];
          match_info.pelota_atajada = false;
          match_info.t_actual += 1;
          Serial.printf("{info:\"%c%d lanza a [%d, %d]\"},", pase.peer_team[0], pase.peer_id, pase.pos[0], pase.pos[1] );
          Serial.println();
        } else { Serial.printf("Largo incorrecto. cb_id: %d", cb_id); imprimirData(data, len); Serial.println(); }
        break;

      //ATAJADA
      case 11:
        //BROADCAST
        if (len >= sizeof(msg_jugada)) {
          memcpy(&atajada, data, sizeof(msg_jugada));
          match_info.pelota_atajada = true;
          match_info.t_actual += 1;
          Serial.printf("%c%d ataja en [%d, %d]", atajada.peer_team[0], atajada.peer_id, atajada.pos[0], atajada.pos[1] );
        } else { Serial.printf("Largo incorrecto. cb_id: %d", cb_id); imprimirData(data, len); Serial.println(); }
        break;

      //EXCEPCION POSICION INALCANZABLE
      case 60:
        //PUNTO//
        f_punto(&expt_pierde, match_info.ultimo_equipo_carrier[0], "posicion inalcanzable");
        esp_now_send(mac_broadcast, (uint8_t*)&expt_pierde, sizeof(expt_pierde));
        match_info.pelota_atajada = true;
        break;

      //EXCEPCION DESICION DISTINTA
      case 61:
        //PUNTO//
        f_punto(&expt_pierde, match_info.ultimo_equipo_carrier[0], "desicion distinta");
        esp_now_send(mac_broadcast, (uint8_t*)&expt_pierde, sizeof(expt_pierde));
        match_info.pelota_atajada = true;
        break;

      //EXCEPCION PROBLEMAS EN LA COORDINACION
      case 104:
        Serial.println("Error en la coordinacion - cb_id: 104");
        error_pedir_reinicio();
        ///////////////////////
        break;

      default:
        Serial.printf("ERROR cb_id %d no registrado", cb_id); Serial.println(); imprimirData(data, len);
        break;
    }
  }
}


void setup(){

  Serial.begin(115200);

  Serial.println();Serial.println("serial_begin");
  #if defined(CONFIG_IDF_TARGET_ESP32S3)
    Serial.println("Configuración para ESP32-S3");
    pixels.begin();
  #elif defined(CONFIG_IDF_TARGET_ESP32)
    Serial.println("Configuración para ESP32");
    pinMode(LED_BUILTIN, OUTPUT);
  #endif

  ///////////////////////////////////
  match_info.max_partidos = 1000;

  peer_A1.ball_carrier = false;
  peer_A1.pos[0] = 1;
  peer_A1.pos[1] = 1;

  peer_A2.ball_carrier = false;
  peer_A2.pos[0] = 4;
  peer_A2.pos[1] = 1;

  peer_B1.ball_carrier = true;
  peer_B1.pos[0] = 1;
  peer_B1.pos[1] = 1;

  peer_B2.ball_carrier = false;
  peer_B2.pos[0] = 4;
  peer_B2.pos[1] = 1;
  ///////////////////////////////////

  led_idle_rgb();

  Serial.println();
  Serial.println("Lista Jugadores:");
  
  int ball_carrier_check = 0;

  for (int i = 0; i<len_lista_agentes; i++){
    s_peer_info* peer = lista_agentes[i];
    
    Serial.printf("- %s%d: %02x:%02x:%02x:%02x:%02x:%02x", 
        peer->team, peer->id,
        peer->mac_addr[0], peer->mac_addr[1], peer->mac_addr[2],
        peer->mac_addr[3], peer->mac_addr[4], peer->mac_addr[5]);

    if (peer->ball_carrier){
      
      ball_carrier_check += 1;
      Serial.println(" - CARRIER INICIAL");
    
    } else {
      Serial.println("");
    }
  };

  if (ball_carrier_check == 0){
    Serial.println("No existe carrier que comience el partido");
    error_pedir_reinicio();
    ///////////////////////
  }
  if (ball_carrier_check != 1){
    Serial.println("Se asignó más de un carrier al comienzo del partido");
    error_pedir_reinicio();
    ///////////////////////
  }

  Serial.printf("- M0: %02x:%02x:%02x:%02x:%02x:%02x - MAESTRO - PEER ACTUAL", 
      mac_addr_M[0], mac_addr_M[1], mac_addr_M[2],
      mac_addr_M[3], mac_addr_M[4], mac_addr_M[5]);
  
  Serial.println(); Serial.println();
  delay(500);
  
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error al inicializar ESP-NOW");
    error_pedir_reinicio();
    ///////////////////////
  };

  ////////////////////////////////////////
  //REGISTRAR INFORAMCION DE LOS AGENTES//
  ////////////////////////////////////////
  esp_now_register_recv_cb(cb_list);
  esp_now_register_send_cb(PrintDataSent); //DEBUG

  for (int i = 0; i<len_lista_agentes; i++){
      Serial.printf("Agregando jugador %i...", i+1);
      
      s_peer_info* peer = lista_agentes[i];
      memcpy(peerInfo_buffer.peer_addr, peer->mac_addr, 6 );  
      
      state = esp_now_add_peer(&peerInfo_buffer);
      
      if (state == ESP_OK) { Serial.println("OK");} 
      else {
        Serial.printf("ERROR [%s]",esp_err_to_name(state));
        Serial.println();

        error_pedir_reinicio();
        ///////////////////////
      }
      
      delay(100);
      
  };
  Serial.print("Agregando Broadcast...");
  memcpy(peerInfo_buffer.peer_addr, mac_broadcast, sizeof(mac_broadcast) );  
  
  state = esp_now_add_peer(&peerInfo_buffer);
  if (state == ESP_OK) { Serial.println("OK");} 
  else {
    Serial.printf("ERROR [%s]",esp_err_to_name(state));
    Serial.println();

    error_pedir_reinicio();
    ///////////////////////
  } Serial.println(); delay(100);


  ////////////////////////////////////
  //PING POR ESP-NOW PARA VER ESTADO//
  ////////////////////////////////////

  for (int i = 0; i<len_lista_agentes; i++){
    s_peer_info* peer = lista_agentes[i];
    Serial.printf("Estado jugador %s%d...", peer->team, peer->id);
    
    strcpy(ping_snd.msg, "estado");
    state = esp_now_send(peer->mac_addr, (uint8_t*)&ping_snd, sizeof(ping_snd));

    if (state == ESP_ERR_ESPNOW_NOT_FOUND) {
        match_info.agentes_listos = false;
        Serial.println("Desconectado");
        delay(1000);
        continue;
    } 
    
    if (state != OK){
        Serial.printf("ERROR al enviar [%s]",esp_err_to_name(state));
        Serial.println();
    }
    //Esperar respuesta por 3 segundos
    unsigned long startTime = millis();
    while (!match_info.ping_recibido && millis() - startTime < 1500) { delay(100); }
    if (match_info.ping_recibido) {
        Serial.println("OK");
        match_info.ping_recibido = false;
    } else {
        Serial.println("Sin Respuesta");
        match_info.agentes_listos = false;
    }
    delay(1000);
  }; Serial.println(); delay(100);

  if (!match_info.agentes_listos) {
    Serial.println("No todos los agentes están en linea");
    error_pedir_reinicio();
    ///////////////////////
  } 


  ////////////////////////////////////
  //COMPARTIR POSICIÓN POR BROADCAST//
  ////////////////////////////////////

  for (int i = 0; i<len_lista_agentes; i++){
    s_peer_info* peer = lista_agentes[i]; 

    coor_snd.team[0] = peer->team[0];
    coor_snd.id = peer->id;
    coor_snd.carrier = peer->ball_carrier;
    
    coor_snd.pos[0] = peer->pos[0];
    coor_snd.pos[1] = peer->pos[1];

    esp_now_send(mac_broadcast, (uint8_t*)&coor_snd, sizeof(coor_snd));

    Serial.printf("Información de %s%d enviada", peer->team, peer->id); Serial.println();
    delay(500);
  }

  strcpy(fin_coor.msg, "completado");
  esp_now_send(mac_broadcast, (uint8_t*)&fin_coor, sizeof(fin_coor));

  //ESPERAR CHECKEO DE ASIGNACIONES PARA LOS PEER
  delay(500);

  if (!match_info.agentes_listos) {
    Serial.println("No todos los agentes están preparados");
    error_pedir_reinicio(); 
    ///////////////////////
  } 

  ///////////////////////
  // INICIO DE PARTIDO //
  ///////////////////////

  Serial.println();
  led_conteo();

  //INICIO DE PARTIDO
  esp_now_send(mac_broadcast, (uint8_t*)&inicio_partido, sizeof(inicio_partido));
  match_info.partido_activo = true;
  
  Serial.println("serial_begin");
  led_operando_b();
}

void loop(){
  
  while(match_info.partido_activo){

    if (!match_info.pelota_atajada) {
      unsigned long startTime = millis();
      while (!match_info.pelota_atajada && millis() - startTime < 3000) { delay(100); }
      if(!match_info.pelota_atajada) { 
        //PUNTO//
        f_punto(&expt_tiempo, match_info.ultimo_equipo_carrier[0], "timeout");
        esp_now_send(mac_broadcast, (uint8_t*)&expt_tiempo, sizeof(expt_tiempo));
        match_info.pelota_atajada = true;
      }
    }
    delay(1000);
  
  }
  
  led_atrapa_g();
  Serial.println("Partido Terminado");

  esp_now_deinit();
  while (true) {
      Serial.println("RESULTADOS:");
      Serial.printf("Puntos_A:%d - Puntos_B:%d - Tiempo:%d - Max_Jugadas:%d - Max_Partidos:%d\n",match_info.pts_A, match_info.pts_B, match_info.t_actual, match_info.max_jugadas, match_info.max_partidos );
      Serial.println();
      delay(1000);
  }
}