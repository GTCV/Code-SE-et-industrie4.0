#include <FS.h>
#include <DHT.h>
#include <WiFi.h>
#include <Arduino.h>
#include <DNSServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>


// Update these with values suitable for your network.

const char* ssid = "cyrille";
const char* password = "12345678";
const char* mqtt_server = "10.22.2.52";
uint32_t color;
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
int sensorValue;
String ETATRGB;
String ETATBP;
String ETATLED;

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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
  }
   Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == 'O' && (char)payload[1] == 'N') {
    digitalWrite(BUILTIN_LED, HIGH); 
  } 
  else if ((char)payload[0] == 'O' && (char)payload[1] == 'F' && (char)payload[2] == 'F')
  {
    digitalWrite(BUILTIN_LED, LOW);
  }
  if ((char)topic[0] == 'R' && (char)topic[1] == 'E' && (char)topic[2] == 'D')
  {
    //analogWrite(PIN_RED,(int)payload);
    int RedValue = atoi((const char *)payload);
    analogWrite(PIN_RED,RedValue);
    Serial.println("rouge");
    Serial.println(RedValue);
  }
  else if ((char)topic[0] == 'G' && (char)topic[1] == 'R' && (char)topic[2] == 'E' && (char)topic[3] == 'E' && (char)topic[4] == 'N')
  {
    //analogWrite(PIN_GREEN, (int)payload);
    int greenValue = atoi((const char *)payload);
    analogWrite(PIN_GREEN,greenValue);
    Serial.println("vert");
    Serial.println(greenValue);

  }
  else if ((char)topic[0] == 'B' && (char)topic[1] == 'L' && (char)topic[2] == 'U' && (char)topic[3] == 'E')
  {
    //analogWrite(PIN_BLUE, (int)payload);
    int blueValue = atoi((const char *)payload);
    analogWrite(PIN_BLUE,blueValue);
    Serial.println("blue");
    Serial.println(blueValue);

  }
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
      client.subscribe("RED");
      client.subscribe("GREEN");
      client.subscribe("BLUE");
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  
  Serial.begin(115200);
  dht.begin();
  pinMode(PIN_RED,   OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE,  OUTPUT);
  pinMode(START, INPUT_PULLUP);
  pinMode(STOP, INPUT_PULLUP);
  pinMode(LED, OUTPUT);  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  Serial.println("Started");
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(analogRead(0));
  Serial.println("TP3 GoogleSheets ready...");
}
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;
String Temp, Hum, strParameter;

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
    
    int humidity = dht.readHumidity();
    int temperature = dht.readTemperature();
    sensorValue = analogRead(33);
    bool flag;
     //sensorValue = analogRead(33);
    DynamicJsonDocument doc(256);
    if (!digitalRead(START) && !flag){
       digitalWrite(LED, HIGH);
       flag = 1;
       ETATBP = "ACTIF";
       ETATLED = "ON";
    }
    if (!digitalRead(STOP) && flag){
       digitalWrite(LED, LOW);
       flag = 0;
       ETATBP = "NOT ACTIF";
       ETATLED = "OFF";
    }
    if (25 < temperature && temperature < 28)
    {
      digitalWrite(PIN_GREEN, HIGH);
      digitalWrite(PIN_BLUE, LOW);
      digitalWrite(PIN_RED, LOW);
      ETATRGB = "BON";
      
    }
    else if (temperature < 25)
    {
      digitalWrite(PIN_BLUE, HIGH);
      digitalWrite(PIN_GREEN, LOW);
      digitalWrite(PIN_RED, LOW);
      ETATRGB = "INFERIEUR";
      
    }
    else if (temperature > 28)
    {
      digitalWrite(PIN_RED, HIGH);
      digitalWrite(PIN_GREEN, LOW);
      digitalWrite(PIN_BLUE, LOW);
      ETATRGB = "SUPERIEUR";
      
    }
    
    
    doc["ETAT"] = ETATRGB;
    doc["LED"] = ETATLED;
    doc["STARTSTOP"] =  ETATBP;
    doc["TEMPERATURE"] = temperature;
    doc["HUMIDITY"] = humidity;
    doc["LDR"] = sensorValue;
  
  char json_string[256];
  serializeJson(doc, json_string);
  client.publish("GAUGE", json_string);
  

 }
}