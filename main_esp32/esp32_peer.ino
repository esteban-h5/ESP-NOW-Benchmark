void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  if (PeerActual == 1) {

      if (status = ESP_NOW_SEND_SUCCESS) {
        flag_resend = 0;
      } else {
        Serial.println("Delivery Fail");
        flag_resend = 1; //No entregado, se debe reenviar
      }
  } 

  else if (PeerActual == 2) 
  {
      if (status = ESP_NOW_SEND_SUCCESS) {
        Serial.println("Delivery Success");
      } else {
        Serial.println("Delivery Fail");
      }
  }
}


void OnDataRecv(const esp_now_recv_info_t *esp_now_info, const uint8_t *incomingData, int len)
{
  if (PeerActual == 1) {
    t_1 = esp_timer_get_time();;
    RTT = t_1 - t_0;

    memcpy(&esp_buffer_rx, incomingData, sizeof(esp_buffer_rx));

    Serial.print("$RTT:");
    Serial.print(RTT);
    Serial.println("}");

    flag_recv = true; //Recibido, se puede reenviar
    delay(2000);
  } 

  else if (PeerActual == 2) 
  {
    memcpy(&esp_buffer_tx, incomingData, sizeof(esp_buffer_tx));
    esp_now_send(mac_addr_E1, (uint8_t*)&esp_buffer_tx, sizeof(esp_buffer_tx));
  }

}

