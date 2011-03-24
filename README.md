Breathalyzer
============

This breathalyzer uses a PIC16F876A attached to an analog MQ-3 Gas Sensor and a [4-Digit 7-Segment Display](http://www.sparkfun.com/products/9483) driven by SPI.

It is compiled in MPLab IDE v8.60 using the HI-TECH C Compiler V9.80.

When the unit is turned on, the display continually updates the raw A/D value of the sensor (every 0.1s), more work needs to be done to convert that value into a reliable BAC.