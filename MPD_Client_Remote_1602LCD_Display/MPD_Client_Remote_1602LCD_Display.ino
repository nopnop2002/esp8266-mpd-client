/*
 *  MPD Client Example for ESP8266/ESP32
 *
 *  Required library
 *  https://github.com/crankyoldgit/IRremoteESP8266
 *  
 *  1602LCD ---- WeMos
 *   GND    ---- GND
 *   VCC    ---- 5V
 *   VE     ---- Potentiometer Resistor
 *   RS     ---- D1(GPIO5)
 *   R/W    ---- GND
 *   Enable ---- D2(GPIO4)
 *   DB0    ---- N.C
 *   DB1    ---- N.C
 *   DB2    ---- N.C
 *   DB3    ---- N.C
 *   DB4    ---- D0(GPIO16)
 *   DB5    ---- D5(GPIO14)
 *   DB6    ---- D6(GPIO12)
 *   DB7    ---- D7(GPIO13)
 *   LED(+) ---- 5V
 *   
 *                            ---- Transistor(2SC1815) 
 *   LED(-) ----              ---- Collector 
 *               GND          ---- Emitter
 *               D8(GPIO15)   ---- Base  
 *
 */
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else        
#include <WiFi.h>
#endif

#include "SPI.h"
#include <LiquidCrystal.h>

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

/* LiquidCrystal lcd(rs, enable, d4, d5, d6, d7) */
LiquidCrystal lcd(5, 4, 16, 14, 12, 13);
/* the number of columns that the display has */
#define lcdCols 16
/* the number of rows that the display has */
#define lcdRows 2
//#define lcdRows 4
/* LDC BackLight Control */
#define BackLight 15

// It needs to match your remote control.
#define PLAY 0x00FF18E7
#define STOP 0x00FF38C7
#define PREV 0x00FF10EF
#define NEXT 0x00FF5AA5
#define VOLU 0x00FF52AD
#define VOLD 0x00FF42BD

// an IR detector/demodulator is connected to GPIO pin 2
const uint16_t RECV_PIN = 2;

IRrecv irrecv(RECV_PIN);

decode_results results;

char* ssid = "Your AP's SSID";
char* password = "Your AP's PASSWORD";
uint16_t port = 6600;
char * host = "192.168.10.109"; // ip or dns

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

void fillBuffer(char * line, int len){
  int sz = strlen(line);
  for (int i=sz;i<len;i++) {
    line[i] = 0x20;
    line[i+1] = 0;
  }
}

 void lcdDisplay(char * lcdbuf, int rows) {
  char line[17];
  memset(line, 0, sizeof(line));
  strncpy(line, lcdbuf, 16);
  fillBuffer(line, 16);
  Serial.println("line1=[" + String(line) + "]");
  lcd.setCursor(0, 0);
  lcd.print(line);

  if (strlen(lcdbuf) > 16) {
    strncpy(line, &lcdbuf[16], 16);
  } else {
    strcpy(line, " ");
  }
  fillBuffer(line, 16);
  Serial.println("line2=[" + String(line) + "]");
  lcd.setCursor(0, 1);
  lcd.print(line);

  if (rows == 2) return;
  if (strlen(lcdbuf) > 32) {
    strncpy(line, &lcdbuf[32], 16);
  } else {
    strcpy(line, " ");
  }
  fillBuffer(line, 16);
  Serial.println("line3=[" + String(line) + "]");
  lcd.setCursor(0, 2);
  lcd.print(line);

  if (strlen(lcdbuf) > 48) {
    strncpy(line, &lcdbuf[48], 16);
  } else {
    strcpy(line, " ");
  }
  fillBuffer(line, 16);
  Serial.println("line4=[" + String(line) + "]");
  lcd.setCursor(0, 3);
  lcd.print(line);
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

  // set up the LCD's number of columns and rows:
  lcd.begin(lcdCols, lcdRows);
  pinMode(BackLight, OUTPUT);

  Serial.println();
  irrecv.enableIRIn();  // Start the receiver
  Serial.print("IRrecvDemo is now running and waiting for IR message on Pin ");
  Serial.println(RECV_PIN);

}


void loop() {
  static int counter = 0;
  String line;
  char state[40];
  char smsg[40];
  char lcdbuf[80] = {0};
  static char oldbuf[80] = {0};
  
  if (!client.connected()) {
    Serial.println("server disconencted");
    lcd.clear();
    digitalWrite(BackLight, LOW);
    delay(10*1000);
    ESP.restart();
  }

  if (irrecv.decode(&results)) {
    serialPrintUint64(results.value, HEX);
    Serial.println("");
    if (results.value == PLAY) {
        if (mpc_command("play") == 0) mpc_error("play");
        interval = lastMillis = 0;
    } else if (results.value == STOP) {
        if (mpc_command("stop") == 0) mpc_error("stop");
        interval = lastMillis = 0;
    } else if (results.value == NEXT) {
        if (mpc_command("play") == 0) mpc_error("play");  // Start if stopped
        if (mpc_command("next") == 0) mpc_error("next");
        if (mpc_command("play") == 0) mpc_error("play");
        interval = lastMillis = 0;
    } else if (results.value == PREV) {
        if (mpc_command("play") == 0) mpc_error("play");  // Start if stopped
        if (mpc_command("previous") == 0) mpc_error("previous");
        if (mpc_command("play") == 0) mpc_error("play");
        interval = lastMillis = 0;
    } else if (results.value == VOLU) {
        if (mpc_command("volume +10") == 0) mpc_error("volume");
    } else if (results.value == VOLD) {
        if (mpc_command("volume -10") == 0) mpc_error("volume");
    }
    irrecv.resume();  // Receive the next value
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
          if (strcmp(lcdbuf, oldbuf) != 0) {
            digitalWrite(BackLight, HIGH);
            lcdDisplay(lcdbuf, lcdRows);
            strcpy(oldbuf, lcdbuf);
          }
        }
        interval = 1;

      }  else {  // state = stop
        lcd.clear();
        digitalWrite(BackLight, LOW);
        memset(oldbuf, 0, sizeof(oldbuf));
        interval = 10;
      }
      counter=0;
    }
  }
}
