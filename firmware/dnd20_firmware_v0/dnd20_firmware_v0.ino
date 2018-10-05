/*
 * DnD20
 * Author / cobbler: Matthew Lind
 * 
 * Code cobbled together from these sources:
 * 
 * 
 */

const int STANDBY     = 0;
const int ROLLING     = 1;
const int DISPLAY     = 2;
const int PROGMODE    = 3;
const int WIFI_CONFIG = 4;

const int BUTTON_PIN = 8; //TODO

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
  const int NUM_READINGS = 10
  const int ACCELERATION_THRESHOLD = 20; //TODO
  static int accelerometer_readings[NUM_READINGS];
  static int index = 0;

  // include latest reading into our vector
  accelerometer_readings[index] = get_accelerometer_magnitude();
  index = (index + 1) % NUM_READINGS;
  
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
  long start_time = millis();
  while(digitalRead(BUTTON_PIN) == LOW)
  {
    if(millis() >= start_time + BUTTON_HOLD_TIME)
    {
      next_state = PROGMODE;
      break;
    }
  }
  
  return next_state;
}

/*
Rolling Mode
 - Calculates the roll value from accelerometer data

 - Moves on to Display Roll mode once settled
*/
unsigned long seed = get_accelerometer_magnitude()*get_accelerometer_magnitude();
int roll = 1;
int rolling()
{
  int next_state = DISPLAY;
  randomSeed(seed);
  roll = (int)random(die_sidedness) + 1;
  return next_state;
}

/*
Display Roll Mode
 - Displays the roll value on the screen
 - Lights up the LED to appropriate color (eventually will animate)
 - Sends an HTTP POST request to Discord

 - Moves onto Standby Mode
*/
int display_roll()
{
  int next_state = STANDBY;
  // update the display

  // update the LED
  if(roll == die_sidedness)
  {
    // GOLD
  }
  else if(roll == 1)
  {
    // RED
  }
  else
  {
    // CYAN
  }

  // send HTTP POST to Discord Server
  return next_state;
}


/*
Programming Mode
 - Allows you to cycle through types of die (i.e. d20, d4, etc)
 - Displays the current sidedness of the die

 - Can move back to Standby Mode with button press and hold
*/
int die_sidedness = 20;
int progmode()
{
  int next_state = PROGMODE;
  const int MAX_SIDEDNESS = 20;
  static new_sidedness = 2;

  // TODO turn LED on green

  const int BUTTON_HOLD = 3000; //ms
  long start_time = millis();
  while(digitalRead(BUTTON_PIN) == LOW)
  {
    // wait for release
  }

  if(millis() > start_time + BUTTON_HOLD)
  {
    die_sidedness = new_sidedness;
    // TODO save new_sidedness to PROGMEM
    // TODO turn LED off
    next_state = STANDBY;
  }
  else
  {
    new_sidedness = (new_sidedness + 1)%MAX_SIDEDNESS;
    if(new_sidedness < 2)
    {
      new_sidedness = 2;
    }
  }
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
  return next_state;
}

int get_accelerometer_magnitude()
{
  // TODO
  int magnitude = 0;
  return magnitude;
}

typedef int (* Generic_State_Function_Array)();
Generic_State_Function_Array DnD20_States[5] = 
    {standby, rolling, display_roll, progmode, wifi_config};

void setup()
{

}

void loop()
{
  static int next_state = 0;
  next_state = DnD20_States[next_state]();
}
