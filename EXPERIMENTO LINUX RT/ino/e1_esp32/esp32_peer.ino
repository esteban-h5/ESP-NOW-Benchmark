void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  if (PeerActual == 1) 
  {
      if (status = ESP_NOW_SEND_SUCCESS) {
        flag_resend = 0;
      } else {
        flag_resend = 1; //No entregado, se debe reenviar
      }
  } 

  else if (PeerActual == 2) 
  {
      if (status = ESP_NOW_SEND_SUCCESS) {
        Serial.println("Delivery Success");
      } else {
        Serial.print("Delivery Fail: ");
        Serial.println(status);
      }
  }
  delay(100);
}


void OnDataRecv(const esp_now_recv_info_t *esp_now_info, const uint8_t *incomingData, int len)
{
  if (PeerActual == 1) 
  {
    t_1 = esp_timer_get_time();
    RTT = t_1 - t_0;

    memcpy(&esp_buffer_rx, incomingData, sizeof(esp_buffer_rx));

    Serial.print(">RTT:");
    Serial.println(RTT);

    flag_recv = true; //Recibido, se puede reenviar
    delay(250);
  } 

  else if (PeerActual == 2) 
  {
    memcpy(&esp_buffer_rx, incomingData, sizeof(esp_buffer_rx));
    esp_now_send(mac_addr_E1, (uint8_t*)&esp_buffer_rx, sizeof(esp_buffer_rx));

    Serial.print("Re-sended ");
    Serial.print(sizeof(esp_buffer_rx));
    Serial.println(" bytes of data");

    // Serial.println("Texto recibido:");
    // Serial.println(esp_buffer_rx.texto);
  }

}

