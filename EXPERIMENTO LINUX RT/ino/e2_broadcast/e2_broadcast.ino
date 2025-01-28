#include <esp_now.h>
#include <WiFi.h>

//#define INTERNAL_LED 2

uint8_t mac_broadcast[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

// uint8_t mac_addr[] = {0x7C, 0xDF, 0xA1, 0xFF, 0x2B, 0x30}; //ESP32-S3 E1
// uint8_t mac_addr[] = {0x68, 0xB6, 0xB3, 0x21, 0x09, 0xE8}; //ESP32-S3 E2

// uint8_t mac_addr[] = {0xB8, 0xD6, 0x1A, 0xAB, 0x4D, 0x8C}; //ESP32 E1
// uint8_t mac_addr[] = {0xB8, 0xD6, 0x1A, 0xAC, 0x03, 0x60}; //ESP32 E2

esp_now_peer_info_t broadcast_slave;

#define CHANNEL 1
#define PRINTSCANRESULTS 0
#define DELETEBEFOREPAIR 0

typedef struct struct_message {
  char texto[250];
} struct_message;

struct_message esp_buffer_tx;
struct_message esp_buffer_rx;

int64_t t_0, t_1, RTT;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  if (status != ESP_NOW_SEND_SUCCESS) {
    Serial.println(status);
  }
  delay(100);
}

void OnDataRecv(const esp_now_recv_info_t *esp_now_info, const uint8_t *incomingData, int len)
{
  memcpy(&esp_buffer_rx, incomingData, sizeof(esp_buffer_rx));

  Serial.print("Re-sended ");
  Serial.print(sizeof(esp_buffer_rx));
  Serial.println(" bytes of data");
}

void setup()
{

  Serial.begin(115200);

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
  memcpy(broadcast_slave.peer_addr, mac_broadcast, sizeof(mac_broadcast));
  broadcast_slave.channel = CHANNEL;
  broadcast_slave.encrypt = false; // Configurar cifrado según tus necesidades
  
  if(esp_now_add_peer(&broadcast_slave) != ESP_OK)
  {
    Serial.println("Failed to slave peer");
    return;
  }

  String buffer;
  
  for (int i=0; i < sizeof(esp_buffer_tx.texto)-1; i++) {
    buffer += char(random(91)+31);
  }
  buffer.toCharArray(esp_buffer_tx.texto, sizeof(esp_buffer_tx.texto));
}

void loop()
{ 
  Serial.print(micros());
  Serial.print(" - Sending: ");
  Serial.print(sizeof(esp_buffer_tx));
  Serial.println(" bytes");
  
  esp_now_send(mac_broadcast, (uint8_t*)&esp_buffer_tx, sizeof(esp_buffer_tx));  
  delay(1000);
}