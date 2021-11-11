boolean captivePortal() 
{
  if (!isIp(server.hostHeader()) && server.hostHeader() != (String(myhostname) + ".local"))
  {
    Serial.println("Request redirected to captive portal");
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
  Page += F(
    "<!DOCTYPE html><html lang='en'><head>"
    "<meta name='viewport' content='width=device-width'>"
    "<title>Setup</title>"
    "<style>*,::after,::before{box-sizing:border-box;}body{margin:0;font-family:'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5;}.form-control{display:block;width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid #ced4da;}button{cursor: pointer;border:1px solid transparent;color:#fff;background-color:#ff7200;border-color:#ff7200;padding:.5rem 1rem;font-size:1.25rem;line-height:1.5;border-radius:.3rem;width:100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:auto;}h1{text-align: center}</style>"
    "</head><body>"
    "<main class='form-signin'><form action='wifisave' method='post'><h1 class=''>WiFi Setup</h1><br/><div class='form-floating'><label><b>SSID:</b></label><input type='text' class='form-control' placeholder='network' name='n'></div><div class='form-floating'><br/><label><b>Password:</b></label><input type='password' class='form-control' placeholder='password' name='p'></div><div class='form-floating'><br/><label><b>Broker:</b></label><input type='ip-address' class='form-control' placeholder='ip-address' name='b'></div><br/><button type='submit'>Save</button>"
  );

  if (server.client().localIP() == apip)
  {
    Page += String(F("<p>You are connected through the AP: ")) + softap_ssid + F("</p>");
  } else 
  {
    Page += String(F("<p>You are connected through the WiFi network: ")) + ssid + F("</p>");
  }

  Page += F(
    "</td></tr>"
    "</table>"
    "<table><tr><th align='left'>Current configuration:</th></tr>"
    "<tr><td>SSID "
  ) + String(ssid) + F(
    "</td></tr>"
    "<tr><td>IP "
  ) + toStringIp(WiFi.localIP()) + F(
    "</td></tr>"
    "<tr><td>Broker IP "
  ) + String(mqtt_server) + F(
    "</td></tr>"
    "</table>"
    "<table><tr><th align='left'>WiFi list:</th></tr>"
  );

  Serial.println("scan start");

  int n = WiFi.scanNetworks();

  Serial.println("scan done");

  if (n > 0) 
  {
    for (int i = 0; i < n; i++)
    {
      Page += String(F("\r\n<tr><td>")) + WiFi.SSID(i) + F("</td></tr>");
    }
  }
  else 
  {
    Page += F("<tr><td>No WiFi found</td></tr></body></form></main></html>");
  }
  server.send(200, "text/html", Page);
  server.client().stop(); // Stop is needed because we sent no content length
}

void handleWifiSave() 
{
  Serial.println("wifi save");
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
