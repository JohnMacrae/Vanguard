# Vanguard
Camper Van Temperature Voltage monitor for ESP32. USes the internal ADC on 2 pins to monitor vehicle and leisure batteries. A DS18b20 is used to monitor temperature. If configurable preset parameters are exceeded, the app uses Telegram to send a status alarm. Uses Universal Telegram library by @Witnessmenow, and IOTAppstory.  The app is has wifi provisioning, and configuarable parameters.

The app spends most of its time in deep sleep. When it wakes up, it checks the voltages and temperature. Occasionally it checks to see if there are any updates. 

</b>Configuration</b>
To wake up the app, send a /status message and wait till it wakes up and returns the ststus. Press the right button for 8 seconds and it will enter the configuration mode. Find the device on your network. (Fing is a useful app for this). Enter the IP address from fing into your browser and configure the parameters. It is recommended only to alter the voltage and temperature parameters and maybe the sleep interval. When you are finished, make sure to exit configuration mode although it will time out on its own after a while.
