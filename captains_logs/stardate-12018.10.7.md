# DnD20 - Oct 6 2018

*edit: this story has a happy ending*

Last night I was thinking about how there are two different configuration modes. There's the mode where you can select the die sidedness, and the mode where you set up wifi. I think these should be combined. Whenever you enter into *The* configuration mode, you will launch the wifi AP (btw, in the future the website should be made available on your wireless network once the credentials are put in).

Furthermore, I was thinking that this device is perfectly suited to be a magic 8 ball. It would be easy enough to choose which type of device you want in the web app using a radio button. Better would be a separate page for each device type, with configuration options, and you can choose and save.

Another improvement to make is to have the button press & hold functionality display a color change on the LED once you've held it down long enough to switch to a new mode.


## Accelerometer Notes
* Using this code [MMA7455](https://playground.arduino.cc/Main/MMA7455)
* I'm thinking that I will combine the 3 axes into a single magnitude value, but then again maybe not
* Noticing a lot of values sporadically of 65535 with the accelerometer simply staying put on the table
    * This is the 16 bit max value, so that is dubious
    * Ah, so it consistently switches back and forth when I rock the accelerometer. These must be negative numbers, let me see if I can find an unsigned int somewhere and make it signed.
    * In the loop() I cast the x,y,z values to int_16t in the print statements, now it looks nicer
* Because I'm having no immediate luck with FFT, I'm just going to wrap up this part of the prototype by considering a "roll" to be holding it at a high enough inclination


## Other improvements
* Added a timeout after the roll
* Made it so you can skip the timeout if you are ready to roll again



## HTTP Request notes
* To figure out how the headings are supposed to be set up, I'm going to CURL [HTTPBIN](http://httpbin.org/)
* I've succesfully sent a CURL request to my Discord server, so I'll duplicate that @ HTTPBIN
* Then I'll get my ESP8266 code to match, and it should be able to send the right stuff to Discord
* CURL info: [birdie0 discord webhooks](https://birdie0.github.io/discord-webhooks-guide/tools/curl.html)
* This works for me @ discord:
    * ` curl -H "Content-type: application/json" -X POST -d '{"username": "ROBOTO", "content": "content of content"}'https://discordapp.com/api/webhooks/####/##### `
* Here's what I get back from @ HTTPBIN:
    * ``` <!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 3.2 Final//EN">
<title>405 Method Not Allowed</title>
<h1>Method Not Allowed</h1>
<p>The method is not allowed for the requested URL.</p> ```
    * possibly because it doesn't want JSON data? or is it the POST?
    * it was the post and the URL
* second try:
    * ``` {
  "headers": {
    "Accept": "*/*",
    "Connection": "close",
    "Content-Length": "56",
    "Content-Type": "application/json",
    "Host": "httpbin.org",
    "User-Agent": "curl/7.52.1"
  }
} ```


* Hitting some issues with I2C dropping after going through the WiFi routine
    * This only occurs if I hardcode in my router credentials and start immediately into client mode
    
* This is what I get back from HTTPBIN:
    * ``` HTTP/1.1 400 Bad Request
Connection: close
Server: Cowboy
Date: Mon, 08 Oct 2018 00:50:32 GMT
Content-Length: 0 ```
* This is what my code looks like:
* NOTE: discord_server = "/headers"
    * ``` void send_http_request(int roll)
{
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect("httpbin.org", httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // This will send the request to the server
  client.print(String("GET ") + discord_server + " HTTP/1.1\r\n" + 
                "Host: httpbin.org" + "\r\n" +
                "Content-Type: application/json\r\n" +
               "Connection: close\r\n" +
               player_name + " rolled: " +
               roll +
               "\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
} ```
    * The payload wasn't in JSON format, maybe that is the issue? Yes.


* I'm getting 302 errord (FOUND). I'm pretty sure it is because I'm trying HTTP and not HTTPS
* In case you were wondering, you can't simply change the HTTP/1.1 to HTTPS/1.1 ... 
     * No, no. Haha, no wrong of course not. That would be too easy.
     * The *real* way to do it is to change ` WifiClient client` to `WifiClientSecure client `. Sophisticated. Refined.
     * Ah, shoot. Don't forget to change the library #include statement too
     * Well I don't know, it crashes.
     
    
* Attempting to use the ESP8266 example, "HTTPSRequest"
* Lots of fiddling, no 100% success yet
* Another note on CURL: use -v to see the headers
* It looks like the issue is that the server isn't reading the data as json data

# **SUCCESS!!!!!**
**Every single piece piece of functionality now works together!** (if not all together)


* After fiddling around getting all of the carriage returns and newlines right in the data, I also managed to get the JSON format right haha
* Using the HTTPSRequest example code I was able to send a message to my Discord server :D
* The reason it was crashing is because I was re-initializing my initialized devices (LED, OLED, Accelerometer)