/*
 * DnD20
 * Author: Matthew Lind
 * last revision: 11/23/2018
 * 
 * Sources:
 * https://github.com/Makuna/NeoPixelBus
 * https://github.com/adafruit/Adafruit_SSD1306
 * 
 */

#include "DnD20.h"


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


void progmode()
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
        Serial.println(dice_types[i]);
        if(device_state.die_sidedness = dice_types[i])
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
        String die_type = "d";
        die_type.concat(device_state.die_sidedness);
        update_OLED(die_type);
      }
    }
    else // game type == Magic 8 Ball, cycle back to D20
    {
      device_state.game_type = D20;
      device_state.die_sidedness = 20;
    }

    delay(20); // avoid button jitter
  }

  // set up for the next mode
  if(device_state.mode != PROGMODE)
  {
    set_neopixel_color(colors.black);
    update_OLED("");
    device_state.run_mode_setup = true;
    EEPROM_save_data();
  }
}


void wifi_setup()
{
  if(device_state.run_mode_setup)
  {
    start_server();
    set_neopixel_color(colors.orange);
    display_mode(device_state.mode);
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

  /*bool do_stuff;
  int i = button_mode_select(&do_stuff);
  if(do_stuff)
  {
    Serial.println("In Here");
  }*/
  
  
  // must give time for WiFi functions to avoid watchdog timer
  yield();
  
}
