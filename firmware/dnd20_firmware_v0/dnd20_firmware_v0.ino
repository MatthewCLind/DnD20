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

#include <NeoPixelBus.h>
const uint16_t PixelCount = 1;
// assumes GPIO3 for ESP8266
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount);
uint8_t colorSaturation = 128;
RgbColor RED(colorSaturation, 0, 0);
RgbColor GREEN(0, colorSaturation, 0);
RgbColor BLUE(0, 0, colorSaturation);
RgbColor YELLOW(colorSaturation >> 1, colorSaturation >> 1, 0);
RgbColor PURPLE(colorSaturation >> 2, 0, colorSaturation);
RgbColor WHITE(colorSaturation >> 2, colorSaturation >> 2, colorSaturation >> 2); 
RgbColor BLACK(0);

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
  // TODO Serve the website

  set_strip_color(PURPLE);
  const int BUTTON_HOLD_TIME = 3000; //ms
  if(button_hold(BUTTON_PIN, 5) > BUTTON_HOLD_TIME)
  {
    next_state = STANDBY;
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
  // TODO
  Serial.println(value);
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
}

void loop()
{
  static int next_state = WIFI_CONFIG;
  next_state = DnD20_States[next_state]();
}
