template<class T> inline Print& operator<<(Print& obj, T arg) {
  obj.print(arg);
  return obj;
}
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <driver/ledc.h>
#include "CTBot.h"
#include "Utilities.h"
#include <EEPROM.h>
#include <WiFi.h>
#include <ThingSpeak.h>
#define SEALEVELPRESSURE_HPA (1013.25)
#define EEPROM_SIZE 12
CTBot miBot;
CTBotInlineKeyboard miTeclado;
Adafruit_BME280 bme;
WiFiClient client;
#define RXD2 16
#define TXD2 17.

int Buzzer = 19;
int Led = 2;
int PIR = 14;
int UltrasonicTrigger = 13;
int UltrasonicEcho = 12;
int distancia = 0;
float tiempo = 0;
float espera = 60;
int movimientoContador = 0;
const int DireccionEstacion = 0;
boolean Estacion = true;
const int DireccionActivo = 1;
boolean Activo = true;

// Telegram configuracion
const char* ssid = "ProyectoFisica";
const char* password = "NEPTOR_bot0711";
const String token = "6760974551:AAFkFz0xHJQoe_hfij1peG7IzokqDyLiEqg";
int64_t IDchats[] = { 6742384195, 1477924319, 1327382118, 1213592950 };
int numIDchats = sizeof(IDchats) / sizeof(IDchats[0]);
const String nombre = "NEPTOR_bot Activado.";

// ThingSpeak configuracion
char thingSpeakAddress[] = "api.thingspeak.com";
unsigned long channelID = 2341025;
const char* writeAPIKey = "18DGITXF2JLL7F62";



void setup() {
  Serial.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println();
  Serial.println("Iniciando NEPTOR_bot de Telegram.");

  EEPROM.begin(EEPROM_SIZE);
  Serial.println("EEPROM Configurada (Datos de memoria restaurados.)");

  Activo = EEPROM.read(DireccionActivo);
  Serial.print("Sensor: ");
  Serial.println(Activo ? "Activada" : "Desactivada");

  Estacion = EEPROM.read(DireccionEstacion);
  Serial.print("Estacion Meteorológica: ");
  Serial.println(Estacion ? "Activada" : "Desactivada");

  unsigned status;
  status = bme.begin(0x76);
  pinMode(Led, OUTPUT);
  pinMode(Buzzer, OUTPUT);
  pinMode(PIR, INPUT_PULLUP);

  ledcSetup(0, 5000, 10);
  ledcAttachPin(Led, 0);

  miBot.wifiConnect(ssid, password);

  miBot.setTelegramToken(token);

  if (miBot.testConnection()) {
    Serial.println("\n NEPTOR_bot conectado a la red Wi-Fi.");
  } else {
    Serial.println("\n NEPTOR_bot no pudo conectarse, solicitando reinicio manual.");
  }
  miTeclado.addButton("Sensor", "sensor", CTBotKeyboardButtonQuery);
  miTeclado.addButton("Estacion", "estacion", CTBotKeyboardButtonQuery);
  miTeclado.addButton("Estado", "estado", CTBotKeyboardButtonQuery);
  miTeclado.addRow();
  miTeclado.addButton("Medir", "medir", CTBotKeyboardButtonQuery);
  miTeclado.addButton("Documentación", "https://publuu.com/flip-book/415331/938656", CTBotKeyboardButtonURL);

  for (int64_t chatID : IDchats) {
    miBot.sendMessage(chatID, "En Línea, Estación Meteorológica: " + nombre);
  }
  tiempo = -espera * 1000;

  // Inicializar ThingSpeak
  ThingSpeak.begin(client);
}

void loop() {
  EstacionMeteorologica();
  SistemaConfiguracion();
}

