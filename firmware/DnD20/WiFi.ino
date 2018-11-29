void start_server()
{
  const char AP_SSID[]     = "DnD20";
  const char AP_PASSWORD[] = "prestidigitation";
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
  server.on("/submit", handleSubmit);
  server.begin();
  Serial.println("HTTP server started");  
  wifi_credentials.server_running = true;
}

void shutdown_server()
{
  WiFi.softAPdisconnect();
  server.stop();
  wifi_credentials.server_running = false;
}

void handleRoot() 
{
  //const char* website = "<style>#project_header{background-color:#191919;position:absolute;top:0;left:0;right:0;font-family:Arial;padding:25px;color:#F3F3F3;font-size:20px;font-weight:500;text-align:center;z-index:10}form{font-family:Arial}.main_header{font-size:19px}.main_subheader{font-size:11px;opacity:.4}#content{position:relative;top:200px;text-align:center;opacity:1}.text_box_main{width:500px;margin:auto;margin-bottom:50px;box-shadow:0 4px 20px rgba(0,0,0,.15);padding:10px}.body_text_header{color:#191919;font-weight:700;opacity:.6;border-style:groove;border-top:none;border-left:none;border-right:none;padding:24px;width:100px;margin:auto;border-width:1px}.body_text_tag{color:#191919;font-weight:700;opacity:.6;font-size:14px;padding:10px;width:100px;margin:auto}.body_code_main{padding:10px;margin:10px;display:inline-block}input[class|=simple_large_submit]{border-radius:5px;background-color:#0c541f;border-color:#116b29;color:#F3F3F3;padding:10px;width:240px;overflow:hidden;border-style:groove;opacity:.5;font-weight:400;font-size:16px;position:relative;margin:5px}input[class|=simple_large_submit]:focus{outline:0;opacity:.7}</style> <div id=project_header> <p class=main_header>DnD20</p> <p class=main_subheader>Configuration</p> </div> <form action=/submit method=POST> <div id=content> <div class=text_box_main> <p class=body_text_header>WiFi Credentials</p> <div class=body_code_main> <input class=simple_large value=SSID><br> <input class=simple_large value=Password type=password><br> </div> </div> <div class=text_box_main> <p class=body_text_header>Discord Info</p> <div class=body_code_main> <input class=simple_large value=discordapp.com><br> <input class=simple_large value='discord webhook url'><br> <input class=simple_large value=bot-name><br> </div> </div> <div class=text_box_main> <div class=body_code_main> <input type=submit class=simple_large_submit value=Submit> </div> </div> </div> </form>";
  const char* website = "Hello";
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
      wifi_credentials.ssid = val;
    }
    else if(argname == "password")
    {
      wifi_credentials.password = val;
    }
    else if(argname == "discord-host")
    {
      discord_data.host = val;
    }
    else if(argname == "discord-server")
    {
      discord_data.url = val;
    }
    else if(argname == "bot-name")
    {
      discord_data.bot_name = val;
    }
    else if(argname == "die-sidedness")
    {
      int new_sidedness = val.toInt();
      device_state.die_sidedness = (new_sidedness > 2 && new_sidedness < 20) ? new_sidedness : 20;
    }
    else if(argname == "game-type")
    {
      if(val == "D20")
      {
        device_state.game_type = 0;
      }
      else if(val == "Magic-8")
      {
        device_state.game_type = 1;
      }
      else
      {
        // we'll just stick with whatever type we're in as default
      }
      
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
  if(wifi_credentials.server_running)
  {
    shutdown_server();
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_credentials.ssid.c_str(), wifi_credentials.password.c_str());
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
  if (!client.connect(discord_data.host, httpPort))
  {
    Serial.println("connection failed");
    return;
  }

  // This will send the request to the server
  String message = "{\"username\": \"" + discord_data.bot_name + "\", \"content\": \"" + (String)device_state.roll + "\"}";
  client.print(String("POST ") + discord_data.url + " HTTP/1.1\r\n" +
               "Host: " + discord_data.host + "\r\n" +
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
}
