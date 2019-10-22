#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "ESP8266TrueRandom.h"
#include "Crypto.h"

#define N 5
#define M 1 
#define KEY_LENGTH 16

const char *ssid = "ESP_D54736"; 

int __f__(int v){ return v % N; }
int __g__(int v){ return v % M; }

int Nv, Nf;

byte keys[N][KEY_LENGTH] = {
  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
  {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16},
  {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17},
  {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18},
  {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19}  
};


ESP8266WebServer server(80);  


void array_to_string(byte array[], unsigned int len, char buffer[])
{
    for (unsigned int i = 0; i < len; i++)
    {
        byte nib1 = (array[i] >> 4) & 0x0F;
        byte nib2 = (array[i] >> 0) & 0x0F;
        buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
        buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    buffer[len*2] = '\0';
}

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

  Serial.print("Server's Nv: ");
  Serial.println(Nv);
  Serial.print("Server's Nf: ");
  Serial.println(Nf);

  String in = String("") + Nv;
  int in_len = in.length();
  char input[in_len+1];
  in.toCharArray(input, in_len+1);
  input[in_len] = '\0';

  SHA256HMAC hmac(keys[__f__(Nv)], KEY_LENGTH);

  hmac.doUpdate(input);

  byte authCode[SHA256HMAC_SIZE];
  hmac.doFinal(authCode);
  
  Serial.println("");
  Serial.print("Encrypted: ");
  
  char auth_str[(SHA256HMAC_SIZE*2)+1];
  array_to_string(authCode, SHA256HMAC_SIZE, auth_str);
  
  Serial.println(auth_str);
  

  Serial.print("Client's Nv: ");
  Serial.println(server.arg(0));
  
  String resp;
  
  if(server.arg(0).equals(auth_str)){
   resp = "ok!\r\n";
   } else {
    resp = "no!\r\n";
  }
  Serial.println(resp);
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
