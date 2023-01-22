
// Uncomment the following line to enable serial debug output
//#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
       #define DEBUG_ESP_PORT Serial
       #define NODEBUG_WEBSOCKETS
       #define NDEBUG
#endif 

#include <Arduino.h>
#include <Stepper.h>
#ifdef ESP8266 
       #include <ESP8266WiFi.h>
#endif 
#ifdef ESP32   
       #include <WiFi.h>
#endif

#include "SinricPro.h"

#include "SinricProBlinds.h"

#define WIFI_SSID         "#########"    //wifi-username
#define WIFI_PASS         "#########"    //wifi-password
#define APP_KEY           "sinric-app-key"      // Should look like "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"
#define APP_SECRET        "sinric-app-secret-key"   // Should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"

#define BLINDS_ID         "sinric-device-id" 
#define BAUD_RATE         115200                // Change baudrate to your need
#define EVENT_WAIT_TIME   60000 


unsigned long startMillis;

int poleStep = 0; 
int  dirStatus = 3;

int Pin1 = 14;//IN1 is connected to 10 
int Pin2 = 12;//IN2 is connected to 11  
int Pin3 = 13;//IN3 is connected to 12  
int Pin4 = 15;
int Pin5 = 5;

int pole1[] ={0,0,0,0, 0,1,1,1, 0};//pole1, 8 step values
int pole2[] ={0,0,0,1, 1,1,0,0, 0};//pole2, 8 step values
int pole3[] ={0,1,1,1, 0,0,0,0, 0};//pole3, 8 step values
int pole4[] ={1,1,0,0, 0,0,0,1, 0};//pole4, 8 step values


int blindsPosition = 0;
int pos;
int pos1;
int val;
int a;
int b= 100;
int mul;
bool powerState = false;
bool RangeValue = false;
bool onPowerState(const String &deviceId, bool &state) {
  Serial.printf("Device %s power turned %s \r\n", deviceId.c_str(), state?"on":"off");
  powerState = state;
  
  
  return true; // request handled properly
}

bool onRangeValue(const String &deviceId, int &position) {
  Serial.printf("Device %s set position to %d\r\n", deviceId.c_str(), position);
  
  pos = position;
  if(powerState == 1 & pos<pos1)
  { 
    val = pos-pos1;
    Serial.printf("%d", pos-pos1);
    startMillis = millis();
    digitalWrite(Pin5,HIGH);
    pos1 = pos;
    
  }
  if(powerState == 1 & pos>pos1)
  { 
    val = pos-pos1;
    Serial.printf("%d", pos-pos1);
    startMillis = millis();
    digitalWrite(Pin5,HIGH);
    pos1 = pos;
    
  }
  if(val>=0)
  {
    a = val;
  }
  if(val<0)
  {
    a = -val;
  }
  
   
  
  return true; // request handled properly
}

bool onAdjustRangeValue(const String &deviceId, int &positionDelta) {
  blindsPosition += positionDelta;
  Serial.printf("Device %s position changed about %i to %d\r\n", deviceId.c_str(), positionDelta, blindsPosition);
  Serial.printf("hi");
  positionDelta = blindsPosition; // calculate and return absolute position
  return true; // request handled properly
}
    

// setup function for WiFi connection
void setupWiFi() {
  Serial.printf("\r\n[Wifi]: Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }
  digitalWrite(Pin5,LOW);

  Serial.printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", WiFi.localIP().toString().c_str());
}

// setup function for SinricPro
void setupSinricPro() {
  // get a new Blinds device from SinricPro
  SinricProBlinds &myBlinds = SinricPro[BLINDS_ID];
  myBlinds.onPowerState(onPowerState);
  myBlinds.onRangeValue(onRangeValue);  
  myBlinds.onAdjustRangeValue(onAdjustRangeValue);

  // setup SinricPro
  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
  SinricPro.begin(APP_KEY, APP_SECRET);
}


// main setup function
void setup() {
  pinMode(Pin1, OUTPUT);//define pin for ULN2003 in1 
  pinMode(Pin2, OUTPUT);//define pin for ULN2003 in2   
  pinMode(Pin3, OUTPUT);//define pin for ULN2003 in3   
  pinMode(Pin4, OUTPUT);//define pin for ULN2003 in4   
  pinMode(Pin5, OUTPUT);
  digitalWrite(Pin5, LOW);
  Serial.begin(BAUD_RATE); 
  Serial.printf("\r\n\r\n");
  setupWiFi();
  setupSinricPro();
  
  
}

void loop() {
  SinricPro.handle();
  if(val>0 & powerState == 1) 
  {
    //Serial.printf("%d/r", mul);
    poleStep++; 
    driveStepper(poleStep); 
    
  }
  
  if(val<0 & powerState == 1)
  {
    //Serial.printf("%d", mul);
    poleStep--; 
    driveStepper(poleStep); 
    
  }
  
  mul = a*b;
  if ((digitalRead(Pin5) == HIGH) & ((millis() - startMillis) >= (mul))) 
  {
      digitalWrite(Pin5, LOW); // turn LED OF
      
    
  }
   
 if(poleStep>7){ 
   poleStep=0; 
 } 
 if(poleStep<0){ 
   poleStep=7; 
 }
 //Serial.printf("%d", pos-pos1);
 
 delay(1); 

  
}

void driveStepper(int c)
{
  
     digitalWrite(Pin1, pole1[c]);  
     digitalWrite(Pin2, pole2[c]); 
     digitalWrite(Pin3, pole3[c]); 
     digitalWrite(Pin4, pole4[c]);  
}
