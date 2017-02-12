
//authentication + DH11 + sync value from Blynk server + Timer
//board:WeMosD1(Retired)
//speed:115200
//#include <avr/avr/wdt.h>
#include <TimeLib.h>
#include <ArduinoJson.h>
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <dht11.h>
#include <SimpleTimer.h> // here is the SimpleTimer library
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
///////
#include <ESP8266HTTPClient.h>
//#include "EspSaveCrash.h" 

LiquidCrystal_I2C lcd(0x3f,16,2); 
//const char* host = "http://mapidroid.aqicn.org/aqicn/json/android/_c87IzEvUf9Yx4WXzXv2Xq2c8m7bhxdL9T3t2Pe2YDWQ_2bH26f5mAA/v9.json?cityID=China%2F%25E6%2588%2590%25E9%2583%25BD%2F%25E9%25AB%2598%25E6%2596%25B0%25E8%25A5%25BF%25E5%258C%25BA%25E5%2588%259B%25E6%2596%25B0%25E4%25B8%25AD%25E5%25BF%2583&lang=zh&package=Asia&appv=130&appn=3.2.2&tz=28800000&metrics=720,1280,2.0&wifi&devid=3f80d5656331761a%20HTTP/1.1";
//const char* host =   "http://mapidroid.aqicn.org/aqicn/json/android/_c87IzEvUf9Yx4WXzXv2Xzauf9u1_2jrr2YKFAA/v9.json?cityID=China%2F%25E6%2588%2590%25E9%2583%25BD%2F%25E9%2583%25AB%25E5%258E%25BF%25E5%2585%259A%25E6%25A0%25A1&lang=zh&package=Asia&appv=132&appn=3.5&tz=28800000&metrics=1080,1920,3.0&wifi&devid=384232c6e38e4cdf%20HTTP/1.1";
const char* host = "http://mapidroid.aqicn.org/aqicn/json/android/_c87IzEvUf9Yx4WXzXv2Xzauf9u1_2jrr2YKFAA/v10.json?cityID=China%2F%E6%88%90%E9%83%BD%2F%E9%83%AB%E5%8E%BF%E5%85%9A%E6%A0%A1&lang=zh&package=Asia&appv=132&appn=3.5&tz=28800000&metrics=1080,1920,3.0&wifi&devid=384232c6e38e4cdf";
//unsigned int port = 80;

dht11 DHT11;

#define DHT11PIN 2

SimpleTimer timer; // Create a Timer object called "timer"!
String CurrentAQI = "0";
long CurrentTime = 0;
int CurrentTemp = 0;
int CurrentHumi = 0;
String CurrentOutdoorTemp = "0";
int switchPin = 14;
int EdgeValue = 0;
int SwitchOn = 0;

int disconnects; // number of times disconnected from server
bool GPIOStatus = false; 
String progress = "DB";
void(* resetFunc) (void) = 0; //declare reset function @ address 0 THIS IS VERY USEFUL

void GetAQI(){

  HTTPClient http;
  http.begin(host);
  int httpCode = http.GET();
  if(httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      //Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if(httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          //Serial.println(payload);
          DynamicJsonBuffer jsonBuffer; 
          //payload = "{\"aqi\":193,\"time\":1486389600}";
          JsonObject& root = jsonBuffer.parseObject(payload);
          if (!root.success()) {
            Serial.println("parseObject(payload) failed");
            return;
          }
          CurrentTime = root["u"];
          //Serial.println("time is :"+(String)CurrentTime);  
          
          //Blynk.email("Get AQI from Website..." + CurrentTime,"start query");
          int caqi = root["a"];
          CurrentAQI = (String)caqi;
          int ctemp = root["t"];
          CurrentOutdoorTemp = (String)(ctemp);
          //Serial.println(caqi);
//          if(caqi<1)
//          {
//            //Blynk.email("Failed to get AQI from Website..." + CurrentTime,"Failed to get AQI from Website, ignore the value");
//          }
//          else{
//            CurrentAQI = (String)caqi;
//          }
          //payload = payload.substring(0,payload.indexOf(','));
          //CurrentAQI = payload.substring(payload.indexOf(':')+1,payload.length());
          
          Serial.println("get-AQI:"+CurrentAQI); 
      }
  } else {
      //Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  } 
  http.end();  
}

