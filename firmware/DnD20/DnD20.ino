/*
 * DnD20
 * Author: Matthew Lind
 * last revision: 11/23/2018
 * 
 * License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 * 
 * Overview:
 * DnD20 overall is a device for sharing your dice rolls online through Discord.
 * If you want to set this device up, you'll need to create a Webhook on your Discord server.
 * 
 * Organization:
 * DnD20.h holds all of the declarations. Arduino will stitch all of these files together
 *  when compiled. They are split up for convenience only, so DnD20.h allows the files
 *  to "be on the same page". Get it?
 * DnD20.ino is the main file where the 3 modes live, Game Mode, Game Setup, and WiFi Setup.
 * Devices.ino is for device-level functions.
 * EEPROM_Management.ino is functions to save and load configuration data
 * WiFi.ino handles the Access Point and HTTP Client code
 * 
 * 
 * Sources:
 * https://github.com/Makuna/NeoPixelBus
 * https://github.com/adafruit/Adafruit_SSD1306
 */

#include "DnD20.h"


/*
 * Game Mode is the main mode of the device.
 * Depending on configuration, the device will act as a:
 *  - D20, D12, D10, D8, D6, or D4
 *   or
 *  - a Magic 8 Ball
 * Only the dice send HTTP POSTs to discord (if connected to WiFi)
 */
void game_mode()
{ 
  // game_mode setup block
  if(device_state.run_mode_setup)
  {
    if(WiFi.status() != WL_CONNECTED)
    {
      start_WiFi_client();
    }
    if(device_state.game_type == MAGIC_8)
    {
      set_neopixel_color(colors.purple);
    }
    display_mode(device_state.mode);
    device_state.run_mode_setup = false;
  }

  bool button_click;
  device_state.mode = button_mode_select(&button_click);

  if(button_click)
  {    
    // blink the LED to separate identical rolls
    set_neopixel_color(colors.black);
    
    delay(100);
    // make the random generator more random
    int seed = millis();
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
       "Most      likely", "Sure", "Yep", "For sure", "Ummm", "Ask later", "Can't say", "Dunno", "What?", "No", "No way",
       "Probly not", "Nuh-uh", "Doubt it"};

       update_OLED(RESPONSES[device_state.roll - 1]);
       set_neopixel_color(colors.purple);
    }

    delay(20); // avoid button jitter
  }

  if(device_state.mode != GAME_MODE)
  {
    set_neopixel_color(colors.black);
    update_OLED("");
    device_state.run_mode_setup = true;
  }
}


/*
 * Game Setup is the mode where you can cycle through different dice,
 * or set it to be a Magic 8 Ball
 * 
 * To cycle to different modes, press and hold the button
 */
void game_setup()
{
  const int MAX_SIDEDNESS = 20;

  if(device_state.run_mode_setup)
  {
    set_neopixel_color(colors.green);
    display_mode(device_state.mode);
    device_state.run_mode_setup = false;
  }

  bool button_click;
  device_state.mode = button_mode_select(&button_click);
  
  //button click to scroll through the different dice / game types
  if(button_click)
  {
    if(device_state.game_type == D20)
    {
      const int num_dice = 6;
      const int dice_types[num_dice] = {4, 6, 8, 10, 12, 20};
      int current_die_index = 0;
      Serial.println(device_state.die_sidedness);
      for(int i = 0; i < num_dice; i++)
      {
        if(device_state.die_sidedness == dice_types[i])
        {
          current_die_index = i;
          break;
        }
      }
      current_die_index--;
      
      if(current_die_index < 0)
      {
        device_state.die_sidedness = 20;
        device_state.game_type = MAGIC_8;
        update_OLED(String("Magic     8 Ball"));
      }
      else
      {
        device_state.die_sidedness = dice_types[current_die_index];
        display_dN(device_state.die_sidedness);
      }
    }
    else // game type == Magic 8 Ball, cycle back to D20
    {
      device_state.game_type = D20;
      device_state.die_sidedness = 20;
      display_dN(device_state.die_sidedness);
    }

    delay(20); // avoid button jitter
  }

  // set up for the next mode
  if(device_state.mode != GAME_SETUP)
  {
    set_neopixel_color(colors.black);
    update_OLED("");
    device_state.run_mode_setup = true;
    EEPROM_save_data();
  }
}

/*
 * WiFi setup hosts an access point that you can log onto with a browser
 *  SSID: D20
 *  Password: prestidigitation
 *  
 * Once in this mode, the OLED will display whatever IP address you should connect to
 * Upon connecting, you will be served a web page to put in your Discord and WiFi credentials
 */
void wifi_setup()
{
  if(device_state.run_mode_setup)
  {
    start_server();
    set_neopixel_color(colors.orange);
    device_state.run_mode_setup = false;
  }

  server.handleClient();
  device_state.mode = button_mode_select();

  // set up for the next mode
  if(device_state.mode != WIFI_SETUP)
  {
    shutdown_server();
    set_neopixel_color(colors.black);
    update_OLED("");
    device_state.run_mode_setup = true;
    EEPROM_save_data();
  }
}


void setup()
{
  Serial.begin(115200);
  Serial.println("Setup Begin");
  
  device_state.run_mode_setup = true;
  device_state.roll = 1; //critical failure :(
  EEPROM_map_init();
  EEPROM_load_data();

  // Button
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.display();

  // NeoPixel
  colors.red    = RgbColor(66,  0,  0);
  colors.green  = RgbColor( 0, 66,  0);
  colors.blue   = RgbColor( 0,  0, 66);
  colors.yellow = RgbColor(33, 33,  0);
  colors.orange = RgbColor(44, 22,  0);
  colors.purple = RgbColor(16,  0, 33);
  colors.white  = RgbColor(33, 16, 16);
  colors.black  = RgbColor( 0,  0,  0);
  neopixel.Begin();
  neopixel.Show();

  // OLED
  Adafruit_SSD1306 display(16);
}

void loop()
{ 
  DnD20_Modes[device_state.mode]();
  
  // must give time for WiFi functions to avoid watchdog timer
  yield();
}
