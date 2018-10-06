# DnD20 - Oct 5 2018

I have gotten a good start on the firmware for my prototype. I started by looking at the Arduino Examples for all of the systems I needed - ESP8266 WiFi, Makuna NeoPixel, MMA7455, etc. I foolishly tried combining all of the examples into a single file as if the way to construct a house is to start by unloading a pile of lumber into a heap on the ground. Learn from my mistakes folks, start with a blank, pristine file :)

I followed my state(less?) pattern that I outlined in this youtube video: https://youtu.be/jQ43RBiS8FQ

At this point I have the main framework down. One advantage of having all functionality lumped into strict states is that you can add in dummy code as a placeholder for functionality. Rather than implementing the OLED code right away, since it takes time, I've got a placeholder using the Serial port.

I also got a start on the webpage. I want to keep this as small (memory-wise) as possible for now, since I'm not sure how much the rest of the sketch is going to take.

The goal at this point is to a fully-functional prototype on a breadboard.