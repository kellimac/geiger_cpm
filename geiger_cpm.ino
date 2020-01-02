
#include <Time.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <FS.h>


#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "us.pool.ntp.org"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);



const int countcycle = 600;
const int debounceMs = 11;
const byte interruptPin = 2;
volatile byte interruptCounter = 0;
int numberOfInterrupts = 0;
unsigned long epochTime; 
unsigned long fbeginTime;
float ttC;
float clpm;
char filename[32] = ""; 
char fileext[4] = ".df";
File fh;
 
void setup() {
 
  Serial.begin(115200);
  WiFi.begin("ssid", "password");

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();
  timeClient.update();
  epochTime =  timeClient.getEpochTime();
  
  //Initialize File System
  if(SPIFFS.begin())
  {
    Serial.println("SPIFFS Initialize....ok");
  }
  else
  {
    Serial.println("SPIFFS Initialization...failed");
  }

/* 
  //Format File System
  if(SPIFFS.format())
  {
    Serial.println("FS format successful");
    opendf();
  }
  else
  {
    Serial.println("File System Formatting Error");
  }

 */

Dir dir = SPIFFS.openDir("/");
// or Dir dir = LittleFS.openDir("/");
while (dir.next()) {
    Serial.print(dir.fileName());
    if(dir.fileSize()) {
        File f = dir.openFile("r");
        Serial.println(f.size());
    }
}


  
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, RISING);
 
}
 
ICACHE_RAM_ATTR void handleInterrupt() {

  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
 
   if (interrupt_time - last_interrupt_time > debounceMs)
  {
    interruptCounter++;
  } else {
    Serial.println(F("Bounce ignored"));

  }
   last_interrupt_time = interrupt_time;
}

 
void loop() {


 
  if(interruptCounter>0){
 
      interruptCounter--;
      numberOfInterrupts++;
      epochTime =  timeClient.getEpochTime();
      ttC = epochTime - fbeginTime;
      clpm = (numberOfInterrupts / ttC) * 60.00;
     // char cstr[16];
    //  sprintf(cstr, "%f", clpm);
     
      Serial.print(epochTime);
      Serial.print(F("."));
      Serial.print(ttC,0);
      Serial.print(F(" -> Count "));
      Serial.print(numberOfInterrupts);
      Serial.print(F(" = "));
      Serial.print(clpm,1);
      Serial.println(F(" CPM."));
      fh.print(epochTime);
      
  } else { 
    timeClient.update();
    epochTime =  timeClient.getEpochTime();
    if ((epochTime - fbeginTime) > countcycle) {opendf();}
  }
 
}

void opendf() {
  fh.close();
  fbeginTime = timeClient.getEpochTime();
  sprintf(filename,"%lu%s",fbeginTime,fileext);
  numberOfInterrupts = 0;
  Serial.print("Creating new data file: ");
  Serial.println(filename);
  File fh = SPIFFS.open(filename, "w");
  if (!fh) {
    Serial.println("file open failed");
  }
}

void uploadf() {
  
  
}
