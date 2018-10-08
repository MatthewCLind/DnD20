/pulled from captain's log stardate-12018.10.4/

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