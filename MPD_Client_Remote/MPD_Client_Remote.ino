/*
 *  MPD Client Example for ESP8266/ESP32
 *
 *  Required library
 *  https://github.com/crankyoldgit/IRremoteESP8266
 *   
 */

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

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

  irrecv.enableIRIn();  // Start IR receiver
  Serial.print("IRrecvDemo is now running and waiting for IR message on Pin ");
  Serial.println(RECV_PIN);
}


void loop() {

  if (!client.connected()) {
    Serial.println("server disconencted");
    delay(10*1000);
    ESP.restart();
  }

  
  if (irrecv.decode(&results)) {
    serialPrintUint64(results.value, HEX);
    Serial.println("");
    if (results.value == PLAY) {
        if (mpc_command("play") == 0) mpc_error("play");
    } else if (results.value == STOP) {
        if (mpc_command("stop") == 0) mpc_error("stop");
    } else if (results.value == NEXT) {
        if (mpc_command("play") == 0) mpc_error("play");  // Start if stopped
        if (mpc_command("next") == 0) mpc_error("next");
        if (mpc_command("play") == 0) mpc_error("play");
    } else if (results.value == PREV) {
        if (mpc_command("play") == 0) mpc_error("play");  // Start if stopped
        if (mpc_command("previous") == 0) mpc_error("previous");
        if (mpc_command("play") == 0) mpc_error("play");
    } else if (results.value == VOLU) {
        if (mpc_command("volume +10") == 0) mpc_error("volume");
    } else if (results.value == VOLD) {
        if (mpc_command("volume -10") == 0) mpc_error("volume");
    }
    irrecv.resume();  // Receive the next value
  }

  // Disconnected after 60 seconds of no communication
  long now = millis();
  if (now < lastMillis) lastMillis = now; // millis is overflow
  if (now - lastMillis > 10000) {
    lastMillis = now;
    if (mpc_command("ping") == 0) mpc_error("ping");
  }
} 
