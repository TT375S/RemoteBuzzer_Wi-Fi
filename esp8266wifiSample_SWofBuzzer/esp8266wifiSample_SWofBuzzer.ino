#include <ESP8266WiFi.h>

const char* ssid     = "aterm-67d898-g";
const char* password = "4c29770bd0558";
const int buzzerPin = 0;
const int LEDPin = 14;
//ブザーがオンか否か。0==OFF
int state = 0;
//チャタリング防止に使う。時間を記録する
int ct, pt;

//ブザーのIPアドレス
const char* host = "192.168.0.4";
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

  // We start by connecting to a WiFi network

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
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void changeState() {
  ct = millis();
  //チャタリング防止
  if (ct - pt > 200) {
    if (state == 0) {
      state = 1;
      digitalWrite(LEDPin, HIGH);
    }
    else {
      state = 0;
      digitalWrite(LEDPin, LOW);
    }
  }
  pt = millis();
}

void loop() {
  delay(2000);
  if (state == 0) {
    Serial.println("SW is False.");
  }
  else {
    Serial.println("SW is True.");
  }
  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // We now create a URI for the request
  String url;
  if (state == 1)
    url = "/gpio/1";
  else url = "/gpio/0";

  temp = (temp + 1) % 2;

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  int timeout = millis() + 5000;
  while (client.available() == 0) {
    if (timeout - millis() < 0) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");
}
