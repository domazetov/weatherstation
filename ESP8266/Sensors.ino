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
    client.publish("ESP1/DHT/humi", buffer);

    snprintf (buffer, MSG_BUFFER_SIZE, "%X", *(uint32_t*)&temperature);
    Serial.print("\tPublish temperature: ");
    Serial.print(buffer);
    client.publish("ESP1/DHT/temp", buffer);
}

void read_bmp()
{
    char buffer[MSG_BUFFER_SIZE];

    float temperature = bmp.readTemperature();
    float pressure = bmp.readPressure();
    // float altitude = bmp.readAltitude(1013.25);
    //todo: Remove Serial.prints below
    Serial.print("\nBMP280: ");
    Serial.print("Temperature: ");
    Serial.print(temperature, 1);
    Serial.print("C");
    Serial.print("\tPressure: ");
    Serial.print(pressure, 1);
    Serial.print("Pa");
    // Serial.print("\tApprox altitude = ");
    // Serial.print(altitude);
    // Serial.print("m");


    snprintf (buffer, MSG_BUFFER_SIZE, "%X", *(uint32_t*)&temperature);
    Serial.print("\nPublish message: ");
    Serial.print(buffer);
    client.publish("ESP1/BMP/temp", buffer);

    snprintf (buffer, MSG_BUFFER_SIZE, "%X", *(uint32_t*)&pressure);
    Serial.print("\tPublish message: ");
    Serial.print(buffer);
    client.publish("ESP1/BMP/pres", buffer);

    // snprintf (buffer, MSG_BUFFER_SIZE, "%X", *(uint32_t*)&altitude);
    // Serial.print("\tPublish message: ");
    // Serial.print(buffer);
    // client.publish("ESP1/BMP/alti", buffer);
}