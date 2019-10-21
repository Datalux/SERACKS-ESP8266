#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "ESP8266TrueRandom.h"
#include "Crypto.h"

#define N 14
#define M 5

#define KEY_LENGTH 16

byte key[KEY_LENGTH] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

const char* ssid = "ESP_D54736"; 

int __f__(int v){ return v % N; }
int __g__(int v){ return v % M; }


int Nv, Nf;

ESP8266WebServer server(80);  

void handleRequest(){

  Serial.println("Received new request...");
  
  Nv = ESP8266TrueRandom.rand();
  Nf = ESP8266TrueRandom.rand();

  Serial.println(String("\n\nSending challenge... ") + Nv + " " + Nf);
  
  String resp = (String)Nv + "," + (String)Nf + "\r\n";
  server.send(200, "text/plain", resp);
}

void handleResponse(){
    
  Serial.println("Received new response...");

  int _nv = __f__(Nv);
  int _nf = __g__(Nf);

  SHA256HMAC hmac(key, KEY_LENGTH);
  hmac.doUpdate(Nv+"0");
 
  byte authCode[SHA256HMAC_SIZE];
  hmac.doFinal(authCode);

  char result[SHA256HMAC_SIZE];

  Serial.println();
  /* authCode now contains our 32 byte authentication code */
  for (byte i=0; i < SHA256HMAC_SIZE; i++){
    result[i] = (char)authCode[i];
  }
  
  Serial.println(result);
  
  char input[SHA256HMAC_SIZE+1];
  server.arg(0).toCharArray(input, SHA256HMAC_SIZE);
  String resp;
  if(strcmp(result, input) != 0){
    resp = "ok!\r\n";
  } else {
    resp = "no!\r\n";
  }
  server.send(200, "text/plain", resp);
}


void setup() {
  delay(200);                           
  Serial.begin(115200);                 
  pinMode(2, OUTPUT);                   
  WiFi.softAP(ssid);

  IPAddress myIP = WiFi.softAPIP();     
  Serial.print("AP IP address: ");
  Serial.println(myIP);         
  server.on("/request", handleRequest);
  server.on("/response", handleResponse);
  server.begin();               
  Serial.println("Server started");
}

void loop() {
  server.handleClient();
}
