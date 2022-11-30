
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <Firebase_ESP_Client.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <WiFiClientSecure.h>
#include <Update.h>
#include <SPIFFS.h>
#include <ETH.h>
#include <SD.h>


// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "cyrille"
#define WIFI_PASSWORD "12345678"

// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "AIzaSyDII3jFZzc9KVu4FIpnT5gBWVY1_JN9dA8"

/* 3. Define the RTDB URL */
#define DATABASE_URL "https://tp8-firebase-813f0-default-rtdb.europe-west1.firebasedatabase.app/" 

#define DHTPIN 26
#define LED1_PIN  2
#define LED2_PIN  18
#define BUTTON_PIN 12
#define DHTTYPE DHT11
#define SCREEN_WIDTH 128 // OLED width,  in pixels
#define SCREEN_HEIGHT 64 // OLED height, in pixels
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

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

if (data.jsonObject().get(result, "LED1")) {
  digitalWrite(LED1_PIN,(bool)result.boolValue);
  if ((bool)result.boolValue == 1){
    oled.setCursor(1, 18);       // set position to display
    oled.println("LED1 :ON"); // set text
    oled.display();              // display on OLED
  }

  else{
    oled.setCursor(1, 18);       // set position to display
    oled.println("LED1 : OFF"); // set text
    oled.display();              // display on OLED
  }
  //Serial.println((bool)result.boolValue);
  
}
if (data.jsonObject().get(result, "LED2")) {
  digitalWrite(LED2_PIN,(bool)result.boolValue);
  if ((bool)result.boolValue == 1){
    oled.setCursor(1, 27);       // set position to display
    oled.println("LED2 :ON"); // set text
    oled.display();              // display on OLED
  }

  else{
    oled.setCursor(1, 27);       // set position to display
    oled.println("LED2 : OFF"); // set text
    oled.display();              // display on OLED
   }
  
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

void setup()
{

  dht.begin();
  Serial.begin(9600);

  // initialize OLED display with I2C address 0x3C
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("failed to start SSD1306 OLED"));
    while (1);
  }

  delay(2000);         // wait two seconds for initializing
  oled.clearDisplay(); // clear display
  oled.setTextSize(1);         // set text size
  oled.setTextColor(WHITE);    // set text color
  oled.setCursor(10, 1);       // set position to display
  oled.println("TP8 FIREBASE"); // set text
  oled.display();              // display on OLED
 
  Serial.begin(115200);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
  
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  Firebase.begin(&config, &auth);

  Firebase.reconnectWiFi(true);

// Recommend for ESP8266 stream, adjust the buffer size to match your stream data size
#if defined(ESP8266)
  stream.setBSSLBufferSize(2048 /* Rx in bytes, 512 - 16384 */, 512 /* Tx in bytes, 512 - 16384 */);
#endif

  if (!Firebase.RTDB.beginStream(&stream, "/test/stream/LEDs"))
    Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());

  Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);

}

void loop()
{

  // Firebase.ready() should be called repeatedly to handle authentication tasks.

  if (Firebase.ready() && (millis() - sendDataPrevMillis > 2000 || sendDataPrevMillis == 0))
  {

    int humidity = dht.readHumidity();
    int temperature = dht.readTemperature();
    int sensorValue = analogRead(35); // read analog input pin 0
    sendDataPrevMillis = millis();
    oled.setCursor(1, 45);       // set position to display
    oled.println((int)temperature); // set text
    oled.display();              // display on OLED
    oled.setCursor(1, 54);       // set position to display
    oled.println((int)humidity); // set text
    oled.display();              // display on OLED

    oled.setCursor(1, 63);       // set position to display
    oled.println("LDR : "); // set text
    oled.display();              // display on OLED
    oled.setCursor(30, 63);       // set position to display
    oled.println((int)sensorValue); // set text
    oled.display();              // display on OLED
    
    if (BUTTON_PIN == 1){
      oled.setCursor(1, 36);       // set position to display
      oled.println("BUTTON : ACTIF"); // set text
      oled.display();              // display on OLED
        
    }
    else {
    oled.setCursor(1, 36);       // set position to display
    oled.println("BUTTON : NOT ACTIF"); // set text
    oled.display();              // display on OLED
    
    }
    FirebaseJson json;
    json.add("BUTTON", digitalRead(BUTTON_PIN)  ? "ACTIF" : " NOT ACTIF");
    json.add("TEMPERATURE", temperature);
    json.add("HUMIDITY", humidity);
    json.add("LDR", sensorValue);
    Serial.printf("Set json... %s\n\n", Firebase.RTDB.setJSON(&fbdo, "/test/stream/data/json", &json) ? "ok" : fbdo.errorReason().c_str());
    
  }

  if (dataChanged)
  {
    dataChanged = false;
    // When stream data is available, do anything here...
  }
}

/*#include <Wire.h>
#include <SD.h>
#include <WiFi.h>
#include <ETH.h>
#include <SPIFFS.h>
#include <Update.h>
#include <Adafruit_GFX.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_SSD1306.h>
#include <WiFiClientSecure.h>

#define SCREEN_WIDTH 128 // OLED width,  in pixels
#define SCREEN_HEIGHT 64 // OLED height, in pixels

// create an OLED display object connected to I2C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(9600);

  // initialize OLED display with I2C address 0x3C
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("failed to start SSD1306 OLED"));
    while (1);
  }

  delay(2000);         // wait two seconds for initializing
  oled.clearDisplay(); // clear display

  oled.setTextSize(1);         // set text size
  oled.setTextColor(WHITE);    // set text color
  oled.setCursor(0, 10);       // set position to display
  oled.println("VOUFO & QUENNY <3"); // set text
  oled.display();              // display on OLED
}

void loop() {
}*/

