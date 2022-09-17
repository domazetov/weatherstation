#include "website.h"

boolean captivePortal(){
	if (!isIp(server.hostHeader()) && server.hostHeader() != (String(myhostname) + ".local")){
		LOG_SERIAL.println("Request redirected to captive portal");
		server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
		server.send(302, "text/plain", "");
		server.client().stop();
		return true;
	}
	return false;
}

void handleWifi(){
	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");
	server.sendHeader("Expires", "-1");

	String Page;
	Page += main_head;
	Page += main_body;

	if (server.client().localIP() == apip){
		Page += String(F("<p>You are connected through the AP: ")) + softap_ssid + F("</p>");
	}else{
		Page += String(F("<p>You are connected through the WiFi network: ")) + ssid + F("</p>");
	}

	Page += "<table><tr><th align='left'>Current configuration:</th></tr>";
#if ENABLE_IR
	Page += String(F("<p>AC Protocol: ")) + typeToString(ac_on_signal.decode_type).c_str() + F("</p>");
#endif
	Page += F("<tr><td>SSID ") + String(ssid) + F("</td></tr><tr><td>IP ") + toStringIp(WiFi.localIP()) + F("</td></tr>");
	Page += F("<tr><td>Broker IP ") + String(mqtt_server) + F("</td></tr></table>");

	Page += "<table><tr><th align='left'>WiFi list:</th></tr>";
	LOG_SERIAL.println("scan start");
	int n = WiFi.scanNetworks();
	LOG_SERIAL.println("scan done");

	if (n > 0){
		for (int i = 0; i < n; i++){
			Page += String(F("\r\n<tr><td>")) + WiFi.SSID(i) + F("</td></tr>");
		}
	}else{
		Page += "<tr><td>No WiFi found</td></tr>";
	}
	Page += "</body></main></html>";
	server.send(200, "text/html", Page);
	server.client().stop();
}

void handleWifiSave(){
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

void handleNotFound(){
	if (captivePortal()){
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

	for (uint8_t i = 0; i < server.args(); i++){
		message += String(F(" ")) + server.argName(i) + F(": ") + server.arg(i) + F("\n");
	}
	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");
	server.sendHeader("Expires", "-1");
	server.send(404, "text/plain", message);
}

#if ENABLE_IR
void handleACSave(){
	LOG_SERIAL.println("AC SAVE");
	static uint8_t state = 0;

	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");
	server.sendHeader("Expires", "-1");

	String Page;
	Page += main_head;
	Page += "<body>";
	Page += "<main class='form-signin'>";
	Page += "<form action='wifisave' method='post'>";
	Page += "<h1>AC SETUP</h1>";
	Page += "<center>";

	switch (state){
	case 0:
		Page += String(F("<p>ON Protocol: ")) + typeToString(ac_on_signal.decode_type).c_str() + F("</p>");
		Page += "<p><b>Send AC ON</b></p></center></body></main></html>";
		server.send(200, "text/html", Page);
		server.client().stop();
		while (!receive_ir_signal(&ac_on_signal));
		if (!strcmp(typeToString(ac_on_signal.decode_type).c_str(), "UNKNOWN")){
			LOG_SERIAL.println("UNKNOWN0");
		}else{
			state++;
		}
		break;
	case 1:
		Page += String(F("<p>ON Protocol: ")) + typeToString(ac_on_signal.decode_type).c_str() + F("</p>");
		Page += String(F("<p>OFF Protocol: ")) + typeToString(ac_off_signal.decode_type).c_str() + F("</p>");
		Page += "<p><b>Send AC OFF</b></p></center></body></main></html>";
		server.send(200, "text/html", Page);
		server.client().stop();
		while (!receive_ir_signal(&ac_off_signal));
		if (!strcmp(typeToString(ac_off_signal.decode_type).c_str(), "UNKNOWN")){
			LOG_SERIAL.println("UNKNOWN1");
		}else{
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
#endif