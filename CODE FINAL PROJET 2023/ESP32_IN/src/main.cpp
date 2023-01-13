//Définition des librairies
#include <WiFi.h>
#include <DHT.h>
#include <Arduino.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_Sensor.h>

//Identifiant de connection wifi entre l'esp32 et le pc
const char* ssid = "cyrille";
const char* password = "12345678";
const char* mqtt_server = "192.168.129.3";

uint32_t color;
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;

//Déclaration des entiers
int value = 0;

//Définition des différentes pins
#define STOP 13 
#define START 12 
#define DHTPIN 26 
#define BUZZER  17 
#define DHTTYPE     DHT11

// DHT sensor
DHT dht(DHTPIN, DHTTYPE);

unsigned long sendDataPrevMillis = 0;

//Fonction qui affiche dans le moniteur si l'on est connecté ou pas
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//Cette fonction permet une connection entre l'ESP32_IN, l'ESP32_OUT et le wifi du PC
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
     
    
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//Cette fonction permet d'initialiser les diiférents composants à utiliser
void setup() {
  
  dht.begin();
  pinMode(START, INPUT);
  pinMode(STOP, INPUT);
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  
}

//Cette fonction permet d'écrire le code que nous voulons mettre en évidence
void loop() {

  //Cette condition vérifie si le client est connecté ou pas
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  int humidity = dht.readHumidity();
  int temperature = dht.readTemperature();
  int sensorValue = analogRead(39);
  // Grab the current state of the sensor
   unsigned long now = millis();
  
   
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;

    //Cette partie de code permet d'envoyer les informations au node-red
    DynamicJsonDocument doc(256);

    doc["START"] = digitalRead(START);
    doc["STOP"] = digitalRead(STOP);
    doc["TEMPERATURE"] = temperature;
    doc["HUMIDITY"] = humidity;
    doc["LDR"] = sensorValue;
  
    char json_string[256];
    serializeJson(doc, json_string);
    client.publish("ESP32_IN", json_string); 
  
  }

}