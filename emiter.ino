
// 0x807F08F7 Código de apagado / encendido

#include <IRremoteESP8266.h>
#include <IRsend.h>


const uint16_t kIrLedPin = 23;  // Pin del LED emisor infrarrojo
const uint16_t kPushButtonPin = 22;  // Pin del push button

IRsend irsend(kIrLedPin);

void setup() {
  Serial.begin(115200);
  irsend.begin();  // Iniciar el LED emisor infrarrojo
  pinMode(kPushButtonPin, INPUT_PULLUP);  // Configurar el push button como entrada con pull-up interno
  Serial.println("Emisor IR listo");
}

void loop() {
  if (digitalRead(kPushButtonPin) == LOW) {
    // Enviar el código IR para encender la televisión utilizando el LED emisor
    irsend.sendNEC(0x807F08F7, 32);  // Reemplaza XXXXXXXX con el código IR de encendido de tu televisión
    delay(500);  // Esperar 500 ms para evitar múltiples envíos mientras se mantiene presionado el botón
  }
}

