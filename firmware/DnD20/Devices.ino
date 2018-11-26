// ----------------------------------
//            NeoPixel
// ----------------------------------

void set_neopixel_color(RgbColor c)
{
  for(int i = 0; i < NUM_NEOPIXELS; i++)
  {
    neopixel.SetPixelColor(i, c);
  }
  neopixel.Show();
}


// ----------------------------------
//           Accelerometer
// ----------------------------------
int get_accelerometer_reading()
{
  int reading = accelerometer.get_reading();
  //Serial.println(reading);
  // anything under 4 can be considered noise
  if(abs(reading) < 4)
  {
    reading = 0;
  }
  return reading;
}


// ----------------------------------
//               OLED
// ----------------------------------
void update_OLED(String value)
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("   ");
  display.println(value);
  display.display();
}

// ----------------------------------
//             Button
// ----------------------------------
int button_hold(int btn_pin)
{
  long start_time = millis();
  while(digitalRead(btn_pin) == LOW)
  {
    // wait for release
    yield();
  }

  return (int)(millis() - start_time);
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

int check_button_mode_select()
{
  // Check for a button press + hold
  const long BUTTON_HOLD_TIME = 1000; //ms
  int next_mode = device_state.mode;
  if(button_hold(BUTTON_PIN, 5) > BUTTON_HOLD_TIME)
  {
      next_mode = (next_mode + 1) % NUM_MODES;
  }
  return next_mode;
}
