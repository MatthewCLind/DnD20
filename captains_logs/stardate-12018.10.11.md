# DnD20 - Oct 11 2018

* Schematic done and PCB started

## Schematic Work

* Starting with another project, which basically duplicated the Adafruit Feather Huzzah, I have started on the schematic
* I noticed that the WS2812 data line is also on the RX line, where the USB programming data comes in, which is why the LED changes colors while it is being programmed. I wonder if there is a good way to separate the two?
* **Oh No!** The MMA7455 accelerometer is obsolete D:
    * I found this one: http://cache.freescale.com/files/sensors/doc/data_sheet/MMA8653FC.pdf
    * I'm going to just move forward with this, because odds are the i2c is similar, but even if it isn't 1-to-1, I know that there is *some* way to interface with it. I'm ok with a theoretically working circuit right now, since my main goal is to get an entire project wrapped up

### MMA8653 Notes
* Perfect, the MMA8653FC already has a schematic symbol in KiCAD :)
* Interesting, there is mention of a "Freefall" interrupt, which is probably what the INT pins are for. That might be a good thing to implement, that way there is the option of using the INT pins, and maybe detecting a freefall is the best possible way to ensure a roll?
    * Check under section 6.7 for more info
* Tentatively attaching INT1 and INT2 on the MMA to the ESP12 GPIO10 and GPIO09
    * look at this for more pin info (pinfo) https://www.esp8266.com/wiki/doku.php?id=esp8266_gpio_pin_allocations#interrupts
