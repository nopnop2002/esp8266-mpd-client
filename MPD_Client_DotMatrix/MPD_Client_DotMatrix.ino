/*
 *  MPD Client Example for ESP8266/ESP32
 *
 *  LED Matrix Pin -> ESP8266 Pin
 *  Vcc            -> 3v
 *  Gnd            -> Gnd
 *  DIN            -> D7
 *  CS             -> D8
 *  CLK            -> D5
 */
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else        
#include <WiFi.h>
#endif

#include <SPI.h>
#include <Adafruit_GFX.h> // Install from Library Manager
#include <Max72xxPanel.h> // https://github.com/markruys/arduino-Max72xxPanel

int pinCS = 15; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

int wait = 100; // In milliseconds
int spacer = 1;
int width = 5 + spacer; // The font width is 5 pixels

char* ssid = "Your AP's SSID";
char* password = "Your AP's PASSWORD";
uint16_t port = 6600;
char * host = "192.168.10.12"; // ip or dns

// Use WiFiClient class to create TCP connections
WiFiClient client;

long lastMillis = 0;
int interval = 0;

int mpc_connect(char * host, int port) {
  char smsg[40];
  char rmsg[40];

  if (!client.connect(host, port)) {
      Serial.println("connection failed");
      return 0;
  }

  String line;
  client.setTimeout(1000);
  line = client.readStringUntil('\0');
  //Serial.print("[");
  //Serial.print(line);
  //Serial.println("]");
  //Serial.println("length()=" + String(line.length()));
  line.toCharArray(rmsg, line.length()+1);
  //Serial.println("strlen()=" + String(strlen(rmsg)));
  rmsg[line.length()-1] = 0;
  Serial.println("rmsg=[" + String(rmsg) + "]");
  if (strncmp(rmsg,"OK",2) == 0) return 1;
  return 0;
}

int mpc_command(char * buf) {
  char smsg[40];
  char rmsg[40];
  sprintf(smsg,"%s\n",buf);
  client.print(smsg);
  Serial.println("smsg=[" + String(buf) + "]");

  String line;
  client.setTimeout(1000);
  line = client.readStringUntil('\0');
  //Serial.print("[");
  //Serial.print(line);
  //Serial.println("]");
  //Serial.println("length()=" + String(line.length()));
  line.toCharArray(rmsg, line.length()+1);
  //Serial.println("strlen()=" + String(strlen(rmsg)));
  rmsg[line.length()-1] = 0;
  Serial.println("rmsg=[" + String(rmsg) + "]");
  if (strcmp(rmsg,"OK") == 0) return 1;
  return 0;
}

void mpc_error(char * buf) {
  Serial.print("mpc command error:");
  Serial.println(buf);
  while(1) {}
}


int getItem(String line, char * item, char * value, int len) {
  int pos1,pos2,pos3;
  Serial.println("item=[" + String(item) + "]");
  pos1=line.indexOf(item);
  //Serial.println("pos1=" + String(pos1));
  String line2;
  line2 = line.substring(pos1);
  pos2=line2.indexOf(":");
  pos3=line2.indexOf(0x0a);
  //Serial.println("pos2=" + String(pos2));
  //Serial.println("pos3=" + String(pos3));
  String line3;
  line3 = line2.substring(pos2+1,pos3);
  //Serial.println("line3=[" + line3 + "]");
  string2char(line3, value, len);
  Serial.println("value=[" + String(value) + "]");
  return(strlen(value));
}

void string2char(String line, char * cstr4, int len) {
  char cstr3[40];
  line.toCharArray(cstr3, line.length()+1);
  //Serial.println("cstr3=[" + String(cstr3) + "]");
  int pos4 = 0;
  for (int i=0;i<strlen(cstr3);i++) {
    //if (cstr3[i] == ' ') continue;
    if (cstr3[i] == ' ' && pos4 == 0) continue;
    cstr4[pos4++] = cstr3[i];
    cstr4[pos4] = 0;
    if (pos4 == (len-1)) break;
  }
  //Serial.println("cstr4=[" + String(cstr4) + "]");
}

/*
 * Reference:
 * https://earthbondhon.com/8x32-led-matrix-max7219-tutorial-with-scrolling-text/
 */
