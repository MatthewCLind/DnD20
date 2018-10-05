# DnD20 - Oct 4 2018
When playing DnD with your friends over the internet, some of the magic is lost. There's something about the latency and missing out on the dice rolling that limits DnD online. As for the dice, I think we can do something about that.

The idea here is to make an electronic die that can interface with your online session so that when you roll, your friends can watch the outcome with you in real time and cheer or jeer depending on the results (and their alignment).

Once you get an IoT die, there are lots more possibilities to include things like "fancy rolls" (tm) which allows you to specify what in the heck you are rolling for. I always forget what I need to add up when I decide to throw my axe rather than use it like a melee weapon. The die / app combo could all figure that out for me.

Yes, there's plenty of good possibilities here, but to start out, we need something that we can actually make.

My first pass at the systems' design looks like this:

# Power
- li-ion battery powered
- needs to last 10 hours (5 hours of gameplay, plus 5 hours of "oops I forgot to charge this")
- On/Off switch to save power
- USB rechargeable
- Display
- Displays up to 99 (IE 2 digits)
- Backlit
- RGB LED
- Flashes heroically when a nat 20 is rolled
- Lights up triumphantly when denatured 20 is rolled
- Burns with sorrow when a critical failure is rolled

# Button Input
- One push button to cycle through type of die
- Push + hold for 3 seconds to enter programming mode
- Set it to D2, D3, ... , D99
- Display shows die type at boot

# Accelerometer
- Determines between roll and simply being moved about
    - How?
    - Band-pass filter?
    - Magnitude of force-vector?
    - Conic-volume traced out by force-vector > some amount?
    - Area of surface traced out by force-vector?
- Provides data to create random number

# WiFi
- At first provides access point for configuration
    * Ideally captive access point
    * Configuration = wifi credentials, Discord server config
- If wifi router access is unobtainable reverts to being an AP

# FUTURE DEV
 - Could interface with online DnD character sheets
 - so you can "roll attack" and it will actually calculate for you
- E.G. interface with Roll20 and OrcPub
- Interface ought to be simple and universal. So in the WiFi config, you simply specify an URL to POST the json with data
- Also accepts json to know what type of die to roll and what modifiers to put on it
- Bluetooth would also be nice so it can have a native phone app that interfaces for easy selection