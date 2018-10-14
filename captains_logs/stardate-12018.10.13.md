# DnD20 - Oct 13 2018

* Magic 8 ball works
* More webapp options

**I learned something amazing!**
Apparently the ESP8266 remembers your network credentials. I was messing around with the device after having it powered off for over an hour, and it started posting to Discord. Interesting stuff. I wonder if there is a way to access that info?

## Firmware Update
* I've combined the wifi mode into the progmode, it just makes more sense
* I've also added in magic 8 ball functionality
* I've updated the web app to accomodate the 8 ball, and changed it to allow a bot name
* I need to make it so that you can re-enter the wifi mode again. right now, it is dependent on password == "" && ssid == "". So once you set those, you never start the AP again