#include <esp_now.h>
#include <WiFi.h>

//#define INTERNAL_LED 2

//////////////////////////////////////////////////
//Cambiar en función del Peer que sube el código//
//////////////////////////////////////////////////
// uint8_t PeerActual = 1; // Peer E1
// uint8_t PeerActual = 2; // Peer E2
//////////////////////////////////////////////////

//uint8_t mac_broadcast[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}

//E1  : B8:D6:1A:AB:4D:8C
//E2  : B8:D6:1A:AC:03:60

uint8_t mac_addr_E1[] = {0xB8, 0xD6, 0x1A, 0xAB, 0x4D, 0x8C};
uint8_t mac_addr_E2[] = {0xB8, 0xD6, 0x1A, 0xAC, 0x03, 0x60};

esp_now_peer_info_t peerInfo_E1;
esp_now_peer_info_t peerInfo_E2;

String buffer;

typedef struct struct_message {
  char texto[250];
} struct_message;

struct_message esp_buffer_tx;
struct_message esp_buffer_rx;


bool flag_recv = 0;
bool flag_resend = 1;

//uint32_t t_0, t_1, RTT;
int64_t t_0, t_1, RTT;

void setup()
{
  Serial.begin(115200);

  // pinMode(INTERNAL_LED, OUTPUT);
  // digitalWrite(INTERNAL_LED, LOW);

  WiFi.mode(WIFI_STA); //Modo WiFi Station

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Registrar callback() al enviar y recibir datos
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Configurar la estructura peerInfo_E1 con la información de E1
  memcpy(peerInfo_E1.peer_addr, mac_addr_E1, sizeof(mac_addr_E1));
  peerInfo_E1.channel = 1;
  peerInfo_E1.encrypt = false; // Configurar cifrado según tus necesidades
  
  // Configurar la estructura peerInfo_E2 con la información de E2
  memcpy(peerInfo_E2.peer_addr, mac_addr_E2, sizeof(mac_addr_E2));
  peerInfo_E2.channel = 1;
  peerInfo_E2.encrypt = false;

  if(esp_now_add_peer(&peerInfo_E1) != ESP_OK)
  {
    Serial.println("Failed to add peer E1");
    return;
  }
  
  if(esp_now_add_peer(&peerInfo_E2) != ESP_OK)
  {
    Serial.println("Failed to add peer E2");
    return;
  }


  
  if (PeerActual == 1) 
  {  
      for (int i=0; i < 249 ;i++) {
        buffer += char(random(91)+31);
      }
      buffer.toCharArray(esp_buffer_tx.texto, sizeof(esp_buffer_tx.texto));
  }
  Serial.print("PEER ACTUAL -> E");
  Serial.println(PeerActual);
  Serial.println("Texto a enviar:");
  Serial.println(esp_buffer_tx.texto);
  Serial.print("Tamaño -> ");
  Serial.println(sizeof(esp_buffer_tx));
}

void loop()
{ 
  if (PeerActual == 1) 
  {  
      // for (int i=0; i < sizeof(esp_buffer_tx.numero_aleatorio);i++) {
      //   cadena += char(random(256);
      // }

      if( flag_recv || flag_resend )
      { 
        t_0 = esp_timer_get_time(); //idem 
        esp_now_send(mac_addr_E2, (uint8_t*)&esp_buffer_tx, sizeof(esp_buffer_tx));
        
        flag_recv = 0;
      } 
  } 

  else if (PeerActual == 2) 
  {
    delay(50);
  }
}

