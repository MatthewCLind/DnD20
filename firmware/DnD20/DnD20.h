/*
 * DnD20.h
 * Author: Matthew Lind
 * last revision: 11/23/2018
*/

#ifndef DnD20_H
#define DnD20_H

#include <Wire.h>
#include <SPI.h>
#include <NeoPixelBus.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266WebServer.h>


// ----------------------------------
//            DnD20.ino
// ----------------------------------
struct device_data
{
  byte mode;
  bool run_mode_setup;
  byte game_type;
  byte die_sidedness;
  int roll;
} device_state;

const byte NUM_MODES  = 3;
const byte GAME_MODE  = 0;
const byte PROGMODE   = 1;
const byte WIFI_SETUP = 2;

const String mode_display_strings[NUM_MODES] = {"Game Mode", "Game Setup", "WiFi Setup"};

const byte D20 = 0;
const byte MAGIC_8 = 1;

void game_mode();
void progmode();
void wifi_setup();

typedef void (* Generic_State_Function_Array)();
Generic_State_Function_Array DnD20_Modes[NUM_MODES] = {game_mode, progmode, wifi_setup};


// ----------------------------------
//           Devices.ino
// ----------------------------------

// NeoPixel
// assumes GPIO3 for ESP826
const int NUM_NEOPIXELS = 1;
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> neopixel(NUM_NEOPIXELS);
struct rainbow
{
  RgbColor red;
  RgbColor green;
  RgbColor blue;
  RgbColor yellow;
  RgbColor orange;
  RgbColor purple;
  RgbColor white;
  RgbColor black;
} colors;
void set_neopixel_color(RgbColor);


// OLED
Adafruit_SSD1306 display;
void update_OLED(String value, int font_size);
void update_OLED(String value);


// Button
const int BUTTON_PIN = 14;
/*
bool button_press(int btn_pin, int max_time = 500, int min_time = 5);
bool button_hold(int btn_pin, int min_time);
int button_time(int btn_pin);
*/
int button_mode_select();
int button_mode_select(bool* button_press);


// ----------------------------------
//           EEPROM_Management.ino
// ----------------------------------
struct EEPROM_data
{
  int maddr;
  int msize;
};

struct EEPROM_Map
{
  EEPROM_data mode;
  EEPROM_data type;
  EEPROM_data die_sidedness;
  EEPROM_data url;
  EEPROM_data host;
  EEPROM_data bot_name;
  EEPROM_data ssid;
  EEPROM_data password;
} EEPROM_map;

void EEPROM_map_init();
String load_block(EEPROM_data data);
void EEPROM_save_block(String val, EEPROM_data data);
void EEPROM_save_data();
void EEPROM_load_data();


// ----------------------------------
//           WiFi.ino
// ----------------------------------

struct discord_info
{
  String url;
  String host;
  String bot_name;
} discord_data;

struct wifi_info
{
  String ssid;
  String password;
  bool server_running;
} wifi_credentials;

// WiFi Server
ESP8266WebServer server(80);
void start_server();
void shutdown_server();
void handleRoot();
void handleSubmit();

bool start_WiFi_client();
void send_discord_http_request(int roll);


#endif
