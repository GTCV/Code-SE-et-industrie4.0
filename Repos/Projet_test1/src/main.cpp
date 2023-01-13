#include <FS.h>
#include <DHT.h>
#include <WiFi.h>
#include <Arduino.h>
#include <DNSServer.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <BlynkSimpleEsp32.h>
#include <addons/RTDBHelper.h>
#include <addons/TokenHelper.h>

// Update these with values suitable for your network.
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPLTAgkUQEQ"
#define BLYNK_DEVICE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "Qvj23f5eJuUV2a7-dLkj1EqFBGIMbom-"
#define API_KEY "AIzaSyDbIVVKjVtGeEv8Ek9WUfRQnBVr303M0bQ"
#define DATABASE_URL "https://projet-6a194-default-rtdb.europe-west1.firebasedatabase.app/" 

//const char* auth = BLYNK_AUTH_TOKEN;
const char* ssid = "cyrille";
const char* password = "12345678";
const char* mqtt_server = "192.168.0.162";
uint32_t color;
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
int sensorValue;
String ETATRGB;

#define LED 2 // GIOP2 pin connected to button
#define STOP 13 // GIOP13 pin connected to button
#define START 12 // GIOP12 pin connected to button
#define DHTPIN 26 //GIOP12 pin connected to button
#define BUZZER  17 // GIOP17 pin connected to button
#define PIN_RED  16 // GIOP16 pin connected to button
#define PIN_BLUE  18 // GIOP18 pin connected to button
#define PIN_GREEN  19 // GIOP19 pin connected to button
#define DHTTYPE     DHT11

// DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// Define Firebase Data object
FirebaseData stream;
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

int count = 0;

volatile bool dataChanged = false;

void streamCallback(FirebaseStream data)
{
  Serial.printf("sream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                data.streamPath().c_str(),
                data.dataPath().c_str(),
                data.dataType().c_str(),
                data.eventType().c_str());
  printResult(data); // see addons/RTDBHelper.h
  Serial.println();
  Serial.printf("Received stream payload size: %d (Max. %d)\n\n", data.payloadLength(), data.maxPayloadLength());
  
  FirebaseJsonData result;

if (data.jsonObject().get(result, "LED")) {
  digitalWrite(LED,(bool)result.boolValue);
}
if (data.jsonObject().get(result, "LED")) {
  digitalWrite(LED,(bool)result.boolValue);
}
  Serial.println(data.jsonString());
  // Due to limited of stack memory, do not perform any task that used large memory here especially starting connect to server.
  // Just set this flag and check it status later.
  dataChanged = true;
}

void streamTimeoutCallback(bool timeout)
{
  if (timeout)
    Serial.println("stream timed out, resuming...\n");

  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}
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


 bool flag;
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
  }
   Serial.println();

}

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
      // ... and resubscribe
      client.subscribe("LED");
    
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

//Récupérer état du bouton
 BLYNK_WRITE(V0){
 digitalWrite(LED, param.asInt());
}
void setup() {
  
  dht.begin();
  pinMode(PIN_RED,   OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE,  OUTPUT);
  pinMode(START, INPUT_PULLUP);
  pinMode(STOP, INPUT_PULLUP);
  pinMode(LED, OUTPUT);  
  WiFi.mode(WIFI_STA);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password, "blynk.cloud", 80);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  Firebase.begin(&config, &auth);

  Firebase.reconnectWiFi(true);

   
    Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());

  Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);
  
}


void loop() {
      
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Grab the current state of the sensor
   unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    
    sensorValue = analogRead(33);
    
    DynamicJsonDocument doc(256);
     Blynk.run();
     int humidity = dht.readHumidity();
     int temperature = dht.readTemperature();

    // Switch on the LED if an 1 was received as first character
    if (!digitalRead(START) && !flag){
       digitalWrite(LED, HIGH);
       flag = 1;
    }
    //if (isnan(event.relative_humidity))
    if (!digitalRead(STOP) && flag){
       digitalWrite(LED, LOW);
       flag = 0;
    }
    
    if (25 < temperature && temperature < 28)
    {
      digitalWrite(PIN_GREEN, HIGH);
      digitalWrite(PIN_BLUE, LOW);
      digitalWrite(PIN_RED, LOW);
      digitalWrite(BUZZER, LOW);
      ETATRGB = "BON";
      
    }
    else if (temperature < 25)
    {
      digitalWrite(PIN_BLUE, HIGH);
      digitalWrite(PIN_GREEN, LOW);
      digitalWrite(PIN_RED, LOW);
      digitalWrite(BUZZER, LOW);
      ETATRGB = "INFERIEUR";
      
    }
    else if (temperature > 28)
    {
      digitalWrite(PIN_RED, HIGH);
      digitalWrite(BUZZER, HIGH);
      digitalWrite(PIN_GREEN, LOW);
      digitalWrite(PIN_BLUE, LOW);
      ETATRGB = "SUPERIEUR";
      
    }
    
    
    doc["ETAT"] = ETATRGB;
    doc["LED"] = digitalRead(LED) ? "ON" : "OFF";
    doc["STARTSTOP"] = digitalRead(LED) ? "ACTIF" : "NOT ACTIF";
    doc["TEMPERATURE"] = temperature;
    doc["HUMIDITY"] = humidity;
    doc["LDR"] = sensorValue;
  
    char json_string[256];
    serializeJson(doc, json_string);
    client.publish("GAUGE", json_string); 
  }

  if (dataChanged)
  {
    dataChanged = false;
    // When stream data is available, do anything here...
  }

}