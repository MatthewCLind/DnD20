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
#include <WiFiClientSecure.h> 
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <NeoPixelBus.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 4

// All from MMA7455 code
#define MMA7455_XOUTL 0x00      // Read only, Output Value X LSB
#define MMA7455_XOUTH 0x01      // Read only, Output Value X MSB
#define MMA7455_YOUTL 0x02      // Read only, Output Value Y LSB
#define MMA7455_YOUTH 0x03      // Read only, Output Value Y MSB
#define MMA7455_ZOUTL 0x04      // Read only, Output Value Z LSB
#define MMA7455_ZOUTH 0x05      // Read only, Output Value Z MSB
#define MMA7455_XOUT8 0x06      // Read only, Output Value X 8 bits
#define MMA7455_YOUT8 0x07      // Read only, Output Value Y 8 bits
#define MMA7455_ZOUT8 0x08      // Read only, Output Value Z 8 bits
#define MMA7455_STATUS 0x09     // Read only, Status Register
#define MMA7455_DETSRC 0x0A     // Read only, Detection Source Register
#define MMA7455_TOUT 0x0B       // Temperature Output Value (Optional)
#define MMA7455_RESERVED1 0x0C  // Reserved
#define MMA7455_I2CAD 0x0D      // Read/Write, I2C Device Address
#define MMA7455_USRINF 0x0E     // Read only, User Information (Optional)
#define MMA7455_WHOAMI 0x0F     // Read only, "Who am I" value (Optional)
#define MMA7455_XOFFL 0x10      // Read/Write, Offset Drift X LSB
#define MMA7455_XOFFH 0x11      // Read/Write, Offset Drift X MSB
#define MMA7455_YOFFL 0x12      // Read/Write, Offset Drift Y LSB
#define MMA7455_YOFFH 0x13      // Read/Write, Offset Drift Y MSB
#define MMA7455_ZOFFL 0x14      // Read/Write, Offset Drift Z LSB
#define MMA7455_ZOFFH 0x15      // Read/Write, Offset Drift Z MSB
#define MMA7455_MCTL 0x16       // Read/Write, Mode Control Register 
#define MMA7455_INTRST 0x17     // Read/Write, Interrupt Latch Reset
#define MMA7455_CTL1 0x18       // Read/Write, Control 1 Register
#define MMA7455_CTL2 0x19       // Read/Write, Control 2 Register
#define MMA7455_LDTH 0x1A       // Read/Write, Level Detection Threshold Limit Value
#define MMA7455_PDTH 0x1B       // Read/Write, Pulse Detection Threshold Limit Value
#define MMA7455_PD 0x1C         // Read/Write, Pulse Duration Value
#define MMA7455_LT 0x1D         // Read/Write, Latency Time Value (between pulses)
#define MMA7455_TW 0x1E         // Read/Write, Time Window for Second Pulse Value
#define MMA7455_RESERVED2 0x1F  // Reserved

// Defines for the bits, to be able to change 
// between bit number and binary definition.
// By using the bit number, programming the MMA7455 
// is like programming an AVR microcontroller.
// But instead of using "(1<<X)", or "_BV(X)", 
// the Arduino "bit(X)" is used.
#define MMA7455_D0 0
#define MMA7455_D1 1
#define MMA7455_D2 2
#define MMA7455_D3 3
#define MMA7455_D4 4
#define MMA7455_D5 5
#define MMA7455_D6 6
#define MMA7455_D7 7

// Status Register
#define MMA7455_DRDY MMA7455_D0
#define MMA7455_DOVR MMA7455_D1
#define MMA7455_PERR MMA7455_D2

// Mode Control Register
#define MMA7455_MODE0 MMA7455_D0
#define MMA7455_MODE1 MMA7455_D1
#define MMA7455_GLVL0 MMA7455_D2
#define MMA7455_GLVL1 MMA7455_D3
#define MMA7455_STON MMA7455_D4
#define MMA7455_SPI3W MMA7455_D5
#define MMA7455_DRPD MMA7455_D6

// Control 1 Register
#define MMA7455_INTPIN MMA7455_D0
#define MMA7455_INTREG0 MMA7455_D1
#define MMA7455_INTREG1 MMA7455_D2
#define MMA7455_XDA MMA7455_D3
#define MMA7455_YDA MMA7455_D4
#define MMA7455_ZDA MMA7455_D5
#define MMA7455_THOPT MMA7455_D6
#define MMA7455_DFBW MMA7455_D7

// Control 2 Register
#define MMA7455_LDPL MMA7455_D0
#define MMA7455_PDPL MMA7455_D1
#define MMA7455_DRVO MMA7455_D2

