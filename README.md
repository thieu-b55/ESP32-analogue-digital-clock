# ESP32-analogue-digital-clock
ESP32 clock with analogue and digital readout

digital indication of:

time

date

temperature

analogue display by 60 leds led ring WS2812B

indication of:

hour_____________3 blue leds

minutes __________1 red led

seconds __________1 green led

hour position ______1 green/red led

DS3231SN RTC module is sybchronised with GPS module every hour

Ambient temperature measurement using DS18B20

The following data can be set via ESP32 own network

* when used without GPS module

*** time

*** date

* when used with GPS module

*** summertime

*** UTC difference
