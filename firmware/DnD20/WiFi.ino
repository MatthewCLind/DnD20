void start_server()
{
  const char* AP_SSID     = "DnD20";
  const char* AP_PASSWORD = "prestidigitation";
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
  server.on("/submit", handleSubmit);
  server.begin();
  Serial.println("HTTP server started");  
  wifi_state.server_running = true;
}

void shutdown_server()
{
  WiFi.softAPdisconnect();
  server.stop();
  wifi_state.server_running = false;
}

void handleRoot() 
{
  const char* website = "<h1>DnD20</h1> <form action='/submit' method='POST'><h2>WiFi Credentials</h2>SSID     <input name='SSID' type='text'><br>password <input name='password' type='password'><br><h2>Discord Info</h2>Host name (probably won't need to change) <input name='discord-host' type='text' value='discordapp.com'><br>Server URL <input name='discord-server' type='text'><br>Bot Name <input name='bot-name' type='text'><br><h2>Game Selection</h2><input type='radio' name='game-type' checked='true' value='D20'>D20<br><input type='radio' name='game-type' value='Magic 8 Ball'>Magic 8 Ball<br>Die-sidedness <input name='die-sidedness' type='number' max='20' min='2' step='1' value = '20'><br><br><input type='submit'></form>";
  
  server.send(200, "text/html", website);
}

void handleSubmit()
{
  for (uint8_t i=0; i<server.args(); i++)
  {
    String argname = server.argName(i);
    String val = server.arg(i);
    if(argname == "SSID")
    {
      wifi_state.ssid = val;
    }
    else if(argname == "password")
    {
      wifi_state.password = val;
    }
    else if(argname == "discord-host")
    {
      discord_credentials.host = val;
    }
    else if(argname == "discord-server")
    {
      discord_credentials.url = val;
    }
    else if(argname == "bot-name")
    {
      discord_credentials.bot_name = val;
    }
    else if(argname == "die-sidedness")
    {
      int new_sidedness = val.toInt();
      device_state.die_sidedness = (new_sidedness > 2 && new_sidedness < 20) ? new_sidedness : 20;
    }
    else if(argname == "game-type")
    {
      device_state.game_type = val;
      if(device_state.game_type == MAGIC_8)
      {
        device_state.die_sidedness = 20;
      }
    }
    else
    {
      //we were sent something unexpected
    }
  }
  server.send(200, "text/html", "A OK");
}

bool start_WiFi_client()
{
  if(wifi_state.server_running)
  {
    shutdown_server();
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_state.ssid.c_str(), wifi_state.password.c_str());
  long timeout = millis() + 3000; //ms
  Serial.println("starting up client");
  while(WiFi.status() != WL_CONNECTED && timeout > millis())
  {
    // wait 3 seconds to try to connect
    yield();
  }
  
  return WiFi.status() == WL_CONNECTED;
}

// assumes wifi is connected
void send_discord_http_request(int roll)
{
  WiFiClientSecure client;
  const int httpPort = 443;
  if (!client.connect(discord_credentials.host, httpPort))
  {
    Serial.println("connection failed");
    return;
  }

  // This will send the request to the server
  String message = "{\"username\": \"" + discord_credentials.bot_name + "\", \"content\": \"" + (String)device_state.roll + "\"}";
  client.print(String("POST ") + discord_credentials.url + " HTTP/1.1\r\n" +
               "Host: " + discord_credentials.host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Content-type: application/json\r\n" +
               "Connection: close\r\n" +
               "Content-length: " + (String)message.length() + 
               "\r\n\r\n");
  client.print(message);
  unsigned long timeout = millis();
  while (client.available() == 0)
  {
    if (millis() - timeout > 5000)
    {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available())
  {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
}