// Interrupt Latch Reset Register
#define MMA7455_CLR_INT1 MMA7455_D0
#define MMA7455_CLR_INT2 MMA7455_D1

// Detection Source Register
#define MMA7455_INT1 MMA7455_D0
#define MMA7455_INT2 MMA7455_D1
#define MMA7455_PDZ MMA7455_D2
#define MMA7455_PDY MMA7455_D3
#define MMA7455_PDX MMA7455_D4
#define MMA7455_LDZ MMA7455_D5
#define MMA7455_LDY MMA7455_D6
#define MMA7455_LDX MMA7455_D7

// I2C Device Address Register
#define MMA7455_I2CDIS MMA7455_D7



// Default I2C address for the MMA7455
#define MMA7455_I2C_ADDRESS 0x1D


// When using an union for the registers and 
// the axis values, the byte order of the accelerometer
// should match the byte order of the compiler and AVR chip.
// Both have the lower byte at the lower address, 
// so they match.
// This union is only used by the low level functions.
typedef union xyz_union
{
  struct
  {
    uint8_t x_lsb;
    uint8_t x_msb;
    uint8_t y_lsb;
    uint8_t y_msb;
    uint8_t z_lsb;
    uint8_t z_msb;
  } reg;
  struct 
  {
    int16_t x;
    int16_t y;
    int16_t z;
  } value;
};


// --------------------------------------------------------
// MMA7455_init
//
// Initialize the MMA7455.
// Set also the offset, assuming that the accelerometer is 
// in flat horizontal position.
// 
// Important notes about the offset:
//    The sensor has internal registers to set an offset.
//    But the offset could also be calculated by software.
//    This function uses the internal offset registers 
//    of the sensor.
//    That turned out to be bad idea, since setting the
//    offset alters the actual offset of the sensor.
//    A second offset calculation had to be implemented 
//    to fine tune the offset.
//    Using software variables for the offset would be 
//    much better.
// 
//    The offset is influenced by the slightest vibration
//    (like a computer on the table).
//    
int MMA7455_init(void)
{
  uint16_t x, y, z;
  int error;
  xyz_union xyz;
  uint8_t c1, c2;

  // Initialize the sensor
  //
  // Sensitivity:
  //    2g : GLVL0
  //    4g : GLVL1
  //    8g : GLVL1 | GLVL0
  // Mode:
  //    Standby         : 0
  //    Measurement     : MODE0
  //    Level Detection : MODE1
  //    Pulse Detection : MODE1 | MODE0
  // There was no need to add functions to write and read 
  // a single byte. So only the two functions to write 
  // and read multiple bytes are used.

  // Set mode for "2g sensitivity" and "Measurement Mode".
  c1 = bit(MMA7455_GLVL0) | bit(MMA7455_MODE0);
  error = MMA7455_write(MMA7455_MCTL, &c1, 1);
  if (error != 0)
    return (error);

  // Read it back, to test the sensor and communication.
  error = MMA7455_read(MMA7455_MCTL, &c2, 1);
  if (error != 0)
    return (error);

  if (c1 != c2)
    return (-99);

  // Clear the offset registers.
  // If the Arduino was reset or with a warm-boot,
  // there still could be offset written in the sensor.
  // Only with power-up the offset values of the sensor 
  // are zero.
  xyz.value.x = xyz.value.y = xyz.value.z = 0;
  error = MMA7455_write(MMA7455_XOFFL, (uint8_t *) &xyz, 6);
  if (error != 0)
    return (error);

  // The mode has just been set, and the sensor is activated.
  // To get a valid reading, wait some time.
  delay(100);

#define USE_INTERNAL_OFFSET_REGISTERS
#ifdef USE_INTERNAL_OFFSET_REGISTERS

  // Calcuate the offset.
  //
  // The values are 16-bits signed integers, but the sensor
  // uses offsets of 11-bits signed integers.
  // However that is not a problem, 
  // as long as the value is within the range.

  // Assuming that the sensor is flat horizontal, 
  // the 'z'-axis should be 1 'g'. And 1 'g' is 
  // a value of 64 (if the 2g most sensitive setting 
  // is used).  
  // Note that the actual written value should be doubled
  // for this sensor.

  error = MMA7455_xyz (&x, &y, &z); // get the x,y,z values
  if (error != 0)
    return (error);

  xyz.value.x = 2 * -x;        // The sensor wants double values.
  xyz.value.y = 2 * -y;
  xyz.value.z = 2 * -(z-64);   // 64 is for 1 'g' for z-axis.

  error = MMA7455_write(MMA7455_XOFFL, (uint8_t *) &xyz, 6);
  if (error != 0)
    return (error);

  // The offset has been set, and everything should be okay.
  // But by setting the offset, the offset of the sensor
  // changes.
  // A second offset calculation has to be done after 
  // a short delay, to compensate for that.
  delay(200);

  error = MMA7455_xyz (&x, &y, &z);    // get te x,y,z values again
  if (error != 0)
    return (error);

  xyz.value.x += 2 * -x;       // add to previous value
  xyz.value.y += 2 * -y;
  xyz.value.z += 2 * -(z-64);  // 64 is for 1 'g' for z-axis.

  // Write the offset for a second time.
  // This time the offset is fine tuned.
  error = MMA7455_write(MMA7455_XOFFL, (uint8_t *) &xyz, 6);
  if (error != 0)
    return (error);

#endif

  return (0);          // return : no error
}


