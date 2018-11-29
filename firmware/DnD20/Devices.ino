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
//               OLED
// ----------------------------------
void update_OLED(String value)
{
  //             font sizes  1   2   3   4
  const int char_width[]  = {6, 12, 18, 24};
  const int char_height[] = {8, 16, 24, 32};

  // maximum number of characters at font 1 = 84
  int num_chars = (value.length() < 84) ? value.length() : 84;
  int font_size;
  int x_offset = 0;
  int y_offset = 0;

  if(num_chars <= 5)
  {
    font_size = 4;

    y_offset = (32 - char_height[3]) / 2;
    x_offset = (128 - (char_width[3] * num_chars)) / 2;
  }
  else if(num_chars <= 7)
  {
    font_size = 3;
    
    y_offset = (32 - char_height[2]) / 2;
    x_offset = (128 - (char_width[2] * num_chars))/2;
  }
  else if(num_chars <= 20)  // size 2 accomodates 2 rows of 10 chars
  {
    font_size = 2;
    
    x_offset = (num_chars <= 10) ? (128 - (char_width[1] * num_chars))/2 : 0;
    
    int rows = (num_chars <= 10) ? 1 : 2;
    y_offset = (32 - (char_height[1] * rows)) / 2;
  }
  else
  {
    int max_columns = 21;
    font_size = 1;
    x_offset = (num_chars <= 21) ? (128 - (char_width[0] * num_chars))/2 : 0;
    
    int rows;
    if(num_chars <= max_columns)
    {
      rows = 1;
    }
    else if(num_chars <= 2*max_columns)
    {
      rows = 2;
    }
    else if(num_chars <= 3*max_columns)
    {
      rows = 3;
    }
    else if(num_chars <= 4*max_columns)
    {
      rows = 4;
    }
    y_offset = (32 - (char_height[0] * rows)) / 2;
  }
  
  display.clearDisplay();
  display.setTextSize(font_size);
  display.setCursor(x_offset, y_offset);
  display.println(value);
  display.display();
}

void display_mode(int zmode)
{
  String display_string = mode_display_strings[zmode];
  update_OLED(display_string);
}

// ----------------------------------
//             Button
// ----------------------------------
/*
bool button_press(int btn_pin, int max_time, int min_time)
{
  int button_down_time = button_time(btn_pin);
  return button_down_time < max_time && button_down_time > min_time;
}

bool button_hold(int btn_pin, int min_time)
{
  int button_down_time = button_time(btn_pin);
  return button_down_time > min_time;
}

int button_time(int btn_pin)
{
  long start_time = millis();
  while(digitalRead(btn_pin) == LOW)
  {
    // wait for release
    yield();
  }

  return (int)(millis() - start_time);
}*/

int button_mode_select(bool* button_press)
{
  *button_press = false;
  bool mode_change = false;
  int next_mode = device_state.mode;
  long start_time = millis();
  int elapsed_time;
  
  while(digitalRead(BUTTON_PIN) == LOW)
  {
    elapsed_time = millis() - start_time;
    if(elapsed_time > 1500)
    {
      mode_change = true;
      next_mode++;
      next_mode = next_mode % NUM_MODES;
      display_mode(next_mode);
      start_time = millis(); // so you can continue to cycle through
    }
    yield();
  }

  if(!mode_change && elapsed_time > 10 && elapsed_time < 1000)
  {
    Serial.println("we think it is a button");
    *button_press = true;
  }

  return next_mode;
}

int button_mode_select()
{
  int next_mode = device_state.mode;
  long start_time = millis();
  int elapsed_time;
  
  while(digitalRead(BUTTON_PIN) == LOW)
  {
    elapsed_time = millis() - start_time;
    if(elapsed_time > 1500)
    {
      next_mode++;
      next_mode = next_mode % NUM_MODES;
      display_mode(next_mode);
      start_time = millis(); // so you can continue to cycle through
    }
    yield();
  }

  return next_mode;
}