void matrixDisplay(char * lcdbuf, int lcdlen) {
  //for ( int i = 0 ; i < width * lcdlen + matrix.width() - 1 - spacer; i++ ) {
  for ( int i = 0 ; i < width * lcdlen + matrix.width() - spacer; i++ ) {
    //Serial.println("i=" + String(i));
    matrix.fillScreen(LOW);
    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2; // center the text vertically
    while ( x + width - spacer >= 0 && letter >= 0 ) {
      //Serial.println("letter=" + String(letter));
      if ( letter < lcdlen ) {
        matrix.drawChar(x, y, lcdbuf[letter], HIGH, LOW, 1);
      }
      letter--;
      x -= width;
    }
    matrix.write(); // Send bitmap to display
    delay(wait);
  }
}

void setup() {
  Serial.begin(115200);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Wait for WiFi...");
  WiFi.begin(ssid, password);

  int cnt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    cnt++;
    if ((cnt % 60) == 0) Serial.println();
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  while(1) {
    Serial.print("connecting to ");
    Serial.println(host);
  //if (mpc_connect(host, port) == 0) mpc_error("connect");
    if (mpc_connect(host, port) == 1) break;
    delay(10*1000);
  }

  // set up the 8×32 LED Matrix MAX7219
  matrix.setIntensity(7); // Use a value between 0 and 15 for brightness
  
  matrix.setPosition( 0, 0, 0) ;  // The first display is at <0, 0>
  matrix.setPosition( 1, 1, 0) ;  // The second display is at <1, 0>
  matrix.setPosition( 2, 2, 0) ;  // The third display is in <2, 0>
  matrix.setPosition( 3, 3, 0) ;  // The fourth display is at <3, 0>
  
  matrix.setRotation(0, 1); // The first display is position upside down
  matrix.setRotation(1, 1); // The first display is position upside down
  matrix.setRotation(2, 1); // The first display is position upside down
  matrix.setRotation(3, 1); // The first display is position upside down

  matrix.fillScreen(LOW);
}

void loop() {
  static int counter = 0;
  String line;
  char state[40];
  char smsg[40];
  char lcdbuf[80] = {0};
  static char oldbuf[80] = {0};
  static int offset = 0;
  
  if (!client.connected()) {
    Serial.println("server disconencted");
    delay(10*1000);
    ESP.restart();
  }

  long now = millis();
  if (now < lastMillis) lastMillis = now; // millis is overflow
  if (now - lastMillis > 1000) {
    lastMillis = now;
    counter++;
    if (counter > interval) {
      sprintf(smsg,"status\n");
      client.print(smsg);
      Serial.println("status");
    
      //read back one line from server
      client.setTimeout(1000);
      line = client.readStringUntil('\0');
      //Serial.println("status=[" + line + "]");
      Serial.println("state=" + String(getItem(line, "state:", state, sizeof(state))) );

      if (strcmp(state,"play") == 0) {
        sprintf(smsg,"currentsong\n");
        client.print(smsg);
        //read back one line from server
        client.setTimeout(1000);
        line = client.readStringUntil('\0');
        Serial.println("currentsong=[" + line + "]");

        char artist[40];
        char title[40];
        int artistLen;
        int titleLen;
        artistLen = getItem(line, "Artist:", artist, sizeof(artist));
        Serial.println("Artist=" + String(artistLen));
        titleLen = getItem(line, "Title:", title, sizeof(title));
        Serial.println("Title=" + String(titleLen));
        memset(lcdbuf, 0, sizeof(lcdbuf));
        if (artistLen > 0 && titleLen > 0) {
          strcpy(lcdbuf, artist);
          strcat(lcdbuf, " - ");
          strcat(lcdbuf, title);
        } else if (artistLen > 0 && titleLen == 0) {
          strcpy(lcdbuf, artist);
        } else if (artistLen == 0 && titleLen > 0) {
          strcpy(lcdbuf, title);
        }

        Serial.println("lcdbuf=[" + String(lcdbuf) + "]");
        if (strlen(lcdbuf) > 0) {
          matrixDisplay(lcdbuf, strlen(lcdbuf));
          strcpy(oldbuf, lcdbuf);
        }
        interval = 0;

      }  else {  // state = stop
        matrix.fillScreen(LOW);
        memset(oldbuf, 0, sizeof(oldbuf));
        interval = 1;
      }
      counter=0;
    }
  }
}
