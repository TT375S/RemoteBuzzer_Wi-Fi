//webserverという名前だけど、今はwebclientです。過去のバージョンではサーバーでした。

#include <ESP8266WiFi.h>
#include <string.h>

//const char* ssid     = "pr500k-c374e6-1";
//const char* password = "272cb07923209";

//const char* ssid     = "ESP_D59142";
//const char* password = "";
const char* ssid     = "aterm-67d898-g";
const char* password = "4c29770bd0558";

int val=0;
//gpioNumber. default is 2.
const int pinnum = 12;
const int ledPin = 14;
//電源入って一番最初の通信かどうか。これにより、ブザー側のリセットボタンを押した後の最初の通信で、スイッチ側もリセットします。
int isFirstConnect = 1;

//接続先のIPアドレス
//const char* host = "192.168.4.1";
const char* host = "192.168.0.31";

void setup() {
  Serial.begin(115200);
  delay(10);

  // prepare GPIO
  pinMode(pinnum, OUTPUT);
  digitalWrite(pinnum, 0);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, 0);
  
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
    // Set GPIO according to the request

      
  //毎秒、更新してると負担がかかるので何秒かごとにSWが押されてるかを確かめる
  digitalWrite(ledPin, HIGH);
  if(val==1)play(pinnum, 300);
  //一瞬だけパイロットランプ点灯のためのdelay(50)
  delay(50);
  digitalWrite(ledPin, LOW);
  //SWが押されてる時はループ中でもLEDつけておく
  if(val == 1)digitalWrite(ledPin, HIGH);
  delay(4000);
  

  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    //接続失敗はLEDピカピカで教える
    flashingLED(ledPin);
    return;
  }else{
    digitalWrite(ledPin, LOW);
  }

  // We now create a URI for the request
  String url;
//  if (state == 1)
//    url = "/gpio/1";
//  else url = "/gpio/0";

//いつもstateを聞くことにする
  url = "/state";
//最初の通信だけは、スイッチの値をリセットする。
  if(isFirstConnect == 1){
    isFirstConnect = 0;
    url = "/gpio/0";
    Serial.println("\r\nisFirstConnect!\r\n");
  }

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("waiting response...\r\n");
  int timeout = millis() + 5000;
  int stTime = millis();
  while (client.available() == 0) {
//    if (timeout - millis() < 0) {
//      Serial.println(">>> Client Timeout !");
//      client.stop();
//      return;
//    }
    if (millis()-stTime > 4000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
    delay(1);
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
    //何故か\rは無視されるけど\nはちゃんと入れないと認識してくれない。Arduinoは\rで改行？Cってそういうものなんだっけ。
    String target_high = "\nGPIO is now high";
    String target_low = "\nGPIO is now low";
    //SWのボタンが押されていればvalを1にしとく
    //全ての行を読むため、Highでないかどうかだけだと、他の行例えば\nを読んだ時にHighでないからLow！って判定しちゃうのでHighとLowは別々にちゃんと確かめる
    if(line == target_high){
        val = 1;
        Serial.print("\n    HIT!\n");
      }else if(line == target_low)
      {
        val = 0;
        Serial.print("\n    NOTHIT!\n");
      }
  }
  Serial.println();
  Serial.println("closing connection");
}