// --------------------------------------------------------
// MMA7455_xyz
//
// Get the 'g' forces.
// The values are with integers as 64 per 'g'.
//
int MMA7455_xyz( uint16_t *pX, uint16_t *pY, uint16_t *pZ)
{
  xyz_union xyz;
  int error;
  uint8_t c;

  // Wait for status bit DRDY to indicate that 
  // all 3 axis are valid.
  do
  {
    error = MMA7455_read (MMA7455_STATUS, &c, 1);
  } while ( !bitRead(c, MMA7455_DRDY) && error == 0);
  if (error != 0)
    return (error);

  // Read 6 bytes, containing the X,Y,Z information 
  // as 10-bit signed integers.
  error = MMA7455_read (MMA7455_XOUTL, (uint8_t *) &xyz, 6);
  if (error != 0)
    return (error);

  // The output is 10-bits and could be negative.
  // To use the output as a 16-bit signed integer,
  // the sign bit (bit 9) is extended for the 16 bits.
  if (xyz.reg.x_msb & 0x02)    // Bit 9 is sign bit.
    xyz.reg.x_msb |= 0xFC;     // Stretch bit 9 over other bits.
  else
    xyz.reg.x_msb &= 0x3;

  if (xyz.reg.y_msb & 0x02)
    xyz.reg.y_msb |= 0xFC;
  else
    xyz.reg.y_msb &= 0x3;

  if (xyz.reg.z_msb & 0x02)
    xyz.reg.z_msb |= 0xFC;
  else
    xyz.reg.z_msb &= 0x3;

  // The result is the g-force in units of 64 per 'g'.
  *pX = xyz.value.x;
  *pY = xyz.value.y;
  *pZ = xyz.value.z;

  return (0);                  // return : no error
}


// --------------------------------------------------------
// MMA7455_read
//
// This is a common function to read multiple bytes 
// from an I2C device.
//
// It uses the boolean parameter for Wire.endTransMission()
// to be able to hold or release the I2C-bus. 
// This is implemented in Arduino 1.0.1.
//
// Only this function is used to read. 
// There is no function for a single byte.
//
int MMA7455_read(int start, uint8_t *buffer, int size)
{
  int i, n, error;

  Wire.beginTransmission(MMA7455_I2C_ADDRESS);
  n = Wire.write(start);
  if (n != 1)
    return (-10);

  n = Wire.endTransmission(false); // hold the I2C-bus
  if (n != 0)
    return (n);

  // Third parameter is true: relase I2C-bus after data is read.
  Wire.requestFrom(MMA7455_I2C_ADDRESS, size, true);
  i = 0;
  while(Wire.available() && i<size)
  {
    buffer[i++]=Wire.read();
  }
  if ( i != size)
    return (-11);

  return (0);                  // return : no error
}


// --------------------------------------------------------
// MMA7455_write
//
// This is a common function to write multiple bytes 
// to an I2C device.
//
// Only this function is used to write. 
// There is no function for a single byte.
//
int MMA7455_write(int start, const uint8_t *pData, int size)
{
  int n, error;

  Wire.beginTransmission(MMA7455_I2C_ADDRESS);
  n = Wire.write(start);        // write the start address
  if (n != 1)
    return (-20);

  n = Wire.write(pData, size);  // write data bytes
  if (n != size)
    return (-21);

  error = Wire.endTransmission(true); // release the I2C-bus
  if (error != 0)
    return (error);

  return (0);                   // return : no error
}

// End MMA7455 code block


String ssid            = "";
String password        = "";
String url  = "/api/webhooks/493947255796137997/UPfq26OFyCuKDLWLVQOe3MGuSH7aD9c-b3EhTGrzPNNXpCxKqHcB5UX-khEHoG9yidwE";
String host = "discordapp.com";
String player_name     = "Matt-bot";
ESP8266WebServer server(80);
boolean server_running = false;

