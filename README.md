# Led_strips_under_bed
Led strips that lights up according to the output of distance sensors.

The project uses 8 of HC-SR04 ultrasonic distance sensors to measure the distance beetween a detected project. They are separated into 2 zones, 4 sensors each. The 2 zones are at a 90 deegre angle (on a corner). Appropriate leds on the 2 led strips light up according to the readout of the sensors. There are 4 zones according to the distance:
-more than 115 cm - nothing lights up
-beetween 85 cm and 115 cm - light up blue
-beetween 55 cm and 85 cm - light up green
-beetween 22 cm and 55 cm - light up red

Everything is controlled via Atmega328p.
