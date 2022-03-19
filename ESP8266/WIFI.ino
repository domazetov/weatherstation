void connect_to_wifi()
{
	LOG_SERIAL.println("Connecting as WiFi client.");
	WiFi.disconnect();
	WiFi.begin(ssid, password);
	int connRes = WiFi.waitForConnectResult();
	LOG_SERIAL.print("Connection status: ");
	LOG_SERIAL.println(connRes);
}

boolean reconnect_MQTT()
{
	LOG_SERIAL.print("Attempting MQTT connection...");

	String clientId = String(APSSID);

	if (client.connect(clientId.c_str()))
	{
		digitalWrite(BUILTIN_LED, HIGH);
		LOG_SERIAL.println("connected");
	}
	else
	{
		digitalWrite(BUILTIN_LED, LOW);
		LOG_SERIAL.println(" failed, retry in 5 seconds");
	}
	return client.connected();
}

boolean isIp(String str)
{
	for (size_t i = 0; i < str.length(); i++)
	{
		int c = str.charAt(i);
		if (c != '.' && (c < '0' || c > '9'))
		{
			return false;
		}
	}
	return true;
}

String toStringIp(IPAddress ip)
{
	String res = "";
	for (int i = 0; i < 3; i++)
	{
		res += String((ip >> (8 * i)) & 0xFF) + ".";
	}
	res += String(((ip >> 8 * 3)) & 0xFF);
	return res;
}
