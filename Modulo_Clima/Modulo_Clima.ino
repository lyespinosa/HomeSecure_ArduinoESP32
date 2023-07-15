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

const char* ssid = "Totalplay-12A2"; //Totalplay-12A2
const char* password = "12A28E0FNtF9Qfmv"; //12A28E0FNtF9Qfmv
const char* mqttServer = "35.169.100.202";
const int mqttPort = 1883;
const char* mqttUser = "leonardo";
const char* mqttPassword = "90tr4dd3zqKeifermc12";

const char* topicAir = "/homeSecure/esp32/air";
const char* topicAirJson = "/homeSecure/esp32/air/json";

char airJsonMessage = null;

WiFiClient espClient;
PubSubClient client(espClient);


const uint16_t kRecvPin = 4;
const uint16_t kIrLedPin = 23;      
const uint16_t kPushButtonPin = 5; 

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


uint16_t poweroff[] = {3142, 1640,  434, 1124,  432, 1126,  432, 418,  432, 416,  432, 416,  432, 1104,  454, 414,  434, 416,
                       432, 1124,  432, 1126,  432, 420,  428, 1124,  434, 416,  432, 416,  434, 1124,  432, 1124,  434, 416,
                       432, 1124,  434, 1126,  432, 416,  432, 418,  432, 1126,  432, 416,  432, 416,  454, 1104,  430, 416,  456,
                       394,  432, 416,  432, 416,  432, 416,  434, 416,  434, 416,  432, 418,  432, 420,  432, 416,  432, 416,
                       434, 416,  432, 418,  432, 418,  432, 416,  432, 416,  432, 416,  434, 414,  434, 414,  434, 418,  432,
                       1122,  436, 414,  432, 416,  434, 1124,  432, 416,  432, 418,  432, 416,  432, 416,  434, 414,  434, 416,
                       432, 416,  434, 414,  432, 416,  434, 1124,  432, 1126,  432, 418,  432, 418,  432, 416,  432, 418,  432,
                       416,  434, 418,  432, 416,  432, 416,  432, 418,  432, 416,  432, 1124,  434, 416,  432, 418,  432, 416,
                       432, 418,  432, 416,  432, 416,  432, 416,  432, 416,  434, 414,  434, 416,  432, 416,  434, 416,  430, 416,
                       432, 418,  432, 416,  434, 414,  432, 416,  436, 416,  430, 420,  432, 416,  432, 418,  432, 416,  432, 418,
                       432, 416,  432, 416,  434, 416,  434, 414,  432, 416,  434, 416,  432, 418,  454, 396,  430, 416,  434, 416,
                       432, 416,  434, 1124,  454, 394,  454, 394,  454, 394,  454, 396,  456, 392,  454, 1104,  432
                     };

uint16_t poweron[] =  {3164, 1616,  456, 1100,  456, 1102,  458, 392,  456, 392,  458, 392,  456, 1102,  454, 394,  458, 394,  454,
                       1102,  454, 1102,  454, 394,  480, 1078,  480, 368,  482, 366,  484, 1076,  480, 1078,  456, 394,  454, 1102,
                       454, 1102,  456, 392,  456, 394,  456, 1100,  456, 394,  456, 392,  456, 1102,  456, 394,  454, 394,  456, 392, 
                       456, 396,  454, 392,  454, 394,  456, 392,  458, 394,  458, 390,  454, 394,  454, 394,  456, 394,  456, 394,  
                       456, 394,  458, 392,  480, 370,  480, 372,  480, 1076,  482, 368,  484, 364,  482, 1076,  456, 394,  456, 394,  
                       456, 1102,  454, 392,  456, 394,  456, 392,  458, 392,  458, 392,  456, 392,  458, 392,  458, 390,  458, 392,  
                       458, 1100,  456, 1102,  456, 394,  432, 414,  434, 416,  432, 418,  454, 394,  432, 416,  434, 416,  434, 416,  
                       430, 420,  430, 416,  434, 1124,  432, 416,  432, 416,  434, 416,  432, 418,  432, 416,  432, 418,  432, 416,  
                       432, 416,  432, 416,  434, 416,  432, 416,  434, 414,  434, 416,  430, 418,  434, 418,  454, 392,  456, 394,  
                       456, 390,  458, 392,  458, 392,  456, 392,  458, 394,  458, 390,  458, 390,  458, 390,  458, 392,  432, 418,  434, 
                       414,  436, 414,  434, 418,  432, 414,  434, 416,  432, 416,  432, 418,  432, 1126,  432, 1124,  438, 412,  456, 394,  
                       456, 394,  432, 418,  432, 1124,  432
                       };

void startConection(){
  WiFi.begin(ssid, password);
  
  Serial.print("Connecting WiFi.");

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".") ;
    lcd.print(".");
  }

  Serial.println("Connected to the WiFi network");
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  while (!client.connected()){      

    Serial.println("Connecting to MQTT...");
       if (client.connect("ESP32Client", mqttUser, mqttPassword )){
         
          Serial.println("connected");
          client.subscribe(topicAir);
          delay(500);

       }
       else
       {   
         
           Serial.println("failed, trying again");
           Serial.print(client.state());
           delay(2000);
       }
  }

}

void sendOff() {
  if(!encendido){
      uint16_t rawDataLength = sizeof(poweroff) / sizeof(poweron[0]);
      uint16_t khz = 38;  
    
      irsend.sendRaw(poweroff, rawDataLength, khz); 
      Serial.println("Aire acondicionado apagado");
      airJsonMessage = "off";
    }
}

void sendOn() {
  if(!encendido){
      uint16_t rawDataLength = sizeof(poweroff) / sizeof(poweron[0]);
      uint16_t khz = 38;  
    
      irsend.sendRaw(poweroff, rawDataLength, khz); 
      Serial.println("Aire acondicionado encendido");
      airJsonMessage = "on";
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
  startConection();

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
  pinMode(kPushButtonPin, INPUT_PULLUP);  
}



void loop() {

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
      sendOn();
      client.publish(topicAirJson, airJsonMessage);
    } else if (message == "off") {
      sendOff();
      client.publish(topicAirJson, airJsonMessage);
    }
  }
}
