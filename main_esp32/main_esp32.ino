#include <esp_now.h>
#include <WiFi.h>

#define INTERNAL_LED 2

//Registrar Peer Actual
esp_now_peer_info_t peerInfo;

typedef struct struct_message {
    int numero;
    float flotante;
    char letra;
} struct_message;

struct_message esp_buffer_tx;
struct_message esp_buffer_rx;

//uint8_t mac_broadcast[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}

uint8_t mac_addr_E1[] = {0xB8, 0xD6, 0x1A, 0xAB, 0x4D, 0x8C};
uint8_t mac_addr_E2[] = {0xB8, 0xD6, 0x1A, 0xAC, 0x03, 0x60};

bool flag_recv = 1;
bool flag_resend = 0;

uint32_t t_0, t_1, RTT;

void setup(){
  Serial.begin(115200);

  pinMode(INTERNAL_LED, OUTPUT);
  digitalWrite(INTERNAL_LED, LOW);

  WiFi.mode(WIFI_STA); //Modo WiFi Station

  if (esp_now_init() != ESP_OK){
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Registrar callback() al enviar y recibir datos
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);


  memcpy(peerInfo.peer_addr, mac_addr_E1, 6);
  
  peerInfo.channel = 1;
  peerInfo.encrypt = false;

  if(esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}


void loop(){
  blink();
  if( flag_recv || flag_resend ){ 
    t_0 = micros(); //idem 
    esp_now_send(NULL, (uint8_t*)&esp_buffer_tx, sizeof(esp_buffer_tx));
    flag_recv = 0;
  }
  delay(1000)
}
