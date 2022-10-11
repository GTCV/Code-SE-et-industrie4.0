//#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#ifdef ESP32
#include <WiFi.h>
#include <LittleFS.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <WiFiClientSecure.h>
#include <SPI.h>
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED width,  in pixels
#define SCREEN_HEIGHT 64 // OLED height, in pixels
#define BUTTON_PIN T6 // ESP32 pin GIOP16, which connected to button
#define LED_PIN    32  // ESP32 pin GIOP18, which connected to led


// The below are variables, which can be changed
int button_state = 0;   // variable for reading the button status

// create an OLED display object connected to I2C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
#define DHTPIN 26
#define DHTTYPE DHT11


AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char* ssid = "cyrille";
const char* password = "12345678";

const char* PARAM_MESSAGE = "message";
DHT dht(DHTPIN, DHTTYPE);
bool ledstate;


void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_DATA)
  {
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    String msg = "";
    if (info->final && info->index == 0 && info->len == len)
    {
      if (info->opcode == WS_TEXT) {
        for (size_t i = 0; i < info->len; i++) {
          msg += (char) data[i];
        }
      }
      if (msg == "on"){
        //led s'allume
        digitalWrite(LED_PIN, HIGH); // turn on LED
        oled.clearDisplay(); // clear display
        oled.setCursor(0, 10);       // set position to display
        oled.println("LED : ON"); // set text
        oled.display();              // display on OLED
        ledstate = true;
      }
      if (msg == "off"){
        //led s'étteint
        digitalWrite(LED_PIN, LOW);  // turn off LED
        oled.clearDisplay(); // clear display
        oled.setCursor(0, 10);       // set position to display
        oled.println("LED OFF"); // set text
        oled.display();              // display on OLED
        ledstate = false;
      }


      Serial.printf("%s\n", msg.c_str());
    }
  }
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("failed to start SSD1306 OLED"));
    while (1);
  }
  oled.clearDisplay(); // clear display
  oled.setCursor(0, 10);       // set position to display
  oled.print("Démmarrage");
  oled.display();

  if (!LittleFS.begin(true)) {
    Serial.println("An Error has occurred while mounting LITTLEFS");
  }

  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("WiFi Failed!\n");
    return;
  }

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Send a GET request to <IP>/get?message=<message>
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(LittleFS, "/index.html", "text/html", false);
  });

  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/style.css", "text/css");
  });

  // Route to load function.js file
  server.on("/function.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/function.js", "text/js");
  });

  // Send a POST request to <IP>/post with a form field message set to <message>
  server.on("/post", HTTP_POST, [](AsyncWebServerRequest * request) {
    String message;
    if (request->hasParam(PARAM_MESSAGE, true)) {
      message = request->getParam(PARAM_MESSAGE, true)->value();
    } else {
      message = "No message sent";
    }
    request->send(200, "text/plain", "Hello, POST: " + message);
  });

  server.onNotFound(notFound);
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  server.begin();
}

void loop() {
  
  delay(1000);
  const float humidity = dht.readHumidity();
  const float temperature = dht.readTemperature();

  DynamicJsonDocument doc(256);

  doc["led"] = ledstate;  
  doc["BP"] = digitalRead(BUTTON_PIN) ? "NOT ACTIF" : "ACTIF"; 
  doc["TEMPERATURE"] = temperature;
  doc["HUMIDITY"] = humidity;
  
  char json_string[256];
  serializeJson(doc, json_string);

  ws.textAll(json_string);
  
}