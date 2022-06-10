
boolean captivePortal()
{
	if (!isIp(server.hostHeader()) && server.hostHeader() != (String(myhostname) + ".local"))
	{
		LOG_SERIAL.println("Request redirected to captive portal");
		server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
		server.send(302, "text/plain", "");
		server.client().stop();
		return true;
	}
	return false;
}

void handleWifi()
{
	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");
	server.sendHeader("Expires", "-1");

	String Page;
	Page += "<!DOCTYPE html>";
	Page += "<html lang='en'>";
	Page += "<head>";
	Page += "<meta name='viewport' content='width=device-width'/>";
	Page += "<title>Setup</title>";
	Page += "<style> *, ::after, ::before { box-sizing: border-box; }";
	Page += "body {";
	Page += "margin: 0;";
	Page += "font-family: 'Segoe UI', Roboto, 'Helvetica Neue', Arial, 'Noto Sans', 'Liberation Sans';";
	Page += "font-size: 1rem;";
	Page += "font-weight: 400;";
	Page += "line-height: 1.5;";
	Page += "color: #212529;";
	Page += "background-color: #f5f5f5;}";
	Page += ".form-control {";
	Page += "display: block;";
	Page += "width: 100%;";
	Page += "height: calc(1.5em + 0.75rem + 2px);";
	Page += "border: 1px solid #ced4da;}";
	Page += "button {";
	Page += "cursor: pointer;";
	Page += "border: 1px solid transparent;";
	Page += "color: #fff;";
	Page += "background-color: #ff7200;";
	Page += "border-color: #ff7200;";
	Page += "padding: 0.5rem 1rem;";
	Page += "font-size: 1.25rem;";
	Page += "line-height: 1.5;";
	Page += "border-radius: 0.3rem;";
	Page += "width: 100%;}";
	Page += ".form-signin {";
	Page += "width: 100%;";
	Page += "max-width: 400px;";
	Page += "padding: 15px;";
	Page += "margin: auto;}";
	Page += "h1 { text-align: center; }";
	Page += "</style>";
	Page += "</head>";
	Page += "<body>";
	Page += "<main class='form-signin'>";
	Page += "<form action='wifisave' method='post'>";
	Page += "<h1>WiFi Setup</h1>";
	Page += "<br />";
	Page += "<div class='form-floating'>";
	Page += "<label><b>SSID:</b></label>";
	Page += "<input type='text' class='form-control' placeholder='network' name='n'/>";
	Page += "</div>";
	Page += "<div class='form-floating'>";
	Page += "<br /><label><b>Password:</b></label>";
	Page += "<input type='password' class='form-control' placeholder='password' name='p'/>";
	Page += "</div>";
	Page += "<div class='form-floating'>";
	Page += "<br /><label><b>Broker:</b></label>";
	Page += "<input type='ip-address' class='form-control' placeholder='ip-address'name='b'/>";
	Page += "</div>";
	Page += "<br /><button type='submit'>Save</button>";
	Page += "</form>";
	Page += "<br />";
	Page += "<form method='get' action='acsave'>";
	Page += "<button type='submit'>Setup AC</button>";
	Page += "</form>";

	if (server.client().localIP() == apip)
	{
		Page += String(F("<p>You are connected through the AP: ")) + softap_ssid + F("</p>");
	}
	else
	{
		Page += String(F("<p>You are connected through the WiFi network: ")) + ssid + F("</p>");
	}

	Page += "<table><tr><th align='left'>Current configuration:</th></tr>";
	Page += String(F("<p>AC Protocol: ")) + typeToString(ac_on_signal.decode_type).c_str() + F("</p>");
	Page += F("<tr><td>SSID ") + String(ssid) + F("</td></tr><tr><td>IP ") + toStringIp(WiFi.localIP()) + F("</td></tr>");
	Page += F("<tr><td>Broker IP ") + String(mqtt_server) + F("</td></tr></table>");

	Page += "<table><tr><th align='left'>WiFi list:</th></tr>";
	LOG_SERIAL.println("scan start");
	int n = WiFi.scanNetworks();
	LOG_SERIAL.println("scan done");

	if (n > 0)
	{
		for (int i = 0; i < n; i++)
		{
			Page += String(F("\r\n<tr><td>")) + WiFi.SSID(i) + F("</td></tr>");
		}
	}
	else
	{
		Page += "<tr><td>No WiFi found</td></tr>";
	}
	Page += "</body></main></html>";
	server.send(200, "text/html", Page);
	server.client().stop();
}

