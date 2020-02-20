#include <WiFiClientSecure.h>
#include <EEPROM.h>

String commandBuffer = "";
String SSID;
String PASS;
String HOST = "www.howsmyssl.com";
String PORT = "443";

const int SSID_ADDR = 0;
const int PASS_ADDR = 31;

const char* test_root_ca= \
     "-----BEGIN CERTIFICATE-----\n" \
     "MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/\n" \
     "MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n" \
     "DkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0Nlow\n" \
     "SjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMT\n" \
     "GkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOC\n" \
     "AQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EF\n" \
     "q6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8\n" \
     "SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0\n" \
     "Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWA\n" \
     "a6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj\n" \
     "/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0T\n" \
     "AQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIG\n" \
     "CCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNv\n" \
     "bTA7BggrBgEFBQcwAoYvaHR0cDovL2FwcHMuaWRlbnRydXN0LmNvbS9yb290cy9k\n" \
     "c3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAw\n" \
     "VAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcC\n" \
     "ARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAz\n" \
     "MDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwu\n" \
     "Y3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsF\n" \
     "AAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJo\n" \
     "uM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/\n" \
     "wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwu\n" \
     "X4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlG\n" \
     "PfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6\n" \
     "KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg==\n" \
     "-----END CERTIFICATE-----\n";

WiFiClientSecure client;

String eepromRead(int addr) {
  char tmp[32];
  for (int i = 0; i < 32; i++) {
    tmp[i] = EEPROM.read(addr+i);
  }
  return String(tmp);
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
void networkGet(String url) {
  client.setCACert(test_root_ca);
  char host_char[HOST.length()+1];
  HOST.toCharArray(host_char, HOST.length()+1);
  Serial.println(host_char);
  
  Serial.println("Connecting To " + HOST + " @ " + url);
  if(!client.connect(host_char, PORT.toInt())) {
    Serial.println("Connection Failed");
  } else {
    client.println("GET " + url + " HTTP/1.0");
    client.println("Host: " + HOST);
    client.println("Connection: close");
    client.println();

    Serial.println("");
    while (client.available()) {
      Serial.write(client.read());
    }
    Serial.println("\n\rDisconnecting from " + HOST);
  }
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
      digitalWrite(13, HIGH);
      delay(200);
      digitalWrite(13, LOW);
      
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
        networkGet(commandBuffer.substring(4));
      } else {
        Serial.println("Invalid Command");
      }
      commandBuffer = "";
      Serial.print("\n>");
    }
  }
}

void setup() {

  pinMode(13, OUTPUT);
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
