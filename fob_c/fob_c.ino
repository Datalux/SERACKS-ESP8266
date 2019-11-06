#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include "Crypto.h"
#include "base64.hpp"

#define N 5

#define KEY_LENGTH 16

#define STATUS_IDLE  0
#define STATUS_REQ   1

int status = 0;

uint8_t GPIO_Pin = D2;
  
const char *host = "192.168.4.1"; 

int __f__(int v){ return v % N; }
int __g__(int v){ return v + 1; }


byte keys[N][KEY_LENGTH] = {
  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
  {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16},
  {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17},
  {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18},
  {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19}  
};

uint8_t iv[KEY_LENGTH] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void bufferSize(char* text, int &length)
{
  int i = strlen(text);
  int buf = round(i / KEY_LENGTH) * KEY_LENGTH;
  length = (buf <= i) ? buf + KEY_LENGTH : length = buf;
}
    
void encrypt(char* plain_text, char* output, int length, byte *key)
{
  byte enciphered[length];
  RNG::fill(iv, KEY_LENGTH); 
  AES aesEncryptor(key, iv, AES::AES_MODE_128, AES::CIPHER_ENCRYPT);
  aesEncryptor.process((uint8_t*)plain_text, enciphered, length);
  int encrypted_size = sizeof(enciphered);
  char encoded[encrypted_size];
  encode_base64(enciphered, encrypted_size, (unsigned char*)encoded);
  strcpy(output, encoded);
}

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

void setReqStatus()
{
  status = STATUS_REQ;
}

void _run()
{
   
  digitalWrite(BUILTIN_LED, HIGH);
  
  Serial.print("connecting to ");
  Serial.println(host);

  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect("192.168.4.1", httpPort)) {
    Serial.println("connection failed");
    status = STATUS_IDLE;
    return;
  }    

  client.print(String("GET ") +"/request"+" HTTP/1.1\r\n" + 
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");   


  Serial.println(String("Waiting for challenge..."));
  
  delay(1000);  

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

  delay(1000);
 
  String in = String("") +v[0];
  int in_len = in.length();
  char input[in_len+1];
  in.toCharArray(input, in_len+1);
  input[in_len] = '\0';

  char str[12];
  sprintf(str, "%d", __g__(v[0]));
  Serial.print("Encrypt: ");
  Serial.println(str);

  // encrypt
  int length = 0;
  bufferSize(str, length);
  char auth_str[length];
  encrypt(str, auth_str, length, keys[__f__(v[0])]);

  Serial.println("");
  Serial.print("Encrypted: ");
  Serial.println(auth_str); 

  Serial.println("Sending response...");
  if (!client.connect("192.168.4.1", httpPort)) {
    Serial.println("connection failed");
    status = STATUS_IDLE;
    return;
  }     


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

  status = STATUS_IDLE;
  
}

void setup()
{
  Serial.begin(115200);          
  delay(10);   
  pinMode(19, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(GPIO_Pin), setReqStatus, FALLING);   
  
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
  pinMode(BUILTIN_LED, OUTPUT);            
  
}



void loop()
{
  if(status == STATUS_REQ){
    _run();
  }
}
