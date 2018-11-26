/*
 * DnD20
 * Author: Matthew Lind
 * last revision: 11/23/2018
 * 
 * Sources:
 * https://playground.arduino.cc/Main/MMA7455
 * https://github.com/Makuna/NeoPixelBus
 * https://github.com/adafruit/Adafruit_SSD1306
 * 
 */

#include "DnD20.h"

/*
TODO - rewrite
Game Mode
 - This is the mode where you look for a legitimate device_state.roll
 - You can also look for a button press + hold

 - Can move to Rolling Mode
 - Can move to Programming Mode
*/
void game_mode()
{
  // game_mode setup block
  if(device_state.run_mode_setup)
  {
    Serial.println("In game mode");
    update_OLED(device_state.game_type);
    if(WiFi.status() != WL_CONNECTED)
    {
      start_WiFi_client();
    }
    device_state.run_mode_setup = false;
  }
  
  // Check for a roll
  const int NUM_READINGS = 100;
  static int readings[NUM_READINGS]; //keep track of the previous 100 accelerometer readings ~0.8 seconds
  static int reading_index = 0;
  int sign_changes;
  bool roll_detected = false;

  reading_index++;
  reading_index %= NUM_READINGS;
  readings[reading_index] = get_accelerometer_reading();

  // to determine if the device was rolled,
  // we are going to count the number of times the signal crosses 0
  // I.E. poor-man's fourier transform
  for(int i = reading_index + 1; i < (NUM_READINGS + reading_index + 1); i++)
  {
    static int last_reading = 1; // because we initialize with 1, we can have a false positive for a sign-change
    int current_reading = readings[i];
    
    // being on zero is not useful
    if(current_reading == 0)
    {
      continue;
    }
    
    int change_val = last_reading * current_reading;
    if(change_val < 0) // the ony way you get a negative value is from a sign-change
    {
      sign_changes++;
    }
    
    last_reading = current_reading;
    Serial.println(current_reading);
  }

  //Serial.print("sign_changes = ");
  //Serial.println(sign_changes);

  roll_detected = sign_changes > 3;
  sign_changes = 0;
  
  if(roll_detected)
  {
    // make the random generator more random
    static int seed = 0;
    seed += readings[0] * readings[0];
    randomSeed(seed);
    device_state.roll = (int)random(device_state.die_sidedness) + 1;
    
    if(device_state.game_type == D20)
    {
      update_OLED(String(device_state.roll));
      
      // update the LED
      if(device_state.roll == device_state.die_sidedness)
      {
        set_neopixel_color(colors.yellow);
      }
      else if(device_state.roll == 1)
      {
        set_neopixel_color(colors.red);
      }
      else
      {
        set_neopixel_color(colors.blue);
      }
  
      // send HTTP POST to Discord Server
      if(WiFi.status() == WL_CONNECTED)
      {
        send_discord_http_request(device_state.roll);
      }
    }
    else if(device_state.game_type == MAGIC_8)
    {
      const String RESPONSES[] = 
      {"Certainly", "Mmhmm", "No doubt", "Yes", "Do it", "Probly",
       "Most likely", "Sure", "Yep", "For sure", "Ummm", "Ask later", "Can't say", "Dunno", "What?", "No", "No way",
       "Probly not", "Nuh-uh", "Doubt it"};

       update_OLED(RESPONSES[device_state.roll]);
    }

    // give the users a chance to read the display before accepting new inputs
    // TODO - make this a timer interrupt?
    long start_time = millis();
    while(millis() - start_time < 2500)
    {
      yield();
    }
  }

  // user can cycle modes with press+hold
  device_state.mode = check_button_mode_select();

  // if nothing is happening, turn the lights off to save battery
  // alternatively, if we are switching modes, give the new mode a tabla rasa
  if(device_state.mode != GAME_MODE)
  {
    set_neopixel_color(colors.black);
    update_OLED("");
  }
  
  if(device_state.mode != GAME_MODE)
  {
    device_state.run_mode_setup = true;
  }
}

