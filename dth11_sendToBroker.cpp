#include <WiFi.h>
#include <PubSubClient.h>

#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT11  

const char* ssid = "Totalplay-12A2";
const char* password = "12A28E0FNtF9Qfmv";
const char* mqttServer = "192.168.100.97";
const int mqttPort = 1883;
const char* mqtt_topic = "esp32/dht11";
const char* mqtt_topic_led = "esp32/led1";
const char* mqttUser = "vpc";
const char* mqttPassword = "123";

WiFiClient espClient;
PubSubClient client(espClient);

DHT dht(DHTPIN, DHTTYPE); 

int ledPin = 5;  // Pin del LED

void setup() { 
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  dht.begin();  

  WiFi.begin(ssid, password);
  Serial.println("...................................");

  Serial.print("Connecting to WiFi.");
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".") ;
  }

  Serial.println("Connected to the WiFi network");
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  while (!client.connected()){      
    Serial.println("Connecting to MQTT...");
       if (client.connect("ESP32Client", mqttUser, mqttPassword )){
           Serial.println("connected");
           client.subscribe(mqtt_topic_led);
       }
       else
       {   Serial.print("failed with state ");
           Serial.print(client.state());
           delay(2000);
       }
  }
}  

void loop()
  {  
    client.loop();
    char str[16];
    float temperatura = dht.readTemperature();

    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println(" °C");

    char tempStr[10];
    snprintf(tempStr, sizeof(tempStr), "%.2f", temperatura);

    client.publish(mqtt_topic, tempStr);
    Serial.println(str);
    delay(500);
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

  if (String(topic) == mqtt_topic_led) {
    if (message == "on") {
      digitalWrite(ledPin, HIGH);  // Encender el LED
    } else if (message == "off") {
      digitalWrite(ledPin, LOW);   // Apagar el LED
    }
  }
}
