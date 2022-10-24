#include <Arduino.h>
//#include <ESP8266WiFi.h>
#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "ArduinoHttpClient.h"
#include "Adafruit_MQTT_Client.h"
#include "WiFiClientSecure.h"
#include "DHT.h"
#include <Adafruit_Sensor.h>

#define PIN_RED    34 // GIOP23
#define PIN_GREEN  33 // GIOP22
#define PIN_BLUE   32 // GIOP21
#define BUTTON_PIN 12 // GIOP21 pin connected to button


// DHT 11 sensor
#define DHTPIN 26
#define DHTTYPE DHT11
// DHT sensor
DHT dht(DHTPIN, DHTTYPE, 15);


/// Lamp pin
const int lamp_pin = 2;
int sensorValue;
int State;
int R;
char text[20];

// WiFi parameters
#define WLAN_SSID       "cyrille"
#define WLAN_PASS       "12345678"

// Adafruit IO
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "GTCV"
#define AIO_KEY         "aio_bWmO015sGOHkeFNu806NeeLwOyA6"

// Functions
void connect();

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Store the MQTT server, client ID, username, and password in flash memory.
// This is required for using the Adafruit MQTT library.
const char MQTT_SERVER[] PROGMEM    = AIO_SERVER;
// Set a unique MQTT client ID using the AIO key + the date and time the sketch
// was compiled (so this should be unique across multiple devices for a user,
// alternatively you can manually set this to a GUID or other random value).
const char MQTT_CLIENTID[] PROGMEM  = AIO_KEY __DATE__ __TIME__;
const char MQTT_USERNAME[] PROGMEM  = AIO_USERNAME;
const char MQTT_PASSWORD[] PROGMEM  = AIO_KEY;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
//Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, AIO_SERVERPORT, MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD);

/****************************** Feeds ***************************************/

// Setup a feed called 'lamp' for subscribing to changes.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
const char LAMP_FEED[] PROGMEM = AIO_USERNAME "/feeds/LED";
Adafruit_MQTT_Subscribe LED = Adafruit_MQTT_Subscribe(&mqtt, LAMP_FEED);
const char LAMP_FEED_RED[] PROGMEM = AIO_USERNAME "/feeds/LEDR";
Adafruit_MQTT_Subscribe LEDR = Adafruit_MQTT_Subscribe(&mqtt, LAMP_FEED_RED);
const char LAMP_FEED_GREEN[] PROGMEM = AIO_USERNAME "/feeds/LEDG";
Adafruit_MQTT_Subscribe LEDG = Adafruit_MQTT_Subscribe(&mqtt, LAMP_FEED_GREEN);
const char LAMP_FEED_BLUE[] PROGMEM = AIO_USERNAME "/feeds/LEDB";
Adafruit_MQTT_Subscribe LEDB = Adafruit_MQTT_Subscribe(&mqtt, LAMP_FEED_BLUE);
Adafruit_MQTT_Publish Temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Temperature");
Adafruit_MQTT_Publish Humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Humidity");
Adafruit_MQTT_Publish LDR = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/LDR");
Adafruit_MQTT_Publish BP = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/BP");
/*Adafruit_MQTT_Publish LED = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/LED");
Adafruit_MQTT_Publish LEDR = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/LEDR");
Adafruit_MQTT_Publish LEDG = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/LEDG");
Adafruit_MQTT_Publish LEDB = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/LEDB");
Adafruit_MQTT_Publish BP = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/BP");*/


