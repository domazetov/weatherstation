void connect_to_wifi(){
	LOG_SERIAL.println("Connecting as WiFi client.");
	WiFi.disconnect();
	WiFi.begin(ssid, password);
	LOG_SERIAL.printf("Connection status: %u", WiFi.waitForConnectResult());
}

boolean reconnect_mqtt(){
	LOG_SERIAL.print("Attempting MQTT connection...");

	String clientId = String(APSSID);

	if (client.connect(clientId.c_str())){
		digitalWrite(LED_PIN, HIGH);
		LOG_SERIAL.println("connected");
    	client.subscribe(subscribe_topic);
	}else{
		digitalWrite(LED_PIN, LOW);
		LOG_SERIAL.println(" failed, retry in 5 seconds");
	}
	return client.connected();
}

uint8_t byte2int(byte* data){
	char value[2];
	value[0]=(char)data[0];
	value[1]=(char)data[1];

	return strtol(value, NULL, 16);
}

void mqtt_callback(char* topic, byte* payload, unsigned int length){
	uint8_t changed_sleep_time = 0xFF;
	uint16_t target_temp = 0xFF;

	LOG_SERIAL.printf("Message arrived [%s]: ", topic);
	for (int i = 0; i < length; i++){
		LOG_SERIAL.print((char)payload[i]);
	}

#if ENABLE_IR
	target_temp = ((uint16_t)byte2int(payload))*100;
	if(AC_OFF != target_temp){
		LOG_SERIAL.printf("\nTemperature, Average : %u, Target : %u ", average_temp, target_temp);
		if(average_temp > target_temp){
			LOG_SERIAL.println("AC ON");
			send_ir_signal(&ac_on_signal);
		}else{
			LOG_SERIAL.println("AC OFF");
			send_ir_signal(&ac_off_signal);
		}
	}
#endif
	changed_sleep_time = byte2int(payload+2);
	if(DEEPSLEEP_OFF != changed_sleep_time){
		LOG_SERIAL.printf("\nDeepSleep for %u minutes", changed_sleep_time);
		ESP.deepSleep(changed_sleep_time*60e6);
	}else{
		LOG_SERIAL.println("\nDefault sleep 10 seconds");
		delay(DEFAULT_SLEEP);
	}
	send_data = true;
}

boolean isIp(String str){
	for (size_t i = 0; i < str.length(); i++){
		int c = str.charAt(i);
		if (c != '.' && (c < '0' || c > '9')){
			return false;
		}
	}
	return true;
}

String toStringIp(IPAddress ip){
	String res = "";
	for (int i = 0; i < 3; i++){
		res += String((ip >> (8 * i)) & 0xFF) + ".";
	}
	res += String(((ip >> 8 * 3)) & 0xFF);
	return res;
}
