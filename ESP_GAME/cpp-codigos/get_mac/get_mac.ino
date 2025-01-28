#include <esp_wifi.h>
#include <WiFi.h>
#include <esp_now.h>

#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel pixels(1, 48, NEO_GRB + NEO_KHZ800);

int val = 1;
int *p = &val;

uint8_t* get_mac(){
  static uint8_t baseMac[6];
  static uint8_t nullmac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    return baseMac;
  } else {
    return nullmac;
  }
};

void setup() {
  Serial.begin(115200);
  pixels.begin();

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error al inicializar ESP-NOW");
  } else {
    uint8_t* mac = get_mac();
    Serial.printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x",
                  mac[0], mac[1], mac[2],
                  mac[3], mac[4], mac[5]);
    Serial.println();
  }
}

void loop() {

  pixels.setPixelColor(0, pixels.Color(100, 100, 0)); pixels.show();
  delay(1000);

  pixels.setPixelColor(0, pixels.Color(0, 100, 0)); pixels.show();
  Serial.println("Anunciando Inicio");
  
  Serial.printf("valor: %i - puntero: %p",val,(void *)p );
  Serial.println();
  delay(1000);
}
