Garage_node10
=============

Jeenode Garage
 
Uses two DHT22 temp & humidity sensors on Node 10.
 
Code from Radioblip
2012-05-09 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php
 
For monitoring Temperature, Humidity and Carbon Monoxide in the garage and wormbox.
Also checks if the garage door is open.
 
The CO sensor heater must be cycled at 1.4v for 90 seconds and 5v for 60 seconds.
At the time just before the 1.4v to 5v transition, the sensor can be read.
The other sensors are read at this time as well, and the data is then sent by radio.
