#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include "Crypto.h"

#define N 14
#define M 5

#define KEY_LENGTH 16

byte key[KEY_LENGTH] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};


const char *host = "192.168.4.1"; 
bool ok = false;

int __f__(int v){ return v % N; }
int __g__(int v){ return v % M; }

void _run(){
  digitalWrite(BUILTIN_LED, HIGH);
  
  Serial.print("connecting to ");
  Serial.println(host);

  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect("192.168.4.1", httpPort)) {
    Serial.println("connection failed");
    return;
  }    

  client.print(String("GET ") +"/request"+" HTTP/1.1\r\n" + 
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");   


  Serial.println(String("Waiting for response..."));
  delay(2000);

  int v[2];
  int i = 0;

  String line;
  while (client.connected() || client.available()){
    if (client.available()){
      line = client.readStringUntil('\n');
    }
  }

  char _line[line.length()+1];
  strcpy(_line, line.c_str());
      
  char *p = strtok(_line,",");
  while (p!= NULL){
      v[i] = atoi(p);
      i++;        
      p = strtok (NULL, ",");
  }
    
  Serial.println(String("i = ") + v[0]);
  Serial.println(String("j = ") + v[1]);

  Serial.println();
  Serial.println("End request\n"); 

  delay(2000);

  SHA256HMAC hmac(key, KEY_LENGTH);
  hmac.doUpdate(v[0] + "0");
  
  /* And or with a string and length */
  
  /*const char *goodbye = "GoodBye World";
  hmac.doUpdate(goodbye, strlen(goodbye));*/
  
  /* And or with a binary message */
  
  /*byte message[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  hmac.doUpdate(message, sizeof(message));*/
  
  /* Finish the HMAC calculation and return the authentication code */
  byte authCode[SHA256HMAC_SIZE];
  hmac.doFinal(authCode);

  Serial.println();
  /* authCode now contains our 32 byte authentication code */
  for (byte i=0; i < SHA256HMAC_SIZE; i++)
  {
      if (authCode[i]<0x10) { Serial.print('0'); }
      Serial.print(authCode[i], HEX);
  }
  Serial.println();


  Serial.println("Sending response...");
  if (!client.connect("192.168.4.1", httpPort)) {
    Serial.println("connection failed");
    return;
  }  


  String param = String("?i=")+ __f__(v[0]) + "&j=" + __g__(v[1]);
  client.print(String("GET ") +"/response"+ param + " HTTP/1.1\r\n" + 
             "Host: " + host + "\r\n" + 
             "Connection: close\r\n\r\n");


  String l;
  while (client.connected() || client.available()){
    if (client.available()){
      l = client.readStringUntil('\n');
    }
  }
  Serial.println(String("Server says: ") + l);
  Serial.println();
  Serial.println(l);
  digitalWrite(BUILTIN_LED, LOW);

  
  
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  Serial.begin(115200);          
  delay(10);                     
  WiFi.mode(WIFI_STA);           
  WiFi.begin("ESP_D54736");      

  while (WiFi.status() != WL_CONNECTED) {     
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());  

}



void loop() {
  _run();
  delay(5000);

  
}
