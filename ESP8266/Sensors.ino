void read_data()
{
  int retry_count = 0;
  char data_buffer[MSG_BUFFER_SIZE];
  float dhth = dht.getHumidity();

  while(isnan(dhth))
  {
    if(retry_count == 5)
    {
      Serial.println("DHT retry timeout");
      break;
    }
    retry_count++;
    Serial.println("NaN: ");
    Serial.print(retry_count);
    delay(3000);
    dhth = dht.getHumidity();
  }

  float dhtt = dht.getTemperature();
  float bmpt = bmp.readTemperature();
  float bmpp = bmp.readPressure()/100;

  Serial.print("\nDHT: ");
  Serial.print("\tHumidity: ");
  Serial.print(dhth, 2);
  Serial.print("%");
  Serial.print("\tTemperature: ");
  Serial.print(dhtt, 2);
  Serial.print("C");
  Serial.print("\nBMP280: ");
  Serial.print("Temperature: ");
  Serial.print(bmpt, 2);
  Serial.print("C");
  Serial.print("\tPressure: ");
  Serial.print(bmpp, 2);
  Serial.print("hPa\n");

  snprintf(data_buffer, 33, "%08X%08X%08X%08X", *(uint32_t*)&dhth, *(uint32_t*)&dhtt, *(uint32_t*)&bmpt, *(uint32_t*)&bmpp);
 
  Serial.println(*(uint32_t*)&dhth, HEX);
  Serial.println(*(uint32_t*)&dhtt, HEX);
  Serial.println(*(uint32_t*)&bmpt, HEX);
  Serial.println(*(uint32_t*)&bmpp, HEX);
  Serial.println(data_buffer);

  client.publish(wstation_topic, data_buffer);
}