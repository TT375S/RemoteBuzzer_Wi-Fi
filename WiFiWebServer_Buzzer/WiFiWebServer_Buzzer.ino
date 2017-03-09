/*
 *  This sketch demonstrates how to set up a simple HTTP-like server.
 *  The server will set a GPIO pin depending on the request
 *    http://server_ip/gpio/0 will set the GPIO2 low,
 *    http://server_ip/gpio/1 will set the GPIO2 high
 *  server_ip is the IP address of the ESP8266 module, will be 
 *  printed to Serial when the module is connected.
 */

#include <ESP8266WiFi.h>

//const char* ssid     = "pr500k-c374e6-1";
//const char* password = "272cb07923209";
const char* ssid     = "aterm-67d898-g";
const char* password = "4c29770bd0558";

//gpioNumber. default is 2.
const int pinnum = 14;

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(10);

  // prepare GPIO2
  pinMode(pinnum, OUTPUT);
  digitalWrite(pinnum, 0);
  
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}


void play(int buzzer, int BEAT) {
  int i = 0;
  const int score[] = {262, 294, 330, 349, 392, 440, 494, 523, 0};
  //analogWrite(buzzer, 500) ;

  for (i = 0; i < 8; i++) { 
    tone(buzzer, score[i]);
    delay(BEAT);
  }
  noTone(buzzer);
}

int val=0;
void loop() {
      // Set GPIO2 according to the request
  if(val)play(pinnum, 300);
  
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {

    return;
  }
  
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
  
  // Match the request
  
  if (req.indexOf("/gpio/0") != -1)
    val = 0;
  else if (req.indexOf("/gpio/1") != -1)
    val = 1;
  //Get state
  else if(req.indexOf("/state") != -1)
    val = val;
  else {
    Serial.println("invalid request");
    client.stop();
    return;
  }


  
  client.flush();

  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nGPIO is now ";
  s += (val)?"high":"low";
  s += "</html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}

