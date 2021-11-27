// Libraries
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

// Macros
#define BMP280_I2C_ADDRESS  0x76
#define MSG_BUFFER_SIZE     50
#ifndef APSSID
#define APSSID "ESP1"
#define APPSK  "12345678"
#endif
// Variables & Functions
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
const char *myhostname = "smarthouse";
const byte DNS_PORT = 53;
boolean connect;
unsigned long lastConnectTry = 0;
unsigned int status = WL_IDLE_STATUS;
unsigned long lastMsg = 0;
int value = 0;
char ssid[33] = "";
char password[65] = "";
char mqtt_server[20] = "";

void setup()
{
  pinMode(BUILTIN_LED, OUTPUT); 
  Serial.begin(115200);
  Serial.println("Configuring AP.");

  WiFi.softAPConfig(apip, apip, netmask);
  WiFi.softAP(softap_ssid, softap_password);
  delay(500);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apip);

  server.on("/", handleWifi);
  server.on("/wifi", handleWifi);
  server.on("/wifisave", handleWifiSave);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started.");

  load_from_eeprom();
  connect = strlen(ssid) > 0;

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  dht.setup(2, DHTesp::DHT11);

  if (!bmp.begin(BMP280_I2C_ADDRESS))
  {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    while (1) delay(10);
  }

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

void connect_to_wifi()
{
  Serial.println("Connecting as WiFi client.");
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  int connRes = WiFi.waitForConnectResult();
  Serial.print("Connection status: ");
  Serial.println(connRes);
}

void loop()
{
  Serial.println("#1");
  if(connect)
  {
    Serial.println("#2");
    Serial.println("Connection requested.");
    connect = false;
    connect_to_wifi();
    lastConnectTry = millis();
  }

  unsigned int s = WiFi.status();
  if (s == 0 && millis() > (lastConnectTry + 60000))
  {
    Serial.println("#3");
    connect = true;
  }

  if (status != s) 
  {
    Serial.println("#4");
    Serial.print("Status: ");
    Serial.println(s);
    status = s;
    if (s == WL_CONNECTED) 
    {
      Serial.println("#5");
      /* Just connected to WLAN */
      Serial.println("");
      Serial.print("Connected to ");
      Serial.println(ssid);
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      // Setup MDNS responder
      if (!MDNS.begin(myhostname)) 
      {
        Serial.println("#6");
        Serial.println("Error setting up MDNS responder!");
      }
      else 
      {
        Serial.println("#7");
        Serial.println("mDNS responder started");
        // Add service to MDNS-SD
        MDNS.addService("http", "tcp", 80);
      }
    }
    else if(s == WL_NO_SSID_AVAIL) 
    {
      Serial.println("#8");
      WiFi.disconnect();
    }
  }

  if (s == WL_CONNECTED) 
  {
    Serial.println("#9");
    MDNS.update();
  }

  delay(2000);
  if (s == WL_CONNECTED)
  {
    Serial.println("#A");
    if (!client.connected())
    {
      Serial.println("#B");
      reconnect();
    }
    client.loop();

    if (client.connected())
    {
      Serial.println("#C");
      unsigned long now = millis();
      if (now - lastMsg > 2000)
      {
        Serial.println("#D");
        lastMsg = now;
        read_dht();
        read_bmp();
        Serial.println("\n");
        delay(10000);
      }
    }
    // ESP.deepSleep(60e6);
  }
  //DNS
  dnsServer.processNextRequest();
  //HTTP
  server.handleClient();
}
