void blink(){
  digitalWrite(INTERNAL_LED, HIGH);
  delay(100);
  digitalWrite(INTERNAL_LED, LOW);
  delay(100);
  digitalWrite(INTERNAL_LED, HIGH);
  delay(100);
  digitalWrite(INTERNAL_LED, LOW);
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status){
  //Serial.print("\r\nLast Packet Send Status:\t");

  if (status = ESP_NOW_SEND_SUCCESS) {
    //Serial.println("Deliver Success");
    flag_resend = 0;
  
  } else {
    //Serial.println ("Delivery Fail");
    flag_resend = 1;
  
  }
}

void OnDataRecv(const esp_now_recv_info_t *esp_now_info, const uint8_t *incomingData, int len){
  t_1 = micros();
  RTT = t_1 - t_0;
  memcpy(&esp_buffer_rx, incomingData, sizeof(esp_buffer_rx));
  Serial.printf("%d Bytes Received\n", len); 
  Serial.printf ("RTT = %Au [us], Estimated Delay = %u [us]\n", RTT, RTT/2);
  Serial.printf("%u\n", RTT/2);
  flag_recv = 1;
}