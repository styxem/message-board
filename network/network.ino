#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <Ticker.h>

// Constants for connecting to Network and Google
#define SSID "WIFIE33AE7"
#define PASS "LYR6J71MLNGRDAWH"
#define IMAP_HOST "imap.gmail.com"
#define IMAP_PORT 993
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

// Misc Constants (Magic Numbers should be avoided)
#define DEBUG true
#define SLEEP_TIME 60
#define EMAIL_FILTER "ohio.edu"
#define DEBUG_EMAIL "pi440113@ohio.edu"

// Login Constants
#define USERNAME "ou.messageboard"
#define PASSWORD "eecsBobcats"
#define USERNAME_BASE64 "b3UubWVzc2FnZWJvYXJk"
#define PASSWORD_BASE64 "ZWVjc0JvYmNhdHM="

// Object that will do the connecting
WiFiClientSecure client;

// Variables to store the received messages
int messages_length = 0;
String* messages;

// Timers
Ticker ticker_cycle;

// Confiure WiFi connection and connect to network
// This function blocks until a connection has been established
void network_connect() {
  if (WiFi.status() != WL_CONNECTED) {

    //Attempt to Connect
    WiFi.begin(SSID, PASS);
    if (DEBUG) {
      Serial.print("Connecting to: ");
      Serial.print(SSID);
      Serial.print(" ");
    }

    while (WiFi.status() != WL_CONNECTED) {
      if (DEBUG)
	      Serial.print('.');
      delay(500);
    }

    if (DEBUG) {
      Serial.print("\n\r");
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
    }
  }
}

// Close connection to WiFi
void network_close() {
  //WiFi.stop();
}

// Establish a telnet-like connection to remote server using WiFiClientSecure
// Non-Blocking
void telnet_connect(char* h, int p) {

  if (DEBUG)  {
    Serial.print("Connecting to: ");
    Serial.print(h);
    Serial.print(" @ ");
    Serial.println(p);
  }

  bool status = client.connect(h, p);

  if (DEBUG) {
    if (status)
      Serial.println("Connection Successfull\n");
    else
      Serial.println("Connection Failed\n");
  }

}

// Close connection to remote host
// Non-Blocking
void telnet_close() {
  client.stop();
}

// Send data to remote host
// Returns responce from remote host
// Non-Blocking(?)
String telnet_send(String data) {
  if (DEBUG)
    Serial.println("Sending data to remote host:\n\t" + data + "\n");

  client.println(data);
  

  // Grab data and keep waiting if more is comming in
  String resp;
  int len = 0;
  do {
    len = resp.length();
    delay(250);
    while (client.available()) {
      resp += (char)client.read();
    }
  } while(len != resp.length());

  if (DEBUG) {
    Serial.println("Host Reponse: [" + String(resp.length()) + "]");
    Serial.println("================================================");
    Serial.print(resp);
    Serial.println("================================================\n");
  }

  return resp;
}

// This function here has taught me why so many people hate c/c++
void parse_message_list(int*& message_list, int& message_list_length, String resp) {
  int line_start = resp.indexOf("* SEARCH");
  String sub_str = resp.substring(line_start + 9, resp.indexOf("\n", line_start));
  message_list_length = 0;
  
  if (sub_str.length() > 0) {
    message_list_length = 1;

    // Find how many numbers were returned
    for(int i = 0; i < sub_str.length(); i++) {
      if (sub_str.charAt(i) == ' ')
        message_list_length++;
    }
  }
  message_list = new int[message_list_length];

  // Add numbers to the list
  for(int i = 0; i < message_list_length; i++) {
    if (i == message_list_length - 1) {
      message_list[i] = sub_str.toInt(); 
    } else {
      message_list[i] = sub_str.substring(0, sub_str.indexOf(" ", 1)).toInt();
    }
    sub_str = sub_str.substring(sub_str.indexOf(" ", 1));
  }      

  if (DEBUG) {
    Serial.print("Message List Length: ");
    Serial.println(message_list_length);
    Serial.print("Message List: ");
    for(int i = 0; i < message_list_length; i++) {
      Serial.print(message_list[i]);
      Serial.print(" ");
    }
    Serial.println("\n");
  }
}

