#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Ultrasonic.h>
#include "LiquidCrystal_I2C.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

const char *ssid = "motorola";  // Totalplay-12A2
const char *password = "904390439"; // 12A28E0FNtF9Qfmv
const char *mqttServer = "homesecuremqtt.ddns.net";
const int mqttPort = 1883;
const char *mqttUser = "leonardo";
const char *mqttPassword = "90tr4dd3zqKeifermc12";

const char *topicUltrasonic = "/homeSecure/esp32/ultrasonic";
const char *topicSmoke = "/homeSecure/esp32/smoke";
const char *topicAlarm = "/homeSecure/esp32/alarm";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long previousUltrasonicMillis = 0;
const unsigned long ultrasonicInterval = 1000;

// Pin para el sensor ultrasónico
const int triggerPin = 18;
const int echoPin = 19;

// Pin para el sensor de humo
const int smokePin = 34;
int smokeThreshold = 500; // Umbral de detección de humo
unsigned long previousSmokeMillis = 0;
const unsigned long smokeInterval = 1000;

const unsigned long smokeCooldownTime = 1 * 60 * 1000; // 1 minutos en milisegundos
bool smokeDetected = false;
unsigned long lastSmokeTime = 0;

// Pin para el buzzer
const int buzzerPin = 2;

// Pin para el botón
const int buttonPin = 5;

// Distancia umbral en centímetros
const int distanciaUmbral = 30;

// Tiempo de espera en milisegundos para encender el buzzer
const unsigned long tiempoEspera = 5000;

// Variables para medir el tiempo
unsigned long tiempoInicio;
unsigned long tiempoActual;

bool buzzerActivo = false;

// Objeto para el sensor ultrasónico
Ultrasonic ultrasonic(triggerPin, echoPin);

const int ledRed = 15;
const int ledGreen = 23;

void reconnect() {

  while (!client.connected()){      

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Start MQTT...");


    Serial.println("Connecting to MQTT...");
    delay(1000);
       if (client.connect("ESP32Client_Detector", mqttUser, mqttPassword )){
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

void reproducirNotificacion()
{
  tone(buzzerPin, 3000); // Generar un tono de 1 kHz
  delay(100);            // Mantener el tono durante 100 milisegundos
  tone(buzzerPin, 5000); // Generar un tono de 1 kHz
  delay(200);            // Mantener el tono durante 100 milisegundos
  noTone(buzzerPin);     // Detener el tono
  delay(200);            // Esperar 200 milisegundos antes de la próxima notificación (ajusta según tus necesidades)
}

void activeBuzerByUltrasonic()
{
  buzzerActivo = true;
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["data"] = "Alarma de movimiento activada";
  jsonDoc["status"] = "on";

  char buffer[200];
  serializeJson(jsonDoc, buffer);

  client.publish(topicUltrasonic, buffer);

  tone(buzzerPin, 500);
  Serial.println("Alarma encendida");
}

void activeBuzerBySmoke()
{
  buzzerActivo = true;
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["data"] = "Alarma de humo activada";
  jsonDoc["status"] = "on";

  char buffer[200];
  serializeJson(jsonDoc, buffer);

  client.publish(topicSmoke, buffer);

  tone(buzzerPin, 500);
  Serial.println("Alarma de humo encendida");
}

void disableBuzzerByUltrasonic()
{
  buzzerActivo = false;
  noTone(buzzerPin);
  tiempoInicio = 0;

  StaticJsonDocument<200> jsonDoc;
  jsonDoc["data"] = "Alarma de movimiento desactivada";
  jsonDoc["status"] = "off";

  char buffer[200];
  serializeJson(jsonDoc, buffer);

  client.publish(topicUltrasonic, buffer);

  Serial.println("El buzer ha sido apagado");

  reproducirNotificacion();
}

void disableBuzzerBySmoke()
{
  buzzerActivo = false;
  noTone(buzzerPin);
  tiempoInicio = 0;

  StaticJsonDocument<200> jsonDoc;
  jsonDoc["data"] = "Alarma de humo desactivada";
  jsonDoc["status"] = "off";

  char buffer[200];
  serializeJson(jsonDoc, buffer);

  client.publish(topicSmoke, buffer);

  Serial.println("El buzer ha sido apagado");

  reproducirNotificacion();
}

void ultrasonicoSensor()
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousUltrasonicMillis >= ultrasonicInterval)
  {
    previousUltrasonicMillis = currentMillis;

    float distancia = ultrasonic.read();

    if (digitalRead(buttonPin) == LOW)
    {
      // apagar el Lbuzzer
      if (buzzerActivo)
      {

        disableBuzzerByUltrasonic();
      }
    }
    else if (distancia <= distanciaUmbral)
    {
      // iniciar el temporizador
      if (tiempoInicio == 0)
      {
        tiempoInicio = millis();
      }
      else
      {
        tiempoActual = millis();
        unsigned long tiempoTranscurrido = tiempoActual - tiempoInicio;

        if (tiempoTranscurrido >= tiempoEspera)
        {
          // encender el buzzer

          if (!buzzerActivo)
          {

            activeBuzerByUltrasonic();
          }
        }
      }
    }
    else
    {
      // reiniciar el temporizador
      tiempoInicio = 0;
    }

    String mensajeDistacia = String(distancia) + " cm";

    Serial.println(mensajeDistacia);
  }
}

void smokeSensor()
{
  unsigned long currentMillis = millis();

  if (currentMillis - lastSmokeTime >= smokeCooldownTime)
  {
    if (currentMillis - previousSmokeMillis >= smokeInterval)
    {
      previousSmokeMillis = currentMillis;

      int smokeValue = analogRead(smokePin);

      if (digitalRead(buttonPin) == LOW)
      {
        // apagar el Lbuzzer
        if (buzzerActivo)
        {

          disableBuzzerByUltrasonic();
        }
      }

      Serial.print("Valor del sensor de humo: ");
      Serial.println(smokeValue);

      if (smokeValue > smokeThreshold)
      {
        activeBuzerBySmoke();
        smokeDetected = true;
        lastSmokeTime = currentMillis;
      }
    }
  }
  else
  {
    if (currentMillis - previousSmokeMillis >= smokeInterval)
    {
      previousSmokeMillis = currentMillis;

      Serial.println("Reinciando sensor de humo");
    }
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(smokePin, INPUT);

  lcd.init();
  lcd.backlight();

  digitalWrite(ledRed, HIGH);
  startConection();
  digitalWrite(ledRed, LOW);
  digitalWrite(ledGreen, HIGH);
}

void loop()
{

  if (!client.connected()) {
    digitalWrite(ledRed, HIGH);
    reconnect();
    digitalWrite(ledRed, LOW);
    digitalWrite(ledGreen, HIGH);
  }
  client.loop();

  ultrasonicoSensor();
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
      if(buzzerActivo == false){
      tone(buzzerPin, 500);
      lcd.setCursor(0, 1);
      lcd.print("Alarma activada");
      Serial.println("Alarma activada");
      buzzerActivo = true;
      }
    } else if (message == "off") {
      if(buzzerActivo == true){
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
      buzzerActivo = false;
      }
    }
  }
}