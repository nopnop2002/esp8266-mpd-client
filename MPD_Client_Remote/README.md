# You have to change these environment.

```
char* ssid = "Your AP's SSID";
char* password = "Your AP's PASSWORD";
uint16_t port = 6600;
char * host = "192.168.10.45"; // ip or dns
```

# It needs to match your remote control.

```
#define PLAY 0x00FF18E7
#define STOP 0x00FF38C7
#define PREV 0x00FF10EF
#define NEXT 0x00FF5AA5
#define VOLU 0x00FF52AD
#define VOLD 0x00FF42BD
```

# Required library
https://github.com/crankyoldgit/IRremoteESP8266
