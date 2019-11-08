#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "ESP8266TrueRandom.h"
#include "Crypto.h"
#include "base64.hpp"

#define N 5
#define M 1 
#define KEY_LENGTH 16

const char *ssid = "VEHICLE";

ESP8266WebServer server(80);

boolean can_open = false;

int Nv, Nf;

int __f__(int v){ return v % N; }
int __g__(int v){ return v + 1; }

byte keys[N][KEY_LENGTH] = {
  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
  {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16},
  {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17},
  {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18},
  {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19}  
};

void array_to_string(byte array[], unsigned int len, char buffer[])
{
    for (unsigned int i = 0; i < len; i++){
        byte nib1 = (array[i] >> 4) & 0x0F;
        byte nib2 = (array[i] >> 0) & 0x0F;
        buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
        buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    buffer[len*2] = '\0';
}

void bufferSize(char* text, int &length)
{
  int i = strlen(text);
  int buf = round(i / KEY_LENGTH) * KEY_LENGTH;
  length = (buf <= i) ? buf + KEY_LENGTH : length = buf;
}
    
void encrypt(char* plain_text, char* output, int length, byte *key, uint8_t *iv)
{
  byte enciphered[length];
  //RNG::fill(iv, KEY_LENGTH); 
  AES aesEncryptor(key, iv, AES::AES_MODE_128, AES::CIPHER_ENCRYPT);
  aesEncryptor.process((uint8_t*)plain_text, enciphered, length);
  int encrypted_size = sizeof(enciphered);
  char encoded[encrypted_size];
  encode_base64(enciphered, encrypted_size, (unsigned char*)encoded);
  strcpy(output, encoded);
}

void decrypt(char* enciphered, char* output, int length, byte *key, uint8_t *iv)
{
  length = length + 1; //re-adjust
  char decoded[length];
  decode_base64((unsigned char*)enciphered, (unsigned char*)decoded);
  bufferSize(enciphered, length);
  byte deciphered[length];
  AES aesDecryptor(key, iv, AES::AES_MODE_128, AES::CIPHER_DECRYPT);
  aesDecryptor.process((uint8_t*)decoded, deciphered, length);
  strcpy(output, (char*)deciphered);
}

char* string2char(String ipString){ // make it to return pointer not a single char
  char* opChar = new char[ipString.length() + 1]; // local array should not be returned as it will be destroyed outside of the scope of this function. So create it with new operator.
  memset(opChar, 0, ipString.length() + 1);

  for (int i = 0; i < ipString.length(); i++)
    opChar[i] = ipString.charAt(i);
  return opChar; //Add this return statement.
}

void handleRequest()
{

  Serial.println("Received new request...");
  
  Nv = ESP8266TrueRandom.rand();
  Nf = ESP8266TrueRandom.rand();

  can_open = true;
  
  Serial.println(String("\n\nSending challenge... ") + Nv + " " + Nf);
  
  String resp = (String)Nv + "," + (String)Nf + "\r\n";
  server.send(200, "text/plain", resp);
}

void handleResponse()
{
    
  Serial.println("Received new response...");

  if(can_open){
    int _nv = __f__(Nv);
    int _nf = __g__(Nf);
  
    Serial.print("Server's Nv: ");
    Serial.println(Nv);
    Serial.print("Server's Nf: ");
    Serial.println(Nf);
  
    /*String in = String("") + _nf;
    int in_len = in.length();
    char input[in_len+1];
    in.toCharArray(input, in_len+1);
    input[in_len] = '\0';*/ 

    uint8_t iv[KEY_LENGTH] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


    char str[12];
    sprintf(str, "%d", _nf);
    Serial.print("Encrypt: ");
    Serial.println(str);
    Serial.println(String("with key: ") + _nv);

    // encrypt
    int length = 0;
    bufferSize(str, length);
    char auth_str [length];
    encrypt(str, auth_str, length, keys[_nv], iv);
  
    Serial.println("");
    Serial.print("Encrypted: ");
    Serial.println(auth_str);

    Serial.println("");

    // decrypt
    /*char *server_input = string2char(server.arg(0));
    length = strlen(server_input);
    char decrypted[length];
    decrypt(server_input, decrypted, length, keys[_nv], iv);
  
    Serial.print("Decrypted: ");
    Serial.println(decrypted);*/

    /*length = server.arg(0).length();
    char server_input[length+1];
    server.arg(0).toCharArray(server_input, length+1);
    char decrypted[length+1];
    decrypt(server_input, decrypted, length+1, keys[_nv]);
  
    Serial.print("Decrypted: ");
    Serial.println(decrypted);*/
  
    Serial.print("Client's Nv: ");
    Serial.println(server.arg(0));
    
    String resp;
    
    if(server.arg(0).equals(auth_str)){
     resp = "ok!\r\n";
     } else {
      resp = "no!\r\n";
      can_open = false;
    }
    Serial.println(resp);
    server.send(200, "text/plain", resp);
  } else{
    server.send(200, "text/plain", "not authorized");
  }
  
  can_open = false;
}


void setup() 
{
  delay(200);                           
  Serial.begin(115200);                 
  pinMode(2, OUTPUT); 
  WiFi.disconnect();                  
  WiFi.softAP(ssid);

  IPAddress myIP = WiFi.softAPIP();     
  Serial.print("AP IP address: ");
  Serial.println(myIP);         
  server.on("/request", handleRequest);
  server.on("/response", handleResponse);
  server.begin();            
  Serial.println("Server started");
  
}

void loop()
{
  server.handleClient();
}