void GetTempAndHum() {
 
  checkWIFI(); 

  int chk = DHT11.read(DHT11PIN);

  //Serial.print("Read sensor: ");
  switch (chk)
  {
    case DHTLIB_OK: 
                //Serial.println("OK"); 
                
                CurrentTemp = (int)DHT11.temperature;
                CurrentHumi = (int)DHT11.humidity; 
                break;
    case DHTLIB_ERROR_CHECKSUM: 
                //Serial.println("Checksum error"); 
                break;
    case DHTLIB_ERROR_TIMEOUT: 
                //Serial.println("Time out error"); 
                break;
    default: 
                //Serial.println("Unknown error"); 
                break;
  }  
  
  //CurrentTemp = (int)DHT11.temperature;
  //CurrentHumi = (int)DHT11.humidity; 
  //Serial.print("Temperature (oC) to V0: ");
  //Serial.println((float)DHT11.temperature, 2);
  //Serial.print("Humidity (%) to V1: ");
  //Serial.println((float)DHT11.humidity, 2); 
  //Serial.println("AQI:"+CurrentAQI);

  DisplayLCD();
}
void DisplayLCD(){
  
  lcd.clear();
  lcd.setCursor(0, 0);  
//  int dAQI = CurrentAQI.toInt();
//  String sCurrentAQI = "";
//  if(dAQI>150){
//    sCurrentAQI=CurrentAQI+" :(";
//  }
//  else if(dAQI>100){
//    sCurrentAQI=CurrentAQI+" :|";
//  }
//  else {
//    sCurrentAQI=CurrentAQI+" :)";
//  }
  
  int curDay = day(CurrentTime);
  int curHour = hour(CurrentTime); 
  curHour = curHour+9;
  if (curHour >= 24)
  {
    curHour = curHour-24;  
    curDay = curDay + 1;
  }
  String cday = (String)curDay; 
  String chour = (String)curHour; 
  
  progress = progress=="db"?"DB":"db";
  
  lcd.print("["+CurrentAQI + "] " + cday + "-" + chour +" " +(String)WiFi.RSSI() + progress); 
  lcd.setCursor(0, 1);  
  lcd.print("T:"+ (String)DHT11.temperature + "/" + CurrentOutdoorTemp + "c H:"+(String)DHT11.humidity+"%");   
}
void SetDataToBlynkVisualPin(){
  Blynk.virtualWrite(V0, CurrentTemp); 
  Blynk.virtualWrite(V1, CurrentHumi); 
  if(CurrentAQI=="0"){
    GetAQI();
    DisplayLCD();
  }
  Blynk.virtualWrite(V2, CurrentAQI.toInt()); 
 // Serial.println("push data to blynk server");
}

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "";
bool isFirstConnect = true;
BLYNK_CONNECTED() {
  if (isFirstConnect) {
    // Request Blynk server to re-send latest values for all pins
    Blynk.syncAll(); 

    isFirstConnect = false;
  } 
}

void checkWIFI()
{
  int cnt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (cnt++ >= 10) {
      WiFi.beginSmartConfig();
      while (1) {
        delay(1000);
        if (WiFi.smartConfigDone()) {
          Serial.println();
          Serial.println("SmartConfig: Success");
          break;
        }
        Serial.print("|");
        if (cnt++ >= 80){
          resetFunc();  
        }
      }
    }
  }

  
}
WidgetTerminal terminal(V5);
//switch on/off seting
BLYNK_WRITE(V4)
{
  SwitchOn = param.asInt();  
  checkAutoSwtich();
  Serial.println("switch value is "+ (String)SwitchOn);
  terminal.println("switch value is "+ (String)SwitchOn);
  terminal.flush();
}
//edge value seting
BLYNK_WRITE(V3)
{
  EdgeValue = param.asInt();  
  checkAutoSwtich();
  Serial.println("edge value set as "+ (String)EdgeValue);
  terminal.println("edge value set as "+ (String)EdgeValue);
  terminal.flush();
}


void checkAutoSwtich()
{ 
    int caqi = CurrentAQI.toInt();
    if(SwitchOn >0 && (caqi==0 || caqi>=EdgeValue))
    {
      if(!GPIOStatus){
        digitalWrite(switchPin,HIGH);
        Serial.println("switch on " + (String)switchPin);
        terminal.println("switch on " + (String)switchPin);
        GPIOStatus = true;
      }
       
    }
    else
    {
      if(GPIOStatus){
        digitalWrite(switchPin,LOW);
        Serial.println("switch off " + (String)switchPin);
        terminal.println("switch off " + (String)switchPin);
        GPIOStatus = false;
      }
    } 
    terminal.flush();
}

void setup()
{
  Serial.begin(9600);
 
  pinMode(switchPin, OUTPUT); 
  lcd.init();   
  lcd.backlight(); 
  
  WiFi.mode(WIFI_STA);

  checkWIFI();
  
  WiFi.printDiag(Serial);
  
  Blynk.config(auth);

  //SaveCrash.print();
  //timer.setInterval(6000L,checkBlynk);//6 seconds
  
  // Here you set interval 1000=1s  current 1 minute
  GetTempAndHum();
  GetAQI();
  DisplayLCD();
  //LoadData();
  //timer.setInterval(60000L,LoadData);//1 minute
  timer.setInterval(60000L,GetTempAndHum);//1 minute
  timer.setInterval(60000L,SetDataToBlynkVisualPin);
  timer.setInterval(60000L,checkAutoSwtich);
  timer.setInterval(600000L,GetAQI);//10 minute
  //timer.setInterval(6000L,GetAQI);//20 minute
  

  
  //ESP.wdtEnable(1000);
  //Serial.print("watch dog setup");
}

void loop()
{
  Blynk.run();
  timer.run(); // SimpleTimer is working
  //Serial.print("watch me!");
  //Serial.print("delay 3000 ms");
  //delay(3000);
  //ESP.wdtFeed();
}

