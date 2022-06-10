void load_from_eeprom()
{
	EEPROM.begin(512);
	EEPROM.get(0, ssid);
	EEPROM.get(0 + sizeof(ssid), password);
	EEPROM.get(0 + sizeof(ssid) + sizeof(password), mqtt_server);
	EEPROM.get(0 + sizeof(ssid) + sizeof(password) + sizeof(mqtt_server), ac_on_signal);
	EEPROM.get(0 + sizeof(ssid) + sizeof(password) + sizeof(mqtt_server) + sizeof(ac_on_signal), ac_off_signal);
	char ok[2 + 1];
	EEPROM.get(0 + sizeof(ssid) + sizeof(password) + sizeof(mqtt_server) + sizeof(ac_on_signal) + sizeof(ac_off_signal), ok);
	EEPROM.end();
	if (String(ok) != String("OK"))
	{
		ssid[0] = 0;
		password[0] = 0;
	}
	LOG_SERIAL.println("Recovered credentials:");
	LOG_SERIAL.println(ssid);
	LOG_SERIAL.println(strlen(password) > 0 ? "********" : "no password");
}

void save_to_eeprom()
{
	EEPROM.begin(512);
	EEPROM.put(0, ssid);
	EEPROM.put(0 + sizeof(ssid), password);
	EEPROM.put(0 + sizeof(ssid) + sizeof(password), mqtt_server);
	EEPROM.put(0 + sizeof(ssid) + sizeof(password) + sizeof(mqtt_server), ac_on_signal);
	EEPROM.put(0 + sizeof(ssid) + sizeof(password) + sizeof(mqtt_server) + sizeof(ac_on_signal), ac_off_signal);
	char ok[2 + 1] = "OK";
	EEPROM.put(0 + sizeof(ssid) + sizeof(password) + sizeof(mqtt_server) + sizeof(ac_on_signal) + sizeof(ac_off_signal), ok);

	EEPROM.commit();
	EEPROM.end();

	LOG_SERIAL.println("Credentials saved!");
}
