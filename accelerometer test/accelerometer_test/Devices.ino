// ----------------------------------
//           Accelerometer
// ----------------------------------
int get_accelerometer_reading()
{
  return accelerometer.get_reading();
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