void setup() {

  dht.begin();
 
  pinMode(lamp_pin, OUTPUT);
  pinMode(PIN_RED,   OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE,  OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);



  Serial.begin(115200);

  Serial.println(F("Adafruit IO Example"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  delay(10);
  Serial.print(F("Connecting to "));
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();

  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());

  // listen for events on the lamp feed
  mqtt.subscribe(&LED);
  mqtt.subscribe(&LEDR);
  mqtt.subscribe(&LEDG);
  mqtt.subscribe(&LEDB);

  // connect to adafruit io
  connect();

}

void loop() {

  Adafruit_MQTT_Subscribe *subscription;

  // ping adafruit io a few times to make sure we remain connected
  if(! mqtt.ping(3)) {
    // reconnect to adafruit io
    if(! mqtt.connected())
      connect();
  }

  // this is our 'wait for incoming subscription packets' busy subloop
  while (subscription = mqtt.readSubscription(1000)) {

    // we only care about the lamp events
    if (subscription == &LED) {

      // convert mqtt ascii payload to int
      char *value = (char *)LED.lastread;
      Serial.print(F("Received: "));
      Serial.println(value);

      // Apply message to lamp
      String message = String(value);
      message.trim();
      if (message == "ON") {digitalWrite(lamp_pin, HIGH);}
      if (message == "OFF") {digitalWrite(lamp_pin, LOW);}

    }

    
    if (subscription == &LEDR) {
      
      // convert mqtt ascii payload to int
      char *valueR = (char *)LEDR.lastread;
      Serial.print(F("Received: "));
      Serial.println(valueR);

      // Apply message to lamp
      String message = String(valueR);
      message.trim();
      if (message) {digitalWrite(PIN_RED, HIGH);}
      //if (message == "OFF") {digitalWrite(lamp_pin, LOW);}

    }

    if (subscription == &LEDG) {
     
      // convert mqtt ascii payload to int
      char *valueG = (char *)LEDG.lastread;
      Serial.print(F("Received: "));
      Serial.println(valueG);

      // Apply message to lamp
      String message = String(valueG);
      message.trim();
      if (message) {digitalWrite(PIN_GREEN, HIGH);}
      //if (message == "OFF") {digitalWrite(lamp_pin, LOW);}

    }

    if (subscription == &LEDB) {
     
      // convert mqtt ascii payload to int
      char *valueB = (char *)LEDB.lastread;
      Serial.print(F("Received: "));
      Serial.println(valueB);

      // Apply message to lamp
      String message = String(valueB);
      message.trim();
      if (message) {digitalWrite(PIN_BLUE, HIGH);}
      //if (message == "OFF") {digitalWrite(lamp_pin, LOW);}

    }

  }

  // Grab the current state of the sensor
  int humidity_data = (int)dht.readHumidity();
  int temperature_data = (int)dht.readTemperature();
  sensorValue = analogRead(35); // read analog input pin 0
  State = digitalRead(BUTTON_PIN);

  // By default, the temperature report is in Celsius, for Fahrenheit uncomment
  //    following line.
  // temperature_data = temperature_data*(9.0/5.0) + 32.0;

  // Publish data
  if (! Temperature.publish(temperature_data))
    Serial.println(F("Failed to publish temperature"));
  else
    Serial.println(F("Temperature published!"));

  if (! Humidity.publish(humidity_data))
    Serial.println(F("Failed to publish humidity"));
  else
    Serial.println(F("Humidity published!"));
  
  if (! LDR.publish(sensorValue))
    Serial.println(F("Failed to publish LDR"));
  else
    Serial.println(F("LDR published!"));
  
  if (! BP.publish(State))
    Serial.println(F("Failed to publish LDR"));
  else
    Serial.println(F("BP published!"));

  // Repeat every 10 seconds
  delay(15000);

}

// connect to adafruit io via MQTT
void connect() {

  Serial.print(F("Connecting to Adafruit IO... "));

  int8_t ret;

  while ((ret = mqtt.connect()) != 0) {

    switch (ret) {
      case 1: Serial.println(F("Wrong protocol")); break;
      case 2: Serial.println(F("ID rejected")); break;
      case 3: Serial.println(F("Server unavail")); break;
      case 4: Serial.println(F("Bad user/pass")); break;
      case 5: Serial.println(F("Not authed")); break;
      case 6: Serial.println(F("Failed to subscribe")); break;
      default: Serial.println(F("Connection failed")); break;
    }

    if(ret >= 0)
      mqtt.disconnect();

    Serial.println(F("Retrying connection..."));
    delay(5000);

  }

  Serial.println(F("Adafruit IO Connected!"));

}