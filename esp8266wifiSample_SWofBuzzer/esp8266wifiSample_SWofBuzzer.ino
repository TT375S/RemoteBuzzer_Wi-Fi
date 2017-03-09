/*
 *  This sketch demonstrates how to set up a simple HTTP-like server.
 *  The server will set a GPIO pin depending on the request
 *    http://server_ip/gpio/0 will set the GPIO2 low,
 *    http://server_ip/gpio/1 will set the GPIO2 high
 *  server_ip is the IP address of the ESP8266 module, will be 
 *  printed to Serial when the module is connected.
 */
#include <ESP8266WiFi.h>

const char* ssid     = "aterm-67d898-g";
const char* password = "4c29770bd0558";
const int buzzerPin = 0;
const int LEDPin = 14;
const int chatteringIgnoreTime = 200;
//const int stateResetTime = 1000*180;
//ブザーがオンか否か。0==OFF
int state = 0;
//チャタリング防止に使う。時間を記録する
int ct, pt;
int stateOnTime=0;

int val = 0;
//ブザーのIPアドレス
const char* host = "192.168.0.25";

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

int temp = 0;
void setup() {
  pinMode(LEDPin, OUTPUT);
  //スイッチのセットアップ
  pinMode(buzzerPin, INPUT);
  ct = pt = 0;
  
  //割り込み準備
  attachInterrupt(0, changeState, FALLING);
  
  //以下はネットワーク接続のセットアップ
  Serial.begin(115200);
  delay(10);
  
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

void changeState() {
  //最新の押した時刻
  ct = millis();
  //チャタリング防止。現在-前回の時刻が小さければ、チャタリングだとして無視する
  if (ct - pt > chatteringIgnoreTime) {
    if (state == 0) {
      state = 1;
      digitalWrite(LEDPin, HIGH);
    }
    else {
      state = 0;
      digitalWrite(LEDPin, LOW);
    }
  }
  //前回押された時刻
  pt = millis();
}

void flashingLED(const int ledpin){
  int i;
  for(i=0; i<8; i++){
    digitalWrite(ledpin, HIGH);
    delay(250);
    digitalWrite(ledpin, LOW);
    delay(250);
  }
  
}

void loop() {
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
  ///gpio/0と1は、スイッチのステートを強制的に変更するデバッグ用
  if (req.indexOf("/gpio/0") != -1){
    val = 0;
    state = 0;
    flashingLED(LEDPin);
    digitalWrite(LEDPin, LOW);
  }
  else if (req.indexOf("/gpio/1") != -1){
    val = 1;
    state = 1;
    flashingLED(LEDPin);
    digitalWrite(LEDPin, HIGH);
  }
  //Get state
  else if(req.indexOf("/state") != -1){
    val = state;
  }
  else {
    Serial.println("invalid request");
    client.stop();
    return;
  }
  
  client.flush();

  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nGPIO is now ";
  s += (val)?"high":"low";
  s += "\r\n</html>\r\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is destroyed
}
