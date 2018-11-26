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

#include "MMA7455.h"

// ----------------------------------
//            DnD20.ino
// ----------------------------------
struct device_data
{
  int mode;
  bool run_mode_setup;
  String game_type;
  int die_sidedness;
  int roll;
} device_state;

const int NUM_MODES = 2;
const int GAME_MODE = 0;
const int PROGMODE  = 1;

const String MAGIC_8 = "Magic 8 Ball";
const String D20 = "D20";

void game_mode();
void progmode();

int  check_button_mode_select();
bool handle_inactivity();

typedef void (* Generic_State_Function_Array)();
Generic_State_Function_Array DnD20_Modes[NUM_MODES] = {game_mode, progmode};


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
  RgbColor purple;
  RgbColor white;
  RgbColor black;
} colors;

void set_neopixel_color(RgbColor);


// Accelerometer
MMA7455 accelerometer;
int get_accelerometer_reading();


// OLED
Adafruit_SSD1306 display;
void update_OLED(String value);


// Button
const int BUTTON_PIN = 14;
int button_hold(int btn_pin);
int button_hold(int btn_pin, int debounce);


// ----------------------------------
//           WiFi.ino
// ----------------------------------

struct discord_info
{
  String url;
  String host;
  String bot_name;
} discord_credentials;

struct wifi_info
{
  String ssid;
  String password;
  bool server_running;
} wifi_state;

// WiFi Server
ESP8266WebServer server(80);
void start_server();
void shutdown_server();
void handleRoot();
void handleSubmit();

bool start_client();
void send_discord_http_request(int roll);


#endif
