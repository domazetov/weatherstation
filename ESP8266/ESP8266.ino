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
#include <Adafruit_BMP280.h>
#include <inttypes.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRutils.h>

//Macros
#define DEVICEID                "WS2"
#define ENABLE_IR               false
#ifndef APSSID
#define APSSID                  "WeatherStation1"
#define APPSK                   "12345678"
#endif
#define LOG                     true
#define LOG_SERIAL              if(LOG)Serial
#define BMP280_I2C_ADDRESS      0x76
#define MSG_BUFFER_SIZE         40
#define LED_PIN                 2
#define BUFFER_SIZE             1024
#define TIMEOUT                 50
#define DNS_PORT                53
#define PUBLISH_SLEEP           2000        //2seconds (ms)
#define DEFAULT_SLEEP           8000       	//8seconds (ms)
#define NORMAL_SLEEP            PUBLISH_SLEEP + DEFAULT_SLEEP
#define RECONNECT_TIMEOUT       10000       //10seconds (ms)
#define CONNECT_TIMEOUT         60000       //60seconds (ms)
#define AC_OFF                  25500
#define DEEPSLEEP_OFF           0xFF
#if ENABLE_IR
#define SETUP_PIN               14
#define DHT_PIN                 10
#define RECEIVE_PIN             13
#define SEND_PIN                12
IRsend irsend(SEND_PIN);
IRrecv irrecv(RECEIVE_PIN, BUFFER_SIZE, TIMEOUT, false);
#else
#define SETUP_PIN               13
#define DHT_PIN                 2
#endif

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
boolean connect;
unsigned long lastConnectAttempt = 0;
unsigned int status = WL_IDLE_STATUS;
unsigned long lastMsg = 0;
unsigned long lastReconnectAttempt = 0;
int value = 0;
char ssid[33] = "";
char password[65] = "";
char mqtt_server[20] = "";
decode_results ac_on_signal;
decode_results ac_off_signal;
char ac_status[3] = "";

char publish_topic[11];
char subscribe_topic[11];

uint16_t average_temp;
bool send_data = true;

void setup()
{
	pinMode(SETUP_PIN, INPUT);
	pinMode(LED_PIN, OUTPUT);
#if ENABLE_IR
	irrecv.enableIRIn();
	irsend.begin();
#endif
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
#if ENABLE_IR
	server.on("/acsave", handleACSave);
#endif
	server.onNotFound(handleNotFound);
	server.begin();
	LOG_SERIAL.println("HTTP server started.");

	load_from_eeprom();
	connect = strlen(ssid) > 0;

	client.setServer(mqtt_server, 1883);
	client.setCallback(mqtt_callback);
	client.subscribe(subscribe_topic);

	dht.setup(DHT_PIN, DHTesp::DHT11);

	if (!bmp.begin(BMP280_I2C_ADDRESS)){
		LOG_SERIAL.println("Could not find a valid BMP280 sensor, check wiring or try a different address!");
		while (1) delay(10);
	}

	bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
					Adafruit_BMP280::SAMPLING_X2,
					Adafruit_BMP280::SAMPLING_X16,
					Adafruit_BMP280::FILTER_X16,
					Adafruit_BMP280::STANDBY_MS_500);

	String temp_string = String(DEVICEID) + String("/data");
	temp_string.toCharArray(publish_topic, sizeof(publish_topic));
	temp_string = String(DEVICEID) + String("/ack");
	temp_string.toCharArray(subscribe_topic, sizeof(subscribe_topic));
}

void reset_send_data()
{
	uint32_t currentmillis = millis();
	static uint32_t previousmillis = 0;
	if(currentmillis - previousmillis >= NORMAL_SLEEP){
		previousmillis = currentmillis;
		if(!send_data){
			send_data = true;
		}
	}
}

void loop()
{
	if(connect){
		LOG_SERIAL.println("Connection requested.");
		connect = false;
		connect_to_wifi();
		lastConnectAttempt = millis();
	}

	unsigned int s = WiFi.status();
	if (s == 0 && millis() > (lastConnectAttempt + CONNECT_TIMEOUT)){
		connect = true;
	}

	if (status != s){
		status = s;
		if (s == WL_CONNECTED){
			LOG_SERIAL.printf("\nConnected to %s, IP address: ", ssid);
			LOG_SERIAL.println(WiFi.localIP());

			if (!MDNS.begin(myhostname)){
				LOG_SERIAL.println("Error setting up MDNS responder!");
			}else{
				LOG_SERIAL.println("mDNS responder started");
				MDNS.addService("http", "tcp", 80);
			}
		}else if(s == WL_NO_SSID_AVAIL){
			WiFi.disconnect();
		}
	}
	if(digitalRead(SETUP_PIN)){//check if switch is ON
		if (s == WL_CONNECTED){
			MDNS.update();
			if (!client.connected()){
				unsigned long now = millis();
				if(now - lastReconnectAttempt > RECONNECT_TIMEOUT){
					lastReconnectAttempt = now;
					if(reconnect_mqtt()){
						lastReconnectAttempt = 0;
					}
				}
			}

			if (client.connected()){
				if(send_data){
					read_data();
					send_data = false;
				}else{
					reset_send_data();
				}
				client.loop();
			}
		}
	}else{
		dnsServer.processNextRequest();
		server.handleClient();
	}
}
