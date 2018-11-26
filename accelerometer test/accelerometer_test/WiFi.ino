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
  int a[] = {333,213,415};
  String website = "k";
  server.send(200, "text/html", website);
}
