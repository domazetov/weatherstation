//Libraries
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <PubSubClient.h>
#include <DHTesp.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <inttypes.h>
#include <NoDelay.h>

//Macros
#define DEVICEID            "WSX"
#ifndef APSSID
#define APSSID              "WeatherStationX"
#define APPSK               "12345678"
#endif
#define LOG                 true
#define LOG_SERIAL          if(LOG)Serial
#define BMP280_I2C_ADDRESS  0x76
#define MSG_BUFFER_SIZE     40
#define SLEEP_PIN           13
#define DEEPSLEEP_TIME      300000000   //5minutes (μs)
#define NORMALSLEEP         3000        //5seconds (ms)
#define PUBLISH_SLEEP       2000        //2seconds (ms)
#define RECONNECT_TIMEOUT   10000       //10seconds (ms)
#define CONNECT_TIMEOUT     60000       //60seconds (ms)

//Variables & Functions
DHTesp dht;
Adafruit_BMP280 bmp;
DNSServer dnsServer;
WiFiClient espClient;
ESP8266WebServer server(80);
IPAddress apip(172, 217, 28, 1);
IPAddress netmask(255, 255, 255, 0);
PubSubClient client(espClient);

const char *softap_ssid = APSSID;
const char *softap_password = APPSK;
const char *myhostname = APSSID;
const byte DNS_PORT = 53;
boolean connect;
unsigned long lastConnectAttempt = 0;
unsigned int status = WL_IDLE_STATUS;
unsigned long lastMsg = 0;
unsigned long lastReconnectAttempt = 0;
int value = 0;
char ssid[33] = "";
char password[65] = "";
char mqtt_server[20] = "";
char wstation_topic[11];

void setup()
{
	pinMode(SLEEP_PIN, INPUT);
	pinMode(BUILTIN_LED, OUTPUT);

	LOG_SERIAL.begin(115200);
	LOG_SERIAL.println("Configuring AP.");

	WiFi.softAPConfig(apip, apip, netmask);
	WiFi.softAP(softap_ssid, softap_password);
	delay(500);
	LOG_SERIAL.print("AP IP address: ");
	LOG_SERIAL.println(WiFi.softAPIP());

	dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
	dnsServer.start(DNS_PORT, "*", apip);

	server.on("/", handleWifi);
	server.on("/wifi", handleWifi);
	server.on("/wifisave", handleWifiSave);
	server.onNotFound(handleNotFound);
	server.begin();
	LOG_SERIAL.println("HTTP server started.");

	load_from_eeprom();
	connect = strlen(ssid) > 0;

	client.setServer(mqtt_server, 1883);

	dht.setup(2, DHTesp::DHT11);

	if (!bmp.begin(BMP280_I2C_ADDRESS))
	{
		LOG_SERIAL.println("Could not find a valid BMP280 sensor, check wiring or try a different address!");
		while (1) delay(10);
	}

	bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
					Adafruit_BMP280::SAMPLING_X2,
					Adafruit_BMP280::SAMPLING_X16,
					Adafruit_BMP280::FILTER_X16,
					Adafruit_BMP280::STANDBY_MS_500);

	String temp_string = String(DEVICEID) + String("/data");
	temp_string.toCharArray(wstation_topic, sizeof(wstation_topic));
}

void loop()
{
	if(connect)
	{
		LOG_SERIAL.println("Connection requested.");
		connect = false;
		connect_to_wifi();
		lastConnectAttempt = millis();
	}

	unsigned int s = WiFi.status();
	if (s == 0 && millis() > (lastConnectAttempt + CONNECT_TIMEOUT))
	{
		connect = true;
	}

	if (status != s)
	{
		LOG_SERIAL.print("Status: ");
		LOG_SERIAL.println(s);
		status = s;
		if (s == WL_CONNECTED)
		{
			LOG_SERIAL.println("");
			LOG_SERIAL.print("Connected to ");
			LOG_SERIAL.println(ssid);
			LOG_SERIAL.print("IP address: ");
			LOG_SERIAL.println(WiFi.localIP());

			if (!MDNS.begin(myhostname))
			{
				LOG_SERIAL.println("Error setting up MDNS responder!");
			}
			else
			{
				LOG_SERIAL.println("mDNS responder started");
				MDNS.addService("http", "tcp", 80);
			}
		}
		else if(s == WL_NO_SSID_AVAIL)
		{
			WiFi.disconnect();
		}
	}

	if (s == WL_CONNECTED)
	{
		MDNS.update();
		if (!client.connected())
		{
			unsigned long now = millis();
			if(now - lastReconnectAttempt > RECONNECT_TIMEOUT)
			{
				lastReconnectAttempt = now;
				if(reconnect_MQTT())
				{
					lastReconnectAttempt = 0;
				}
			}
		}
		client.loop();

		if (client.connected())
		{
			read_data();

            if(digitalRead(SLEEP_PIN))
            {
                LOG_SERIAL.println("deepSleep");
                ESP.deepSleep(DEEPSLEEP_TIME);
            }
            else
            {
                LOG_SERIAL.println("delay");
                delay(NORMALSLEEP);
            }
		}
	}
	dnsServer.processNextRequest();
	server.handleClient();
}
