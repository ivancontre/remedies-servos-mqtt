#include "variables.h"

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include <HTTPClient.h>

HTTPClient http;
WiFiClient espClient;
PubSubClient client(espClient);

// Servos init
Servo servoMondayA;
const int pinServoMondayA = 22;
Servo servoMondayB;
const int pinServoMondayB = 21;

Servo servoTuesdayA;
const int pinServoTuesdayA = 19;
Servo servoTuesdayB;
const int pinServoTuesdayB = 18;

Servo servoWednesdayA;
const int pinServoWednesdayA = 17;
Servo servoWednesdayB;
const int pinServoWednesdayB = 16; //25

Servo servoThursdayA;
const int pinServoThursdayA = 32;
Servo servoThursdayB;
const int pinServoThursdayB = 33;

Servo servoFridayA;
const int pinServoFridayA = 23;
Servo servoFridayB;
const int pinServoFridayB = 26;

Servo servoSaturdayA;
const int pinServoSaturdayA = 27;
Servo servoSaturdayB;
const int pinServoSaturdayB = 14;

Servo servoSundayA;
const int pinServoSundayA = 13;
Servo servoSundayB;
const int pinServoSundayB = 12;

const int pinLedWifi = 4;
const int pinLedMqtt = 5;

void setup() {
  Serial.begin(9600);

  pinMode(pinLedWifi, OUTPUT);
  pinMode(pinLedMqtt, OUTPUT);

  attachServos();

  connectWifi();  

  getStatus();

  connectMqtt();
  
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void connectWifi() {
  // connecting to a WiFi network
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     digitalWrite(pinLedMqtt, LOW);
     digitalWrite(pinLedWifi, LOW);
     Serial.println("Connecting to WiFi..");
  }
  digitalWrite(pinLedWifi, HIGH);
  
  Serial.println("Connected to the WiFi network");
}

String generarStringAleatorio(int longitud) {
    // Caracteres posibles en el string aleatorio
    const char caracteresPosibles[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    int i;
    int maximoIndice = sizeof(caracteresPosibles) - 1;

    // Inicializar la semilla para los números aleatorios
    randomSeed(analogRead(0));

    // Crear un objeto String para almacenar el string aleatorio
    String stringAleatorio = "";

    // Generar el string aleatorio
    for (i = 0; i < longitud; ++i) {
        int indiceAleatorio = random(0, maximoIndice);
        stringAleatorio += caracteresPosibles[indiceAleatorio];
    }

    return stringAleatorio;
}

void connectMqtt() {
    //connecting to a mqtt broker
   client.setServer(mqtt_broker, mqtt_port);
   client.setCallback(callback);
   while (!client.connected()) {
       String client_id = idEsp32;
       Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
       if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
          digitalWrite(pinLedMqtt, HIGH);
          //updateConnectionOnline(true);
          Serial.println("Public emqx mqtt broker connected");
       } else {
          digitalWrite(pinLedMqtt, LOW);
          //updateConnectionOnline(false);
          Serial.println("failed with state ");
          Serial.print(client.state());
          delay(5000);
       }
   }
   // publish and subscribe
   //client.publish(topic, "Hi EMQX I'm ESP32 ^^");
   client.subscribe(topic);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando reconectarse MQTT...");
    String client_id = idEsp32;
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      digitalWrite(pinLedMqtt, HIGH);
      //updateConnectionOnline(true);
      Serial.println("Public emqx mqtt broker connected");
          
    } else {
      digitalWrite(pinLedMqtt, LOW);
      //updateConnectionOnline(false);
      Serial.print("Fallo, rc=");
      Serial.print(client.state());
      Serial.println(" intentar de nuevo en 5 segundos");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }

  client.subscribe(topic);
}

void attachServos() {
  servoMondayA.attach(pinServoMondayA);
  servoMondayB.attach(pinServoMondayB);

  servoTuesdayA.attach(pinServoTuesdayA);
  servoTuesdayB.attach(pinServoTuesdayB);

  servoWednesdayA.attach(pinServoWednesdayA);
  servoWednesdayB.attach(pinServoWednesdayB);

  servoThursdayA.attach(pinServoThursdayA);
  servoThursdayB.attach(pinServoThursdayB);

  servoFridayA.attach(pinServoFridayA);
  servoFridayB.attach(pinServoFridayB);

  servoSaturdayA.attach(pinServoSaturdayA);
  servoSaturdayB.attach(pinServoSaturdayB);

  servoSundayA.attach(pinServoSundayA);
  servoSundayB.attach(pinServoSundayB);
}