/*
Programming Mode
 - Allows you to cycle through types of dice (i.e. d20, d4, etc)
 - DISPLAY_ROLLs the current sidedness of the die

 - Can move back to Standby Mode with button press and hold
*/
void progmode()
{
  const int MAX_SIDEDNESS = 20;

  // TODO: perhaps track the LED color in device_state so you don't continually update the LED
  //       might also want to track what is on the OLED in that case,
  //       alternatively, you could track whether this is a "fresh start" for a new mode,
  //       IE the first time into the mode, so it will run whatever "setup" block is necessary
  if(device_state.run_mode_setup)
  {
    Serial.println("PROGMODE");
    set_neopixel_color(colors.green);
    update_OLED(" PROGMODE");
    device_state.run_mode_setup = false;
  }

  int press_time = button_hold(BUTTON_PIN, 5);
  bool btn_click = press_time > 20 && press_time < 1000;
  
  //check for a button click to scroll through the different dice / game types
  if(btn_click)
  {
    Serial.println("button has been clicked in progmode");
    if(device_state.game_type == D20)
    {
      device_state.die_sidedness = (device_state.die_sidedness + 1) % (MAX_SIDEDNESS + 1);
      if(device_state.die_sidedness < 2)
      {
        device_state.die_sidedness = 20;
        device_state.game_type = MAGIC_8;
        update_OLED(MAGIC_8);
      }
      else
      {
        update_OLED(String(device_state.die_sidedness));
      }
    }
    else // game type == Magic 8 Ball, cycle back to D20
    {
      device_state.game_type = D20;
    }
  }
  else if(!wifi_state.server_running)
  {
    start_server();
  }
  else
  {
    server.handleClient();
  }

  // user can cycle modes with press+hold
  device_state.mode = check_button_mode_select();

  // if nothing is happening, turn the lights off to save battery
  if(device_state.mode != PROGMODE) // || handle_inactivity())
  {
    Serial.println("Handling inactivity or state change");
    shutdown_server();
    set_neopixel_color(colors.black);
    update_OLED("");
  }
  if(device_state.mode != PROGMODE)
  {
    device_state.run_mode_setup = true;
  }
}


bool handle_inactivity()
{
  const long inactive_time = 6000; // hold on, now wait just a minute
  long start_time = millis();

  while(digitalRead(BUTTON_PIN) == HIGH && get_accelerometer_reading() < 10)
  {
    if(millis() - start_time > inactive_time)
    {
      return true;
    }
    yield();
  }
  return false;
}


void setup()
{
  device_state.mode = PROGMODE;
  device_state.run_mode_setup = true;
  device_state.game_type = D20;
  device_state.die_sidedness = 20;
  device_state.roll = 1;
  
  Serial.begin(9600);

  // Button
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(" DnD20");
  display.display();

  // accelerometer
  Wire.begin();
  MMA7455 accelerometer();

  // NeoPixel
  colors.red    = RgbColor(66, 0, 0);
  colors.green  = RgbColor(0, 66, 0);
  colors.blue   = RgbColor(0, 0, 66);
  colors.yellow = RgbColor(33, 33, 0);
  colors.purple = RgbColor(16, 0, 33);
  colors.white  = RgbColor(33, 16, 16);
  colors.black  = RgbColor(0, 0, 0);
  neopixel.Begin();
  neopixel.Show();

  // OLED
  Adafruit_SSD1306 display(16);

  // WiFi
  discord_credentials.url = "";
  discord_credentials.host = "discordapp.com";
  discord_credentials.bot_name = "DnD20-bot";
  

  Serial.println("Devices Initialized");
}

void loop()
{  
  // run either game mode or programming mode
  DnD20_Modes[device_state.mode]();

  // must give time for WiFi functions to avoid watchdog timer
  yield();
}
