#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "LiquidCrystal_I2C.h"
#include <DHT.h>
#include <Ultrasonic.h>

#define DHTPIN 4
#define DHTTYPE DHT11  

LiquidCrystal_I2C lcd(0x27, 16,2);

const char* ssid = "Totalplay-12A2"; //Totalplay-12A2
const char* password = "12A28E0FNtF9Qfmv"; //12A28E0FNtF9Qfmv
const char* mqttServer = "35.169.100.202";
const int mqttPort = 1883;
const char* mqttUser = "leonardo";
const char* mqttPassword = "90tr4dd3zqKeifermc12";

const char* topicTemperature = "/homeSecure/esp32/temperature";
const char* topicTemperatureDB = "/homeSecure/esp32/temperatureDB";
const char* topicUltrasonic = "/homeSecure/esp32/ultrasonic";
const char* topicSmoke = "/homeSecure/esp32/smoke";
const char* topicAlarm = "/homeSecure/esp32/alarm";



WiFiClient espClient;
PubSubClient client(espClient);

DHT dht(DHTPIN, DHTTYPE); 

//Definir todos los pines de conexión 

const int ledRed = 18;
const int ledGreen = 19;

const int buzzerPin = 2;
const int buttonPin = 5;

unsigned long previousTemperatureMillisDB = 0;
const unsigned long temperatureIntervalDB = 10000 * 60 * 60 * 2; //2hrs

unsigned long previousTemperatureMillis = 0;
const unsigned long temperatureInterval = 1000 * 5; //5s

/*//----Sensor ultrasónico

// Pin para el sensor ultrasónico
const int triggerPin = 18;
const int echoPin = 19;
// Pin para el buzzer

// Pin para el botón

// Distancia umbral en centímetros
const int distanciaUmbral = 30;
// Tiempo de espera en milisegundos para encender el buzzer
const unsigned long tiempoEspera = 5000;
// Variables para medir el tiempo
unsigned long tiempoInicio;
unsigned long tiempoActual;
// Objeto para el sensor ultrasónico
Ultrasonic ultrasonic(triggerPin, echoPin);*/



void startConection(){
  dht.begin();  
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

  while (!client.connected()){      

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Start MQTT...");


    Serial.println("Connecting to MQTT...");
       if (client.connect("ESP32Client", mqttUser, mqttPassword )){
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("connected");
          Serial.println("connected");
          client.subscribe(topicAlarm);
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

/*void ultrasonicoSensor(){
  float distancia = ultrasonic.read();

  if (digitalRead(buttonPin) == LOW) {
    // Si el botón está presionado, apagar el Lbuzzer
    noTone(buzzerPin);
    tiempoInicio = 0;
    Serial.println("LED apagado");
  } else if (distancia <= distanciaUmbral) {
    // Si la distancia es menor o igual al umbral, iniciar el temporizador
    if (tiempoInicio == 0) {
      tiempoInicio = millis();
    } else {
      tiempoActual = millis();
      unsigned long tiempoTranscurrido = tiempoActual - tiempoInicio;

      if (tiempoTranscurrido >= tiempoEspera) {
        // Si el tiempo transcurrido supera el tiempo de espera, encender el buzzer

        StaticJsonDocument<200> jsonDoc;

        // Añade datos al objeto JSON
        jsonDoc["data"] = "Alarma de movimiento activada";
        jsonDoc["status"] = "on";

        char buffer[200];
        serializeJson(jsonDoc, buffer);

        client.publish(topicUltrasonic, buffer);
        Serial.println("Datos enviados al broker: " + String(buffer));
        
        tone(buzzerPin, 1000);
        Serial.println("Alarma encendida");
        
      }
    }
  } else {
    // Si la distancia supera el umbral, reiniciar el temporizador
    tiempoInicio = 0;
  }

  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println(" cm");

}*/

void temperatureSensor(){
  unsigned long currentMillis = millis();
  if (currentMillis - previousTemperatureMillis >= temperatureInterval) {
    previousTemperatureMillis = currentMillis;

    
    float temperatura = dht.readTemperature();

    if (!isnan(temperatura)) {
      String tempFormated = "Temp: " + String(temperatura) + " " + (char)223 + "C";
      Serial.println(tempFormated);

      char tempStr[10];
      snprintf(tempStr, sizeof(tempStr), "%.2f", temperatura);

      lcd.setCursor(0, 0);
      lcd.print(tempFormated);

      client.publish(topicTemperature, tempStr);
    }
  }
}

void temperatureSensorDB(){
  unsigned long currentMillis = millis();
  if (currentMillis - previousTemperatureMillisDB >= temperatureIntervalDB) {
    previousTemperatureMillisDB = currentMillis;

    
    float temperatura = dht.readTemperature();

    if (!isnan(temperatura)) {
      String tempFormated = "Temp: " + String(temperatura) + " " + (char)223 + "C";
      Serial.println(tempFormated);

      char tempStr[10];
      snprintf(tempStr, sizeof(tempStr), "%.2f", temperatura);

      lcd.setCursor(0, 0);
      lcd.print(tempFormated);

      client.publish(topicTemperatureDB, tempStr);
    }
  }
}

void reproducirNotificacion() {
  tone(buzzerPin, 3000);  // Generar un tono de 1 kHz
  delay(100);  // Mantener el tono durante 100 milisegundos
  tone(buzzerPin, 5000);  // Generar un tono de 1 kHz
  delay(200);  // Mantener el tono durante 100 milisegundos
  noTone(buzzerPin);  // Detener el tono
  delay(200);  // Esperar 200 milisegundos antes de la próxima notificación (ajusta según tus necesidades)
}

void setup() { 
  Serial.begin(115200);
  
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledRed, OUTPUT); 
  pinMode(ledGreen, OUTPUT); 

  lcd.init();
  lcd.backlight();


  digitalWrite(ledRed, HIGH);
  startConection();
  digitalWrite(ledRed, LOW);
  digitalWrite(ledGreen, HIGH);
  
}  

void loop(){
  client.loop();
  //ultrasonicoSensor();
  temperatureSensor(); 
  temperatureSensorDB(); 
  

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

  if (String(topic) == topicAlarm) {
    if (message == "on") {
      tone(buzzerPin, 2000);
      lcd.setCursor(0, 1);
      lcd.print("Alarma activada");
      Serial.println("Alarma activada");
    } else if (message == "off") {
      noTone(buzzerPin);
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("Alarma desactivada");
      Serial.println("Alarma desactivada");
      reproducirNotificacion();
      delay(1500);
      lcd.setCursor(0, 1);
      lcd.print("                ");
      
    }
  }
}
