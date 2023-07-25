#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include <IRsend.h>
#include "LiquidCrystal_I2C.h"

LiquidCrystal_I2C lcd(0x27, 16,2);

const char* ssid = "motorola"; //Totalplay-12A2
const char* password = "904390439"; //12A28E0FNtF9Qfmv
const char* mqttServer = "homesecuremqtt.ddns.net";
const int mqttPort = 1883;
const char* mqttUser = "leonardo";
const char* mqttPassword = "90tr4dd3zqKeifermc12";

const char* topicAir = "/homeSecure/esp32/air";
const char* topicAirJson = "/homeSecure/esp32/air/json";

char* airJsonMessage = NULL;

WiFiClient espClient;
PubSubClient client(espClient);


const uint16_t kRecvPin = 4;
const uint16_t kIrLedPin = 23;      
const uint16_t kPushButtonPin = 5; 

const int ledRed = 18;
const int ledGreen = 19;

const uint32_t kBaudRate = 115200;
const uint16_t kCaptureBufferSize = 1024;

#if DECODE_AC
const uint8_t kTimeout = 50;
#else   

const uint8_t kTimeout = 15;
#endif  

const uint16_t kMinUnknownSize = 12;


#define LEGACY_TIMING_INFO false

IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;  // Somewhere to store the results


IRsend irsend(kIrLedPin);

bool encendido = false;


uint16_t poweroff[] = {3160, 1642,  434, 1122,  432, 1128,  430, 418,  432, 414,  434, 418,  432, 1124,  432, 416,  434, 418,  432, 1124,
                       434, 1124,  434, 416,  432, 1126,  432, 416,  438, 412,  434, 1122,  432, 1126,  434, 416,  432, 1124,  434, 1124,  
                       432, 418,  432, 416,  434, 1124,  432, 418,  432, 418,  432, 1126,  434, 416,  430, 418,  434, 416,  430, 418,  434, 
                       414,  434, 416,  432, 416,  434, 416,  432, 416,  434, 416,  434, 414,  432, 416,  434, 414,  432, 416,  434, 414,  
                       432, 418,  432, 416,  432, 418,  432, 418,  432, 416,  432, 1126,  432, 416,  432, 418,  432, 1124,  434, 1122,  
                       434, 416,  432, 418,  432, 416,  434, 416,  430, 418,  432, 416,  432, 1126,  432, 1126,  432, 1126,  432, 1124,  
                       434, 416,  432, 416,  434, 414,  432, 418,  434, 416,  432, 416,  432, 418,  430, 416,  434, 416,  432, 416,  432, 
                       1124,  432, 416,  434, 416,  432, 418,  452, 396,  434, 416,  430, 418,  432, 416,  432, 416,  434, 416,  432, 416,  
                       434, 416,  434, 416,  432, 416,  434, 418,  432, 416,  432, 416,  434, 416,  432, 418,  434, 416,  432, 416,  432, 
                       416,  432, 416,  432, 418,  432, 416,  432, 416,  434, 416,  454, 396,  432, 416,  432, 418,  432, 418,  432, 414,  
                       434, 416,  434, 416,  430, 1126,  434, 1122,  432, 1124,  432, 418,  432, 416,  432, 418,  432, 416,  432, 1124,  454};

uint16_t poweron[] =  {3142, 1640,  434, 1126,  432, 1124,  434, 416,  434, 416,  434, 414,  434, 1124,  432, 416,  432, 416,  434, 
                       1122,  434, 1124,  434, 416,  432, 1126,  432, 416,  434, 416,  434, 1124,  434, 1122,  434, 416,  434, 1124,  
                       432, 1126,  432, 418,  432, 416,  432, 1126,  434, 418,  432, 416,  432, 1126,  432, 416,  434, 416,  430, 416,  
                       432, 418,  432, 416,  434, 416,  432, 418,  432, 416,  432, 416,  434, 418,  432, 416,  434, 416,  434, 416,  432, 
                       416,  434, 414,  434, 416,  432, 420,  430, 1126,  432, 418,  432, 416,  434, 1126,  434, 414,  434, 416,  434, 1124,  
                       434, 1124,  432, 418,  430, 418,  432, 418,  432, 416,  434, 416,  432, 416,  434, 1124,  432, 1124,  432, 1126,  
                       432, 1124,  432, 418,  432, 416,  434, 416,  432, 418,  432, 414,  432, 416,  434, 416,  432, 418,  434, 414,  432, 
                       416,  432, 1126,  432, 416,  430, 418,  430, 418,  432, 416,  436, 414,  432, 418,  430, 420,  430, 418,  436, 412,  
                       432, 418,  432, 418,  430, 418,  432, 416,  434, 416,  434, 414,  434, 414,  432, 418,  432, 416,  432, 418,  430, 
                       416,  432, 416,  434, 416,  434, 416,  432, 416,  434, 414,  434, 416,  434, 416,  432, 418,  434, 414,  434, 416,  
                       434, 416,  434, 416,  432, 418,  434, 1122,  436, 1124,  432, 416,  434, 1124,  434, 416,  432, 418,  432, 416,  432, 1126,  434};

