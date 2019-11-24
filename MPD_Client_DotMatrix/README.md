# You have to change these environment.

```
char* ssid = "Your AP's SSID";
char* password = "Your AP's PASSWORD";
uint16_t port = 6600;
char * host = "192.168.10.45"; // ip or dns
```

# It needs to match your Matrix LED.

```
int pinCS = 15; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;
```

# Wirering

|DotMatrix||ESP8266|
|:-:|:-:|:-:|
|VCC|--|3.3V|
|GND|--|GND|
|DIN|--|IO13(D7)|
|CS|--|IO15(D8)|
|CLK|--|IO14(D5)|

# Wire cable length

Wire cable length should be as short as possible.   


![LedMatrix-1](https://user-images.githubusercontent.com/6020549/68864658-d87d3700-0734-11ea-9c62-08c02b5a8e18.JPG)