void updateConnectionOnline(bool isOnline) {
  Serial.print("HTTP actualizando online");
  http.useHTTP10(true);
  http.begin("https://" + urlAPI + "/api/auth/" + idEsp32 + "/online"); //HTTP
  http.addHeader("Content-Type", "application/json");
  http.addHeader("x-token", tokenESP32);

  String jsonPayload = isOnline ? "{\"online\": true}" : "{\"online\": false}";
  int httpResponseCode = http.PUT(jsonPayload);

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");

    // Read the response
    String response = http.getString();
    Serial.println(response);
  } else {
    Serial.print("Error in HTTP request. Code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}


void getStatus() {
  http.useHTTP10(true);
  http.begin("https://" + urlAPI + "/api/v2/status"); //HTTP
  http.addHeader("x-token", tokenESP32);
  int httpCode = http.GET();

  if(httpCode > 0) {
    Serial.printf("[HTTP] GET. code: %d\n", httpCode);
    if(httpCode == HTTP_CODE_OK) {
      DynamicJsonDocument doc(2048);
      deserializeJson(doc, http.getString());
      JsonArray jsonArray = doc.as<JsonArray>();

      for (JsonVariant value : jsonArray) {
        String day = value["day"].as<String>();
        bool enabledAM = value["enabledAM"].as<bool>();
        bool enabledPM = value["enabledPM"].as<bool>();
        initServos(day, enabledAM, enabledPM);
      }      
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}

void initServos(String day, bool enabledAM, bool enabledPM) {
  int dayNum = getDayNumber(day);
  switch(dayNum) {
    case 1:
      enabledAM ? servoMondayA.write(0) : servoMondayA.write(90);
      enabledPM ? servoMondayB.write(0) : servoMondayB.write(90);
      break;
    case 2:
      enabledAM ? servoTuesdayA.write(0) : servoTuesdayA.write(90);
      enabledPM ? servoTuesdayB.write(0) : servoTuesdayB.write(90);
      break;
    case 3:
      enabledAM ? servoWednesdayA.write(0) : servoWednesdayA.write(90);
      enabledPM ? servoWednesdayB.write(0) : servoWednesdayB.write(90);
      break;
    case 4:
      enabledAM ? servoThursdayA.write(0) : servoThursdayA.write(90);
      enabledPM ? servoThursdayB.write(0) : servoThursdayB.write(90);
      break;
    case 5:
      enabledAM ? servoFridayA.write(0) : servoFridayA.write(90);
      enabledPM ? servoFridayB.write(0) : servoFridayB.write(90);
      break;
    case 6:
      enabledAM ? servoSaturdayA.write(0) : servoSaturdayA.write(90);
      enabledPM ? servoSaturdayB.write(0) : servoSaturdayB.write(90);
      break;
    case 7:
      enabledAM ? servoSundayA.write(0) : servoSundayA.write(90);
      enabledPM ? servoSundayB.write(0) : servoSundayB.write(90);
      break;
  }  
}

void changeServos(String day, String key, bool value) {
  int dayNum = getDayNumber(day);
  switch(dayNum) {
    case 1:
      if (key == "enabledAM") {
        value ? servoMondayA.write(0) : servoMondayA.write(90);
      } else{
        value ? servoMondayB.write(0) : servoMondayB.write(90);
      }
      break;
    case 2:
      if (key == "enabledAM") {
        value ? servoTuesdayA.write(0) : servoTuesdayA.write(90);
      } else{
        value ? servoTuesdayB.write(0) : servoTuesdayB.write(90);
      }
      break;
    case 3:
      if (key == "enabledAM") {
        value ? servoWednesdayA.write(0) : servoWednesdayA.write(90);
      } else{
        value ? servoWednesdayB.write(0) : servoWednesdayB.write(90);
      }
      break;
    case 4:
      if (key == "enabledAM") {
        value ? servoThursdayA.write(0) : servoThursdayA.write(90);
      } else{
        value ? servoThursdayB.write(0) : servoThursdayB.write(90);
      }
      break;
    case 5:
      if (key == "enabledAM") {
        value ? servoFridayA.write(0) : servoFridayA.write(90);
      } else{
        value ? servoFridayB.write(0) : servoFridayB.write(90);
      }
      break;
    case 6:
      if (key == "enabledAM") {
        value ? servoSaturdayA.write(0) : servoSaturdayA.write(90);
      } else{
        value ? servoSaturdayB.write(0) : servoSaturdayB.write(90);
      }
      break;
    case 7:
      if (key == "enabledAM") {
        value ? servoSundayA.write(0) : servoSundayA.write(90);
      } else{
        value ? servoSundayB.write(0) : servoSundayB.write(90);
      }
      break;
  }  
}

int getDayNumber(String day) {
  if (day == "LUNES") return 1;
  if (day == "MARTES") return 2;
  if (day == "MIÉRCOLES") return 3;
  if (day == "JUEVES") return 4;
  if (day == "VIERNES") return 5;
  if (day == "SÁBADO") return 6;
  if (day == "VIERNES") return 7;
}

void callback(char *topic, byte *payload, unsigned int length) {
   Serial.print("Message arrived in topic: ");
   Serial.println(topic);
   Serial.print("Message:");
  
   size_t payloadSize = sizeof(payload) - 1;
   DynamicJsonDocument doc(payloadSize + 256);
   DeserializationError error = deserializeJson(doc, payload);

  if (error) {
      Serial.print(F("Error al deserializar JSON: "));
      Serial.println(error.c_str());
      return;
  }

  String day = doc["day"];
  String key = doc["key"];
  bool value = doc["value"].as<bool>();

  changeServos(day, key, value);
  Serial.println(day);
  Serial.println(key);
  Serial.println(value);
  Serial.println("-----------------------");
}