void SistemaConfiguracion() {
  TBMessage msg;

  if (miBot.getNewMessage(msg)) {
    // Itera sobre los ID de chat
    for (int i = 0; i < numIDchats; i++) {
      if (msg.sender.id == IDchats[i]) {
        if (msg.messageType == CTBotMessageText) {
          if (msg.text.equalsIgnoreCase("opciones")) {
            PedirEstado();
          } else {
            Serial.println("Enviar 'opciones'");
            miBot.sendMessage(msg.sender.id, "prueba 'opciones'");
          }
        } else if (msg.messageType == CTBotMessageQuery) {
          Serial << "Mensaje: " << msg.sender.firstName << "\n";
          if (msg.callbackQueryData.equals("sensor")) {
            Activo = !Activo;
            String Mensaje = "Sensor: ";
            Mensaje += (Activo ? "Activo" : "Apagado");
            Serial.println(Mensaje);
            miBot.endQuery(msg.callbackQueryID, Mensaje);
            EEPROM.put(DireccionActivo, Activo);
            EEPROM.commit();
          } else if (msg.callbackQueryData.equals("estacion")) {
            Estacion = !Estacion;
            String Mensaje = "Estacion: ";
            Mensaje += (Estacion ? "Activo" : "Apagado");
            Serial.println(Mensaje);
            miBot.endQuery(msg.callbackQueryID, Mensaje);
            EEPROM.put(DireccionEstacion, Estacion);
            EEPROM.commit();
          } else if (msg.callbackQueryData.equals("estado")) {
            PedirEstado();
          } else if (msg.callbackQueryData.equals("medir")) {
            MedirBME280();
          }
        }
      }
    }
  }
}

void PedirEstado() {
  Serial.println("Enviando 'opciones'");
  String Mensaje = "Estado Actual\n";
  Mensaje += "Sensor: ";
  Mensaje += (Activo ? "Activo" : "Apagado");
  Mensaje += " - Estacion: ";
  Mensaje += (Estacion ? "Activo" : "Apagado");
  Serial.println(Mensaje);
  // Envía mensajes a cada ID de chat
  for (int i = 0; i < numIDchats; i++) {
    miBot.sendMessage(IDchats[i], Mensaje);
    miBot.sendMessage(IDchats[i], "Cambiar", miTeclado);
  }
}

void EstacionMeteorologica() {
  if (Activo) {
    int pirValue = digitalRead(PIR);
    long distancia = 0.01723 * readUltrasonicDistance(UltrasonicTrigger, UltrasonicEcho);
    if (pirValue == HIGH || (distancia <= 20 and distancia > 0)) {
      if (distancia <= 20 and distancia > 0) {
        Serial.println("Movimiento detectado por el sensor PIR.");
        Serial.println("Objeto detectado por el sensor ultrasónico.");
        Serial.print("Distancia: ");
        Serial.println(distancia);
        if (Estacion) {
          ThingSpeak.setField(1, distancia);
          MedirBME280();
        }
      } else {
        noTone(Buzzer);
        digitalWrite(Buzzer, LOW);
      }
    } else {
      noTone(Buzzer);
      digitalWrite(Buzzer, LOW);
      Serial.println("Sin movimiento");
      delay(3000);
    }
  }
}

long readUltrasonicDistance(int UltrasonicTrigger,
                            int UltrasonicEcho) {
  pinMode(UltrasonicTrigger, OUTPUT);
  digitalWrite(UltrasonicTrigger, LOW);
  delayMicroseconds(2);
  digitalWrite(UltrasonicTrigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(UltrasonicTrigger, LOW);
  pinMode(UltrasonicEcho, INPUT);
  return pulseIn(UltrasonicEcho, HIGH);
}
void MedirBME280() {
  if (Estacion) {
    Serial.print("Temperatura = ");
    String recibido = Serial.readStringUntil('\n');
    Serial.println(recibido);
    Serial.print("Presión = ");
    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");
    Serial.print("Humedad = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");
    Serial.println();
    movimientoContador++;
    ThingSpeak.setField(2, movimientoContador);
    ThingSpeak.setField(3, recibido);
    ThingSpeak.setField(4, bme.readPressure());
    ThingSpeak.setField(5, bme.readAltitude(SEALEVELPRESSURE_HPA));
    ThingSpeak.setField(6, bme.readHumidity());
    int x = ThingSpeak.writeFields(channelID, writeAPIKey);
    if (x == 200) {
      Serial.println("Datos ingresados de manera exitosa.");
      tone(Buzzer, 2700);
      delay(1000);
      noTone(Buzzer);
      digitalWrite(Buzzer, LOW);
    } else {
      noTone(Buzzer);
      digitalWrite(Buzzer, LOW);
      Serial.println("Datos no ingresados.");
    }
    Serial.println("-------------");
    delay(3000);
  }
}
