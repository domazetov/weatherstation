void read_dht()
{
    char buffer[MSG_BUFFER_SIZE];

    delay(dht.getMinimumSamplingPeriod());

    float humidity = dht.getHumidity();
    float temperature = dht.getTemperature();
    //todo: Remove Serial.prints below
    Serial.print("\nDHT: ");
    Serial.print(dht.getStatusString());
    Serial.print("\tHumidity: ");
    Serial.print(humidity, 1);
    Serial.print("%");
    Serial.print("\t\tTemperature: ");
    Serial.print(temperature, 1);
    Serial.print("C");

    snprintf (buffer, MSG_BUFFER_SIZE, "%X", *(uint32_t*)&humidity);
    Serial.print("\nPublish humidity: ");
    Serial.print(buffer);
    client.publish(topic_dht_h, buffer);

    snprintf (buffer, MSG_BUFFER_SIZE, "%X", *(uint32_t*)&temperature);
    Serial.print("\tPublish temperature: ");
    Serial.print(buffer);
    client.publish(topic_dht_t, buffer);
}

void read_bmp()
{
    char buffer[MSG_BUFFER_SIZE];

    float temperature = bmp.readTemperature();
    float pressure = bmp.readPressure()/100;

    Serial.print("\nBMP280: ");
    Serial.print("Temperature: ");
    Serial.print(temperature, 1);
    Serial.print("C");
    Serial.print("\tPressure: ");
    Serial.print(pressure, 1);
    Serial.print("hPa");

    snprintf (buffer, MSG_BUFFER_SIZE, "%X", *(uint32_t*)&temperature);
    Serial.print("\nPublish message: ");
    Serial.print(buffer);
    client.publish(topic_bmp_t, buffer);

    snprintf (buffer, MSG_BUFFER_SIZE, "%X", *(uint32_t*)&pressure);
    Serial.print("\tPublish message: ");
    Serial.print(buffer);
    client.publish(topic_bmp_p, buffer);
}