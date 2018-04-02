/*
 *  MPD Client Example for ESP8266
 */

#include <ESP8266WiFi.h>

const char* ssid = "Your AP's SSID";
const char* password = "Your AP's PASSWORD";

// Use WiFiClient class to create TCP connections
WiFiClient client;

int mpc_connect(char * host, int port) {
  char smsg[40];
  char rmsg[40];

  if (!client.connect(host, port)) {
      Serial.println("connection failed");
      Serial.println("wait 5 sec...");
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


int getStatusItem(String line, char * item, char * value) {
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
  string2char(line3,value);
  Serial.println("value[" + String(value) + "]");
  return(strlen(value));
}

void string2char(String line, char * cstr4) {
  char cstr3[40];
  line.toCharArray(cstr3, line.length()+1);
  //Serial.println("cstr3=[" + String(cstr3) + "]");
  //char cstr4[40];
  int pos4 = 0;
  for (int i=0;i<strlen(cstr3);i++) {
    if (cstr3[i] == ' ') continue;
    cstr4[pos4++] = cstr3[i];
    cstr4[pos4] = 0;
  }
  //Serial.println("cstr4=[" + String(cstr4) + "]");
  
}

 
void setup() {
  Serial.begin(9600);

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

  delay(500);

  uint16_t port = 6600;
  char * host = "192.168.10.40"; // ip or dns
  static int number=0;
  
  Serial.print("connecting to ");
  Serial.println(host);

  if (mpc_connect(host, port) == 0) mpc_error("connect");

  String line;
  char citem[40];
  char smsg[40];

  sprintf(smsg,"status\n");
  client.print(smsg);
  Serial.println("status");
  
  //read back one line from server
  client.setTimeout(1000);
  line = client.readStringUntil('\0');
  //Serial.println("[" + line + "]");

  Serial.println("state=" + String(getStatusItem(line, "state:", citem)) );

  if (strcmp(citem,"stop") == 0) {
    if (mpc_command("play") == 0) mpc_error("play");
  } else {
    if (mpc_command("stop") == 0) mpc_error("stop");
  }
  
  sprintf(smsg,"status\n");
  client.print(smsg);
  Serial.println("status");

  //read back one line from server
  client.setTimeout(1000);
  line = client.readStringUntil('\0');
  //Serial.println("[" + line + "]");

  Serial.println("state=" + String(getStatusItem(line, "state:", citem)) );
  Serial.println("volume=" + String(getStatusItem(line, "volume:", citem)) );

}


void loop() {
}
