void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1')
  {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  }
  else
  {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
}

void reconnect()
{
  static boolean enable = true;
  static int counter = 0;
  if(enable == true)
  {
    // Loop until we're reconnected
    Serial.println("#E");
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("#F");
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("ESP1/status", "online");
      // ... and resubscribe
      client.subscribe("ESP1/in");
    }
    else
    {
      Serial.println("#G");
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      //delay(5000);
      enable = false;
    }
  }
  else
  {
    Serial.println("#H");
    counter++;
    if(counter == 5)
    {
      Serial.println("#I");
      enable = true;
      counter = 0;
    }
  }
}