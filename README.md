[![CodeFactor](https://www.codefactor.io/repository/github/ttocsneb/tixclock/badge/master)](https://www.codefactor.io/repository/github/ttocsneb/tixclock/overview/master)

# TIXClock
A home-made TIX clock.  I am still in development of this project.  It will be a TIX clock powered by an Arduino Pro Mini and a DS1307 RTC.

The idea came from thinking how I could reduce the pins needed for a 3x9 matrix.  Normally, you would need 12 pins.
However since you will only ever have 1 row on at a time, you can use a decoder to simplify the number of pins needed
to drive the matrix.

I was able to reduce the 12 pins to 8 pins (7 if you use a 4:16 decoder).
