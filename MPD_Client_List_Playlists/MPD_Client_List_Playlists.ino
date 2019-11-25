/*
 *  MPD Client Example for ESP8266/ESP32
 */

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else        
#include <WiFi.h>
#endif

char* ssid = "Your AP's SSID";
char* password = "Your AP's PASSWORD";
uint16_t port = 6600;
char * host = "192.168.10.45"; // ip or dns

// Use WiFiClient class to create TCP connections
WiFiClient client;

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

int getItems(String line, int start, char * item, char * value, int len) {
  int pos1,pos2,pos3;
  Serial.println("item=[" + String(item) + "]");
  pos1=line.indexOf(item, start);
  //Serial.println("pos1=" + String(pos1));
  if (pos1 < 0) return(pos1);
  
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
  //return(strlen(value));
  return(pos1+pos3+1);
}

void string2char(String line, char * cstr4, int len) {
  char cstr3[256];
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


  Serial.print("connecting to ");
  Serial.println(host);
  if (mpc_connect(host, port) == 0) mpc_error("connect");
 
  String line;
  char smsg[40];
  sprintf(smsg,"listplaylists\n");
  client.print(smsg);
  Serial.println("listplaylists");

  //read back one line from server
  client.setTimeout(1000);
  line = client.readStringUntil('\0');
  Serial.println("line=[" + line + "]");

  //show playlist
  int offset = 0;
  char playlist[128];
  while(1) {
    offset = getItems(line, offset, "playlist:", playlist, sizeof(playlist));
    if (offset < 0) break;
    //Serial.print("playlist=");
    //Serial.println(playlist);
  }
}


void loop() {

}
