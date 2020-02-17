#include <WiFi.h>
#include <EEPROM.h>

String commandBuffer = "";
String SSID;
String PASS;
String HOST;
String PORT;

const int SSID_ADDR = 0;
const int PASS_ADDR = 31;

void setup() {

  pinMode(21, OUTPUT);
  EEPROM.begin(512);
  
  // Configure Serial
  Serial.begin(9600); // No need to be fast
  while(!Serial); // Wait for connection.

  // Clear garbage from terminal
  Serial.write(27);
  Serial.print("[2J");
  Serial.write(27);
  Serial.print("[H");

  // Load SSID and PASS from EEPROM
  SSID = eepromRead(SSID_ADDR);
  PASS = eepromRead(PASS_ADDR); // This max length is just a guess

  listHelp();
  Serial.print("\n>");
}

void loop() {
  handelSerial();
}

// This really isn't necessary for the project but may be useful for debugging
void handelSerial() {
  while (Serial.available() > 0) {
    char tmp = Serial.read();
    Serial.print(tmp);
    if (tmp == 127) {
      commandBuffer.remove(commandBuffer.length() - 1, 1);
      // I couldn't easily find the escape sequence to delete a character
      Serial.write(27);
      Serial.print("[1D");
      Serial.write(27);
      Serial.print("[K");
    } else if (tmp != 13) {
      commandBuffer += tmp;
    } else {
      Serial.print('\n');
      // TODO: Replace with non-blocking (Interrupt timer for led flashes?)
      digitalWrite(21, HIGH);
      delay(200);
      digitalWrite(21, LOW);
      
      if (commandBuffer == "scan") {
        listNetworks();
      } else if (commandBuffer == "help") {
        listHelp();
      } else if (commandBuffer == "show") {
        showVars();
      } else if (commandBuffer == "connect") {
        networkConnect();
      } else if (commandBuffer.indexOf("sethost") >= 0) {
        HOST = commandBuffer.substring(8);
      } else if (commandBuffer.indexOf("setport") >= 0) {
        PORT = commandBuffer.substring(8);
      } else if (commandBuffer.indexOf("setpass") >= 0) {
        eepromWrite(PASS_ADDR, commandBuffer.substring(8));
        PASS = eepromRead(PASS_ADDR);
      } else if (commandBuffer.indexOf("setssid") >= 0) {
        eepromWrite(SSID_ADDR, commandBuffer.substring(8));
        SSID = eepromRead(SSID_ADDR);
      } else if (commandBuffer.indexOf("get") >= 0) {
        Serial.println(networkGet(commandBuffer.substring(4)));
      } else {
        Serial.println("Invalid Command");
      }
      commandBuffer = "";
      Serial.print("\n>");
    }
  }
}

void listHelp() {
  Serial.println("Available Commands:");
  Serial.println("\tscan      List available networks");
  Serial.println("\thelp      Show this list");
  Serial.println("\tsetssid   Set SSID");
  Serial.println("\tsetpass   Set Password");
  Serial.println("\tsethost   Set host to connect to");
  Serial.println("\tsetport   Set port to connect to");
  Serial.println("\tshow      Show set variables");
  Serial.println("\tconnect   Connect to set nework using set password");
  Serial.println("\tget       Send get request to URL");
}

void listNetworks() {
  // List available network
  byte numSsid = WiFi.scanNetworks();
  Serial.println("Found " + String(numSsid) + " Networks:");
  Serial.println("\t|              SSID              |TYPE|STENGTH|");
  Serial.println("\t|--------------------------------|----|-------|");
  for (int i = 0; i < numSsid; i ++) {
    char out[47];

    // SSID needs to be converted to char type
    String s = WiFi.SSID(i);
    char ssid[32];
    s.toCharArray(ssid, 32);

    int type = WiFi.encryptionType(i);
    int strength = WiFi.RSSI(i);
    
    sprintf(out, "|%32s|", ssid);
    Serial.println("\t" + String(out) + "   " + String(type) + "|    " + String(strength) + "|");
  }
}

String networkGet(String url) {
  WiFiClient client;
  char host_char[HOST.length()+1];
  HOST.toCharArray(host_char, HOST.length()+1);
  Serial.println(host_char);
  
  Serial.println("Connecting To " + HOST + url);
  client.connect("www.google.com", 80);
  client.println("GET " + url);
  client.println("Host: " + HOST);
  client.println("Connection: close");
  client.println();

  Serial.println("");
  while (client.available()) {
    Serial.write(client.read());
  }
  Serial.println("\n\rDisconnecting from " + HOST);
  client.stop();
}

void networkConnect() {
  if (WiFi.status() != WL_CONNECTED) {
    char s[32], p[32];
    SSID.toCharArray(s, 32);
    PASS.toCharArray(p, 32);
    WiFi.begin(s, p);
    Serial.print("Connecting to " + SSID);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print('.');
      delay(500);
    }
    Serial.print("\n\r");
  }
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void showVars() {
  Serial.println("Showing Variables:");
  Serial.println("\tSSID   " + SSID);
  Serial.println("\tPASS   " + PASS);
  Serial.println("\tHOST   " + HOST);
  Serial.println("\tPORT   " + PORT);
}

void eepromWrite(int addr, String str) {
  char tmp[32];
  str.toCharArray(tmp, 32);
  for (int i = 0; i < 32; i++) {
    EEPROM.write(addr+i, tmp[i]);
  }
  EEPROM.commit();
}

String eepromRead(int addr) {
  char tmp[32];
  for (int i = 0; i < 32; i++) {
    tmp[i] = EEPROM.read(addr+i);
  }
  return String(tmp);
}
