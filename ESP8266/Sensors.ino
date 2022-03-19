void read_data()
{
	int retry_count = 0;
	char data_buffer[MSG_BUFFER_SIZE];
	float dhth = dht.getHumidity();

	while(isnan(dhth))
	{
		if(retry_count == 5)
		{
			LOG_SERIAL.println("DHT retry timeout");
			break;
		}
		retry_count++;
		LOG_SERIAL.println("NaN: ");
		LOG_SERIAL.print(retry_count);
		delay(3000);
		dhth = dht.getHumidity();
	}

	float dhtt = dht.getTemperature();
	float bmpt = bmp.readTemperature();
	float bmpp = bmp.readPressure()/100;

	LOG_SERIAL.print("\nDHT: ");
	LOG_SERIAL.print("\tHumidity: ");
	LOG_SERIAL.print(dhth, 2);
	LOG_SERIAL.print("%");
	LOG_SERIAL.print("\tTemperature: ");
	LOG_SERIAL.print(dhtt, 2);
	LOG_SERIAL.print("C");
	LOG_SERIAL.print("\nBMP280: ");
	LOG_SERIAL.print("Temperature: ");
	LOG_SERIAL.print(bmpt, 2);
	LOG_SERIAL.print("C");
	LOG_SERIAL.print("\tPressure: ");
	LOG_SERIAL.print(bmpp, 2);
	LOG_SERIAL.print("hPa\n");

	snprintf(data_buffer, 33, "%08X%08X%08X%08X", *(uint32_t*)&dhth, *(uint32_t*)&dhtt, *(uint32_t*)&bmpt, *(uint32_t*)&bmpp);
	LOG_SERIAL.print("Message payload: ");
	LOG_SERIAL.println(data_buffer);

	client.publish(wstation_topic, data_buffer);
	delay(PUBLISH_SLEEP); //Delay needed for MQTT Broker to capture sent message
}