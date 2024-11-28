#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

#define D0    16
#define D1    5
#define D2    4
#define D3    0
#define D4    2
#define D5    14
#define D6    12
#define D7    13
#define D8    15
#define D9    3
#define D10   1
 
// WIFI
//const char* SSID = "iPhone de Gabriel Santos da";
//const char* PASSWORD = "gabriel02";

// const char* SSID = "DAYNIS - 2.4/5G";
// const char* PASSWORD = "batata11.11";

const char* SSID = "CARLOS";
const char* PASSWORD = "biel#2024";
const char* host = "receba-api-amhwetd4a7fgaafv.eastus-01.azurewebsites.net";
const int port = 80;
const int httpsPort = 443;
const char* serverURL = "/api/verifyLocker";
const String jwtToken = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJlbWFpbCI6ImJpZWwxNDAyMjAwMkBnbWFpbC5jb20iLCJ1bmlkYWRlcyI6W3siSWRVbmlkYWRlIjoxLCJJZENvbmRvbWluaW8iOjEsIkJsb2NvIjoiQSIsIkFwYXJ0YW1lbnRvIjoiMTAxIiwiTm9tZUNvbmRvbWluaW8iOiJDb25kb23DrW5pbyBKYXJkaW0gZGFzIEZsb3JlcyJ9LHsiSWRVbmlkYWRlIjoyLCJJZENvbmRvbWluaW8iOjEsIkJsb2NvIjoiQSIsIkFwYXJ0YW1lbnRvIjoiMTAyIiwiTm9tZUNvbmRvbWluaW8iOiJDb25kb23DrW5pbyBKYXJkaW0gZGFzIEZsb3JlcyJ9XSwiaWF0IjoxNzMyMzQyMzU3LCJleHAiOjE3NjM4NzgzNTd9.ol7Y-P-ZDGbMlCvA33-KB_Vnr7Neypap1Esw259Rpys";


WiFiClientSecure client;

//Prototypes
void initSerial();
void InitOutput(void);
void InitInput(void);
void initWiFi();
void reconectWiFi(); 

void setup(){
    initSerial();
    InitOutput();
    InitInput();
    initWiFi();
}

void initSerial(){
    Serial.begin(115200);
}

void InitOutput(void){
    pinMode(D8, OUTPUT); //Fechadura
    pinMode(D4, OUTPUT); //Led Red
    pinMode(D3, OUTPUT); //Led Green
    pinMode(D2, OUTPUT); //Alarm
}

void InitInput(void){
    pinMode(D1, INPUT_PULLUP);
}

void initWiFi(){
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
    reconectWiFi();
}
  
void reconectWiFi(){

    if (WiFi.status() == WL_CONNECTED)
        return;
         
    WiFi.begin(SSID, PASSWORD);
     
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
   
    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}

void VerificaConexoesWiFI(void){
    if(WiFi.status() != WL_CONNECTED){
      Serial.println("Não conectado");
      reconectWiFi();
    } 
}
 
void loop() 
{   
  bool status;      //Recebe da API
  bool busy;        //Recebe da API 
  bool timeOpen;    //Recebe da API
  bool fdCurso;     //Envia para API
  int idLocker = 1; //Envia para API

  VerificaConexoesWiFI();
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("Não conectado");
    reconectWiFi();
  } else {
    client.setInsecure();
    if (client.connect(host, httpsPort)) {
      fdCurso = digitalRead(D1); //0 - Fechado / 1 - Aberto
      String payload = "{";
      payload += "\"idLocker\":" + String(idLocker) + ",";
      payload += "\"fdCurso\":" + String(fdCurso);
      payload += "}";
      Serial.println(payload);
      String endpoint = "/api/verifyLocker";
      client.print(String("POST ") + endpoint + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Authorization: Bearer " + String(jwtToken) + "\r\n" +
                 "Connection: close\r\n" +
                 "Content-Type: application/json\r\n" +
                 "Content-Length: " + payload.length() + "\r\n\r\n" +
                 payload);
      String responseBody = "";
      bool isBody = false;
      while (client.connected() || client.available()) {
        if (client.available()) {
          String line = client.readStringUntil('\n');
          if (line == "\r") {
            isBody = true; 
            continue;
          }
          if (isBody) {
            responseBody += line; 
          }
        }
      }
      Serial.println(responseBody);
      StaticJsonDocument<200> doc;
      DeserializationError error = deserializeJson(doc, responseBody);
      if (!error) {
        status = doc["status"];
        busy = doc["busy"];
        timeOpen = doc["timeOpen"];
      } else {
        Serial.print("Erro ao parsear JSON: ");
        Serial.println(error.c_str());
      }
    }
  }
  if(status){
    digitalWrite(D8, HIGH);
  } else {
    digitalWrite(D8, LOW);
  }
  if(busy){
    digitalWrite(D4, HIGH);
    digitalWrite(D3, LOW);
  } else {
    digitalWrite(D4, LOW);
    digitalWrite(D3, HIGH);
  }

  if(timeOpen){
    tone(D2, 2616);
  } else {
    noTone(D2);
  }
}