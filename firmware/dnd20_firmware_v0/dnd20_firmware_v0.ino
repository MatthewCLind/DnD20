/*
 * DnD20
 * Author / cobbler: Matthew Lind
 * 
 * Code cobbled together from these sources:
 * https://playground.arduino.cc/Main/MMA7455
 * https://github.com/Makuna/NeoPixelBus
 * https://github.com/adafruit/Adafruit_SSD1306
 * 
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

String ssid            = "";
String password        = "";
String discord_server  = "/api/webhooks/493947255796137997/UPfq26OFyCuKDLWLVQOe3MGuSH7aD9c-b3EhTGrzPNNXpCxKqHcB5UX-khEHoG9yidwE";
String player_name     = "Matt";
ESP8266WebServer server(80);
boolean server_running = false;


#include <NeoPixelBus.h>
const uint16_t PixelCount = 1;
// assumes GPIO3 for ESP8266
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount);
uint8_t colorSaturation = 66;
RgbColor RED(colorSaturation, 0, 0);
RgbColor GREEN(0, colorSaturation, 0);
RgbColor BLUE(0, 0, colorSaturation);
RgbColor YELLOW(colorSaturation >> 1, colorSaturation >> 1, 0);
RgbColor PURPLE(colorSaturation >> 2, 0, colorSaturation);
RgbColor WHITE(colorSaturation >> 2, colorSaturation >> 2, colorSaturation >> 2); 
RgbColor BLACK(0);

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 4
Adafruit_SSD1306 display(16);

const int STANDBY       = 0;
const int ROLLING       = 1;
const int DISPLAY_ROLL  = 2;
const int PROGMODE      = 3;
const int WIFI_CONFIG   = 4;

const int BUTTON_PIN = 14; //TODO

unsigned long seed = 0;
int roll = 1;
int die_sidedness = 20;

/*
Standby Mode
 - This is the mode where you look for a legitimate roll
 - You can also look for a button press and hold

 - Can move to Rolling Mode
 - Can move to Programming Mode
*/
int standby()
{
  int next_state = STANDBY;

  // Check for a roll
  const int NUM_READINGS = 10;
  const int ACCELERATION_THRESHOLD = 87; //TODO
  static int accelerometer_readings[NUM_READINGS];
  static int index = 0;

  // include latest reading into our vector
  accelerometer_readings[index] = get_accelerometer_magnitude();
  index = (index + 1) % (NUM_READINGS + 1);
  
  int total_acceleration = 0;
  for(int i = 0; i < NUM_READINGS; i++)
  {
    total_acceleration += abs(accelerometer_readings[i]);
  }
  
  //global seed
  seed += total_acceleration;

  // define a legitimate roll as a 'large' sum
  if(total_acceleration > ACCELERATION_THRESHOLD)
  {
    next_state = ROLLING;
  }

  // Check for a button press + hold
  const long BUTTON_HOLD_TIME = 3000; //ms
  if(button_hold(BUTTON_PIN, 5) > BUTTON_HOLD_TIME)
  {
      next_state = PROGMODE;
  }
  
  return next_state;
}

/*
Rolling Mode
 - Calculates the roll value from accelerometer data

 - Moves on to DISPLAY_ROLL Roll mode once settled
*/
int rolling()
{
  int next_state = DISPLAY_ROLL;
  randomSeed(seed);
  roll = (int)random(die_sidedness) + 1;
  return next_state;
}

/*
DISPLAY_ROLL Roll Mode
 - DISPLAY_ROLLs the roll value on the screen
 - Lights up the LED to appropriate color (eventually will animate)
 - Sends an HTTP POST request to Discord

 - Moves onto Standby Mode
*/
int display_roll()
{
  int next_state = STANDBY;
  
  // update the display
  update_OLED(roll);

  // update the LED
  if(roll == die_sidedness)
  {
    set_strip_color(YELLOW);
  }
  else if(roll == 1)
  {
    set_strip_color(RED);
  }
  else
  {
    set_strip_color(BLUE);
  }

  // send HTTP POST to Discord Server
  if(WiFi.status() == WL_CONNECTED)
  {
    send_http_request(roll);
    yield();
  }
  
  return next_state;
}