// Here we are getting the message and necessary headers through two different queries
// This could probably be done with just one query or we could parse the whole message
// with all the headers but that is difficult to do with the limited string functions
// in the arduino language and this is just a prototype so why make our lives hard?
String parse_message(int message_number) {
  // Get Subject and date/time
  String headers = telnet_send("a1 FETCH " + String(message_number) + " BODY[HEADER.FIELDS (SUBJECT DATE)]");
  // Pull out the subject
  int subject_position = headers.indexOf("Subject:") + 9; // 9 is the length of Subject: and a space
  String subject = headers.substring(subject_position, headers.indexOf("\n", subject_position));
  int date_position = headers.indexOf("Date:") + 6; // 6 is the length of Date: and a space
  String date = headers.substring(date_position, headers.indexOf("\n", date_position));

  // Get the text/plain part of the message
  String text_plain = telnet_send("a1 FETCH " + String(message_number) + " BODY[1]");
  // The body does not have a nice prefix, so we take a substring between the first and
  // second newline
  int newline_position = text_plain.indexOf("\n") + 1; // 1 is the length of '\n' 
  String body = text_plain.substring(newline_position, text_plain.indexOf("\n", newline_position));

  if (DEBUG)
    Serial.println("Parsed Message:\n\t" + subject + " -- " + body + " -- " + date + "\n");

  return subject + "\n" + body + "\n" + date;
}

// Check Email
// Updates list of messages at pointer
// Updates number of messages in list
void check_email(String*& parsed_message_list, int& message_list_length) {
  telnet_connect(IMAP_HOST, IMAP_PORT);
  telnet_send("a1 LOGIN " + String(USERNAME) + " " + String(PASSWORD));
  telnet_send("a1 SELECT INBOX");

  int* message_list;
  parse_message_list(message_list, message_list_length, telnet_send("a1 SEARCH UNSEEN FROM \"" + String(EMAIL_FILTER) +"\""));
  parsed_message_list = new String[message_list_length];
  
  for (int i = 0; i < message_list_length; i++) {
    parsed_message_list[i] = parse_message(message_list[i]);
  }

  // Array needs to be deleted because it was created with new
  delete [] message_list;

  telnet_send("a1 LOGOUT");
}

// This method updates the globaly stored messages cache
void update_message_cache() {
  digitalWrite(13, HIGH);
  //network_connect(); // This does not like to work when called by the ticker

  // We don't need a memory leak
  delete [] messages;
  
  check_email(messages, messages_length);

  telnet_close();

  if (DEBUG) {
    for (int i = 0; i < messages_length; i++) {
      Serial.println("Message " + String(i) + " : " + "[" + messages[i].length() + "]");
      Serial.println("================================================");
      Serial.println(messages[i]);
      Serial.println("================================================\n");
    }
  }
  digitalWrite(13, LOW);
}

// This method sends emails
void send_email(String subject, String body, String rcpt) {
  telnet_connect(SMTP_HOST, SMTP_PORT);
  telnet_send("EHLO smtp.gmail.com");
  telnet_send("AUTH LOGIN");
  telnet_send(USERNAME_BASE64);
  telnet_send(PASSWORD_BASE64);
  telnet_send("MAIL FROM: <ou.messageboard@gmail.com>");
  telnet_send("rcpt to: <" + rcpt + ">");
  telnet_send("DATA");
  telnet_send("SUBJECT: " + subject);
  telnet_send("\n");
  telnet_send(body);
  telnet_send(".");
  telnet_send("quit");
  telnet_close();
}

// Where main code lives
// Called by ticker
void cycle() {
  update_message_cache();
  if (messages_length > 0)
    send_email("Test Email", messages[0], String(DEBUG_EMAIL));
}

void setup() {
  Serial.begin(115200);
  while(!Serial);

  // Clear garbage from terminal                                                                      
  Serial.write(27);                                                                                   
  Serial.print("[2J");                                                                                
  Serial.write(27);                                                                                   
  Serial.print("[H");

  pinMode(13, OUTPUT);
  network_connect();

  ticker_cycle.attach(SLEEP_TIME, cycle);
  
}

void loop() {
}
