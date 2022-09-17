void read_data(){
	int retry_count = 0;
	char data_buffer[MSG_BUFFER_SIZE];
	float dhth = dht.getHumidity();

	while(isnan(dhth)){
		if(retry_count == 5){
			LOG_SERIAL.println("DHT retry timeout");
			break;
		}
		retry_count++;
		LOG_SERIAL.printf("\nNaN: %u", retry_count);
		delay(3000);
		dhth = dht.getHumidity();
	}

	float dhtt = dht.getTemperature();
	float bmpt = bmp.readTemperature();
	float bmpp = bmp.readPressure()/100;

	LOG_SERIAL.printf("\nDHT:\tHumidity: %.0f%%\t\tTemperature: %.2f°C", dhth, dhtt);
	LOG_SERIAL.printf("\nBMP280:\tPressure: %.2fhPa\tTemperature: %0.2f°C", bmpp, bmpt);

	average_temp = (uint16_t)((dhtt+bmpt)*50);

	snprintf(data_buffer, 33, "%08X%08X%08X%08X",	*(uint32_t*)&dhth, *(uint32_t*)&dhtt,
													*(uint32_t*)&bmpt, *(uint32_t*)&bmpp);
	LOG_SERIAL.printf("\nMessage payload: %s\n", data_buffer);

	client.publish(publish_topic, data_buffer);
	delay(PUBLISH_SLEEP); //Delay needed for MQTT Broker to capture sent message
}
