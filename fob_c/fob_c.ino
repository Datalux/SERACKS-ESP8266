#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include "Crypto.h"

#define N 14
#define M 5

#define KEY_LENGTH 16


byte keys[N][KEY_LENGTH] = {
  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
  {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16},
  {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17},
  {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18},
  {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19}  
};

const char *host = "192.168.4.1"; 
bool ok = false;

int __f__(int v){ return v % N; }
int __g__(int v){ return v % M; }

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

  String in = String("") +v[0];
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

  Serial.println("Sending response...");
  if (!client.connect("192.168.4.1", httpPort)) {
    Serial.println("connection failed");
    return;
  }  


  //String param = String("?i=")+ __f__(v[0]) + "&j=" + __g__(v[1]);
  String param = String("?i=")+ auth_str;
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