void handleWifiSave()
{
	LOG_SERIAL.println("wifi save");
	server.arg("n").toCharArray(ssid, sizeof(ssid) - 1);
	server.arg("p").toCharArray(password, sizeof(password) - 1);
	server.arg("b").toCharArray(mqtt_server, sizeof(mqtt_server) - 1);
	server.sendHeader("Location", "wifi", true);
	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");
	server.sendHeader("Expires", "-1");
	server.send(302, "text/plain", "");
	server.client().stop();
	save_to_eeprom();
	connect = strlen(ssid) > 0;
}

void handleNotFound()
{
	if (captivePortal())
	{
		return;
	}
	String message = F("File Not Found\n\n");
	message += F("URI: ");
	message += server.uri();
	message += F("\nMethod: ");
	message += (server.method() == HTTP_GET) ? "GET" : "POST";
	message += F("\nArguments: ");
	message += server.args();
	message += F("\n");

	for (uint8_t i = 0; i < server.args(); i++)
	{
		message += String(F(" ")) + server.argName(i) + F(": ") + server.arg(i) + F("\n");
	}
	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");
	server.sendHeader("Expires", "-1");
	server.send(404, "text/plain", message);
}

void handleACSave()
{
	LOG_SERIAL.println("AC SAVE");
	static uint8_t state = 0;

	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");
	server.sendHeader("Expires", "-1");

	String Page;
	Page += "<!DOCTYPE html>";
	Page += "<html lang='en'>";
	Page += "<head>";
	Page += "<meta name='viewport' content='width=device-width' />";
	Page += "<title>Setup</title>";
	Page += "<style> *, ::after, ::before { box-sizing: border-box; }";
	Page += "body {";
	Page += "margin: 0;";
	Page += "font-family: 'Segoe UI', Roboto, 'Helvetica Neue', Arial, 'Noto Sans', 'Liberation Sans';";
	Page += "font-size: 1rem;";
	Page += "font-weight: 400;";
	Page += "line-height: 1.5;";
	Page += "color: #212529;";
	Page += "background-color: #f5f5f5; }";
	Page += ".form-control {";
	Page += "display: block;";
	Page += "width: 100%;";
	Page += "height: calc(1.5em + 0.75rem + 2px);";
	Page += "border: 1px solid #ced4da; }";
	Page += "button {";
	Page += "cursor: pointer;";
	Page += "border: 1px solid transparent;";
	Page += "color: #fff;";
	Page += "background-color: #ff7200;";
	Page += "border-color: #ff7200;";
	Page += "padding: 0.5rem 1rem;";
	Page += "font-size: 1.25rem;";
	Page += "line-height: 1.5;";
	Page += "border-radius: 0.3rem;";
	Page += "width: 100%; }";
	Page += ".form-signin {";
	Page += "width: 100%;";
	Page += "max-width: 400px;";
	Page += "padding: 15px;";
	Page += "margin: auto; }";
	Page += "h1 { text-align: center; }";
	Page += "</style>";
	Page += "</head>";
	Page += "<body>";
	Page += "<main class='form-signin'>";
	Page += "<form action='wifisave' method='post'>";
	Page += "<h1>AC SETUP</h1>";
	Page += "<center>";

	switch (state)
	{
	case 0:
		Page += String(F("<p>ON Protocol: ")) + typeToString(ac_on_signal.decode_type).c_str() + F("</p>");
		Page += "<p><b>Send AC ON</b></p></center></body></main></html>";
		server.send(200, "text/html", Page);
		server.client().stop();
		while (!receive_ir_signal(&ac_on_signal))
			;
		if (!strcmp(typeToString(ac_on_signal.decode_type).c_str(), "UNKNOWN"))
		{
			LOG_SERIAL.println("UNKNOWN0");
		}
		else
		{
			state++;
		}
		break;
	case 1:
		Page += String(F("<p>ON Protocol: ")) + typeToString(ac_on_signal.decode_type).c_str() + F("</p>");
		Page += String(F("<p>OFF Protocol: ")) + typeToString(ac_off_signal.decode_type).c_str() + F("</p>");
		Page += "<p><b>Send AC OFF</b></p></center></body></main></html>";
		server.send(200, "text/html", Page);
		server.client().stop();
		while (!receive_ir_signal(&ac_off_signal))
			;
		if (!strcmp(typeToString(ac_off_signal.decode_type).c_str(), "UNKNOWN"))
		{
			LOG_SERIAL.println("UNKNOWN1");
		}
		else
		{
			state++;
		}
		break;
	case 2:
		Page += String(F("<p>ON Protocol: ")) + typeToString(ac_on_signal.decode_type).c_str() + F("</p>");
		Page += String(F("<p>OFF Protocol: ")) + typeToString(ac_off_signal.decode_type).c_str() + F("</p>");
		Page += "<p><b>Setup Done</b></p><br/><form method='get' action='wifi'><button type='submit'>Setup WIFI</button></form></center></body></main></html>";
		server.send(200, "text/html", Page);
		state = 0;
		break;
	}
}