void reconnect() {

  while (!client.connected()){      

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Start MQTT...");


    Serial.println("Connecting to MQTT...");
    delay(1000);
       if (client.connect("ESP32Client_Clima", mqttUser, mqttPassword )){
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("connected");
          Serial.println("connected");
          client.subscribe(topicAir);
          delay(500);
          lcd.clear();

       }
       else
       {   
           lcd.clear();
           lcd.setCursor(0, 0);
           lcd.print("Failed");
           Serial.println("failed, trying again");
           Serial.print(client.state());
           delay(2000);
       }
  }

}


void startConection()
{
  WiFi.begin(ssid, password);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Starting");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi");

  Serial.print("Connecting WiFi.");


  lcd.setCursor(0, 1);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".") ;
    lcd.print(".");
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connected WiFi");
  delay(1000);


  Serial.println("Connected to the WiFi network");
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

}

void sendOff() {
  if(encendido){
      uint16_t rawDataLength = sizeof(poweroff) / sizeof(poweroff[0]);
      uint16_t khz = 38;  
    
      irsend.sendRaw(poweroff, rawDataLength, khz); 
      Serial.println("Aire acondicionado apagado");
      airJsonMessage = "off";
      Serial.println("Clima apagado");
      encendido = false;
    }
}

void sendOn() {
  if(!encendido){
      uint16_t rawDataLength = sizeof(poweron) / sizeof(poweron[0]);
      uint16_t khz = 38;  
    
      irsend.sendRaw(poweron, rawDataLength, khz); 
      Serial.println("Aire acondicionado encendido");
      airJsonMessage = "on";
      Serial.println("Clima encendido");
      encendido = true;
    }
}

void airIR() {
  if (irrecv.decode(&results)) {
    uint32_t now = millis();
    Serial.printf(D_STR_TIMESTAMP " : %06u.%03u\n", now / 1000, now % 1000);
    if (results.overflow)
      Serial.printf(D_WARN_BUFFERFULL "\n", kCaptureBufferSize);
    Serial.println(D_STR_LIBRARY "   : v" _IRREMOTEESP8266_VERSION_ "\n");
    Serial.print(resultToHumanReadableBasic(&results));
    String description = IRAcUtils::resultAcToString(&results);
    if (description.length()) Serial.println(D_STR_MESGDESC ": " + description);
    yield();
#if LEGACY_TIMING_INFO
    Serial.println(resultToTimingInfo(&results));
    yield();  
#endif  
    Serial.println(resultToSourceCode(&results));
    Serial.println();    
    yield();             
  }


  if (digitalRead(kPushButtonPin) == LOW) {  
    encendido = !encendido;
   
    sendOff();
    sendOn();
    
    delay(500); 
  }
}

void setup() {
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(kPushButtonPin, INPUT_PULLUP);  
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);

  
  Serial.begin(115200);
  
  digitalWrite(ledRed, HIGH);
  startConection();
  digitalWrite(ledRed, LOW);
  digitalWrite(ledGreen, HIGH);

#if defined(ESP8266)
  Serial.begin(kBaudRate, SERIAL_8N1, SERIAL_TX_ONLY);
#else  // ESP8266
  Serial.begin(kBaudRate, SERIAL_8N1);
#endif  // ESP8266
  while (!Serial) 
    delay(50);
  Serial.printf("\n" D_STR_IRRECVDUMP_STARTUP "\n", kRecvPin);
#if DECODE_HASH
 
  irrecv.setUnknownThreshold(kMinUnknownSize);
#endif 
  irrecv.enableIRIn();  // Start the receiver

  irsend.begin();  
}



void loop() {

  if (!client.connected()) {
    digitalWrite(ledRed, HIGH);
    reconnect();
    digitalWrite(ledRed, LOW);
    digitalWrite(ledGreen, HIGH);
  }
  client.loop();

  airIR();

  
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido en el tópico: ");
  Serial.println(topic);
  
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Mensaje recibido: ");
  Serial.println(message);

  if (String(topic) == topicAir) {
    if (message == "on") {
      Serial.println("on");
      sendOn();

      StaticJsonDocument<200> jsonDoc;
      jsonDoc["data"] = "Clima activado";
      jsonDoc["status"] = "on";

      char buffer[200];
      serializeJson(jsonDoc, buffer);

      client.publish(topicAirJson, buffer);
    } else if (message == "off") {
      Serial.println("off");
      sendOff();

      StaticJsonDocument<200> jsonDoc;
      jsonDoc["data"] = "Clima desactivado";
      jsonDoc["status"] = "off";

      char buffer[200];
      serializeJson(jsonDoc, buffer);

      client.publish(topicAirJson, buffer);
    }
  }
}
