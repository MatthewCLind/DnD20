/*
 * accelerometer_test
 * Author: Matthew Lind
 * last revision: 11/21/2018
 * 
 * This is to generate accelerometer data for analysis
 * The code is largely the same as DnD20
 * 
 * Sources:
 * https://playground.arduino.cc/Main/MMA7455
 * https://github.com/Makuna/NeoPixelBus
 * https://github.com/adafruit/Adafruit_SSD1306
 * 
 */

#include "DnD20.h"


long time_it(void func())
{
  long start_time = micros();
  func();
  return micros() - start_time;
}


void test_mode()
{

  int press_time = button_hold(BUTTON_PIN, 5);
  //check for a button click to scroll through the different dice
  if(press_time > 20 && press_time < 1000)
  {
    
    auto gather_data = []()
    {
      for(int i = 0; i < NUM_DATA_POINTS; i++)
      {
        data[i] = get_accelerometer_reading();
        //yield();
      }  
    };

    gather_data();

    Serial.print("data = [");
    Serial.print(data[0]);
    for(int i = 1; i < NUM_DATA_POINTS; i++)
    {
      Serial.print(", ");
      Serial.print(data[i]);
      yield();
    }
    Serial.println("]");
    /*long how_long = time_it(gather_data);
    Serial.print("It took ");
    Serial.print(how_long);
    Serial.println("us to gather readings");*/
  }
}

void setup()
{  
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.println("Initializing devices");
  Serial.println("LED");

  // accelerometer
  Wire.begin();
  MMA7455 accelerometer();

  Serial.println("Devices Initialized");
}

void loop()
{  
  // run either game mode or programming mode
  test_mode();

  // must give time to watchdog timer for WiFi functions
  yield();
}
