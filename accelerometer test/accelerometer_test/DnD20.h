/*
 * DnD20.h
 * Author: Matthew Lind
 * last revision: 11/16/2018
*/

#ifndef DnD20_H
#define DnD20_H

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266WebServer.h>

#include "MMA7455.h"

// ----------------------------------
//            accelerometer_test.ino
// ----------------------------------

const int NUM_DATA_POINTS = 1000;
int data[NUM_DATA_POINTS];
void test_mode();

// ----------------------------------
//           Devices.ino
// ----------------------------------

// Accelerometer
MMA7455 accelerometer;
int get_accelerometer_reading();

// Button
const int BUTTON_PIN = 14;
int button_hold(int btn_pin);
int button_hold(int btn_pin, int debounce);


// ----------------------------------
//           WiFi.ino
// ----------------------------------

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

#endif