const uint16_t PixelCount = 1;
// assumes GPIO3 for ESP8266
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount);
uint8_t colorSaturation = 66;
RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor yellow(colorSaturation >> 1, colorSaturation >> 1, 0);
RgbColor purple(colorSaturation >> 2, 0, colorSaturation);
RgbColor white(colorSaturation >> 2, colorSaturation >> 2, colorSaturation >> 2); 
RgbColor black(0);

Adafruit_SSD1306 display(16);

const int STANDBY       = 0;
const int ROLLING       = 1;
const int DISPLAY_ROLL  = 2;
const int ROLL_TIMEOUT  = 3;
const int PROGMODE      = 4;
const int WIFI_CONFIG   = 5;

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
  const int ACCELERATION_THRESHOLD = 400; //TODO
  static int accelerometer_readings[NUM_READINGS];
  static int index = 0;

  // include latest reading into our vector
  accelerometer_readings[index] = get_accelerometer_reading();
  index = (index + 1) % (NUM_READINGS + 1);
  
  int total_acceleration = 0;
  for(int i = 0; i < NUM_READINGS; i++)
  {
    total_acceleration += accelerometer_readings[i];
  }
  Serial.println(total_acceleration);
  
  //global seed
  seed += total_acceleration;

  // define a legitimate roll as a 'large' sum
  if(total_acceleration > ACCELERATION_THRESHOLD)
  {
    next_state = ROLLING;
  }

  // Check for a button press + hold
  const long BUTTON_HOLD_TIME = 1000; //ms
  if(button_hold(BUTTON_PIN, 5) > BUTTON_HOLD_TIME)
  {
      next_state = PROGMODE;
  }

  //Reset the readings array so it doesn't interfere with the next roll
  if(next_state != STANDBY)
  {
    for(int i = 0; i < NUM_READINGS; i++)
    {
      accelerometer_readings[i] = 0;
    }
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
  int next_state = ROLL_TIMEOUT;
  
  // update the display
  update_OLED(roll);

  // update the LED
  if(roll == die_sidedness)
  {
    set_strip_color(yellow);
  }
  else if(roll == 1)
  {
    set_strip_color(red);
  }
  else
  {
    set_strip_color(blue);
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
Post-Roll Timeout Mode
 - Give the user a chance to check out the roll and move the die without triggering another roll
 
 - Advances to Standby
 */
int roll_timeout()
{
  int next_state = STANDBY;
  long timeout = millis() + 5000;
  while(millis() < timeout && button_hold(BUTTON_PIN) < 20)
  {
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

  set_strip_color(green);

  const int BUTTON_HOLD = 1000; //ms
  const int BUTTON_PRESS = 20; //ms
  int press_time = button_hold(BUTTON_PIN, 5);
  if(press_time > BUTTON_HOLD)
  {
    die_sidedness = new_sidedness;
    // TODO save new_sidedness to PROGMEM
    set_strip_color(black);
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
  set_strip_color(purple);
  const int BUTTON_HOLD_TIME = 1000; //ms
  yield();
  if(ssid != "" && password != "" && host != "" && start_client())
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

int get_accelerometer_reading()
{
  int magnitude = 0;
  uint16_t x,y,z;
  MMA7455_xyz(&x, &y, &z);

  magnitude = (int16_t) x;
  magnitude = (int) magnitude;
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
  while(WiFi.status() != WL_CONNECTED || timeout > millis())
  {
    // wait 10 seconds to try to connect
    yield();
  }
  Serial.println("we are now connected");
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
  WiFiClientSecure client;
  const int httpPort = 443;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // This will send the request to the server
  String message = "{\"username\": \"" + player_name + "\", \"content\": \"" + roll + "\"}";
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Content-type: application/json\r\n" +
               "Connection: close\r\n" +
               "Content-length: " + message.length() + 
               "\r\n\r\n");
  client.print(message);
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

void initialize_devices()
{
  Serial.println("Initializing devices");
  Serial.println("LED");
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

  // Accelerometer
  Wire.begin();
  MMA7455_init();

  Serial.println("Devices Initialized");
}


typedef int (* Generic_State_Function_Array)();
Generic_State_Function_Array DnD20_States[] = 
    {standby, rolling, display_roll, roll_timeout, progmode, wifi_config};

void setup()
{
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  initialize_devices();
}

void loop()
{
  static int next_state = WIFI_CONFIG;
  next_state = DnD20_States[next_state]();
  delay(1);
}