/*
Programming Mode
 - Allows you to cycle through types of die (i.e. d20, d4, etc)
 - DISPLAY_ROLLs the current sidedness of the die

 - Can move back to Standby Mode with button press and hold
*/
int progmode()
{
  int next_state = PROGMODE;
  const int MAX_SIDEDNESS = 20;
  static int new_sidedness = 2;

  set_strip_color(GREEN);

  const int BUTTON_HOLD = 3000; //ms
  const int BUTTON_PRESS = 20; //ms
  int press_time = button_hold(BUTTON_PIN, 5);
  if(press_time > BUTTON_HOLD)
  {
    die_sidedness = new_sidedness;
    // TODO save new_sidedness to PROGMEM
    set_strip_color(BLACK);
    next_state = STANDBY;
  }
  else if(press_time > BUTTON_PRESS)
  {
    new_sidedness = (new_sidedness + 1) % (MAX_SIDEDNESS + 1);
    if(new_sidedness < 2)
    {
      new_sidedness = 2;
    }
  }
  update_OLED(new_sidedness);
  return next_state;
}

/*
WiFi Configuration mode
 - This mode hosts a web page so that you can connect to WiFi
 - Also allows you to input your Discord server
 - Once connected, we won't enter into this mode again, unless WiFi is lost

 - Can move to STANDBY state if button is pressed and held
*/
int wifi_config()
{
  int next_state = WIFI_CONFIG;
  set_strip_color(PURPLE);
  const int BUTTON_HOLD_TIME = 3000; //ms
  yield();
  if(ssid != "" && password != "" && discord_server != "" && start_client())
  {
    next_state = STANDBY;
  }
  else if(button_hold(BUTTON_PIN, 5) > BUTTON_HOLD_TIME)
  {
    next_state = STANDBY;
  }
  else if(!server_running)
  {
    start_server();
  }
  else if(server_running)
  {
    server.handleClient();
  }
  
  return next_state;
}

int get_accelerometer_magnitude()
{
  // TODO
  int magnitude = 0;
  magnitude = random(11);
  return magnitude;
}

int button_hold(int btn_pin)
{
  long start_time = millis();
  while(digitalRead(btn_pin) == LOW)
  {
    // wait for release
    yield();
  }

  return (int)millis() - start_time;
}

int button_hold(int btn_pin, int debounce)
{
  long start_time = millis();
  while(digitalRead(btn_pin) == LOW)
  {
    // wait for release
    delay(debounce);
    yield();
  }
  return (int)millis() - start_time;
}

void set_strip_color(RgbColor c)
{
  for(int i = 0; i < PixelCount; i++)
  {
    strip.SetPixelColor(i, c);
  }
  strip.Show();
}

void update_OLED(int value)
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("   ");
  display.println(value);
  display.display();
  Serial.println(value);
}

void handleRoot() 
{
  const String website = "<h1>DnD20</h1> <form action='/submit' method='post'> <h2>WiFi Credentials</h2> SSID<input name='SSID' type='text'><br> password <input name='password' type='password'><br> <h2>Discord Info</h2> Server URL <input name='discord-server' type='text'><br> <input type='submit'> </form>";
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
      ssid = val;
      Serial.println("SSID: " + ssid);
    }
    else if(argname == "discord-server")
    {
      //discord_server = val;
    }
    else if(argname == "password")
    {
      password = val;
      Serial.println("password: " + password);
    }
  }
  server.send(200, "text/html", "A OK");
}

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
  server_running = true;
}

void shutdown_server()
{
  WiFi.softAPdisconnect();
  server.stop();
  yield();
  server_running = false;
}

boolean start_client()
{
  if(server_running)
  {
    shutdown_server();
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  yield();
  long timeout = millis() + 10000; //ms
  Serial.println("starting up client");
  while(timeout > millis())
  {
    // wait 10 seconds to try to connect
    yield();
  }
  if(WiFi.status() != WL_CONNECTED)
  {
    password = "";
    ssid = "";
    //discord-server == "";
    //player_name = "";
  }
  return WiFi.status() == WL_CONNECTED;
}

void send_http_request(int roll)
{
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect("www.discordapp.com", httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // This will send the request to the server
  client.print(String("POST ") + discord_server + " HTTPS/1.1\r\n" + 
                "Host: www.discordapp.com" + "\r\n" +
                "Content-Type: application/json\r\n" +
               "Connection: close\r\n" +
               player_name + " rolled: " +
               roll +
               "\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
}


typedef int (* Generic_State_Function_Array)();
Generic_State_Function_Array DnD20_States[5] = 
    {standby, rolling, display_roll, progmode, wifi_config};

void setup()
{
  Serial.begin(9600);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // LED
  strip.Begin();
  strip.Show();

  // OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(" DnD20");
  display.display();
}

void loop()
{
  static int next_state = WIFI_CONFIG;
  next_state = DnD20_States[next_state]();
  delay(1);
}
