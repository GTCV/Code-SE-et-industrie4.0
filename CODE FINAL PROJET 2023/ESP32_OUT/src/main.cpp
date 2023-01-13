//Définition des librairies
#include <WiFi.h>
#include <Wire.h>
#include <Arduino.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Identifiant de connection wifi entre l'esp32 et le pc
const char* ssid = "cyrille";
const char* password = "12345678";
const char* mqtt_server = "192.168.129.3";

uint32_t color;
WiFiClient espClient;
PubSubClient client(espClient);

//Déclaration des Chaines de caractères
String ETATBP;
String ETATRGB;
String ETATLED;

//Déclaration des boolléennes
bool led;
bool chaud;
bool froid;
bool msgReceived;
bool chaudButton = false;
bool froidButton = false;

//Déclaration des entiers
int value = 0;
int sensorValue;
int LDR;
int humidity ;
int temperature;
unsigned long lastMsg = 0;
unsigned long sendDataPrevMillis = 0;


//Définition des différentes pins
#define LED 2 
#define BUZZER  17 
#define PIN_RED  16 
#define PIN_BLUE  18 
#define PIN_GREEN  19 
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 

// create an OLED display object connected to I2C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

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

//Fonction qui permet de prendre les informations du dashboard pour le hardware
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  //Initialisation de la fonction JSON
  DynamicJsonDocument doc(256);
  deserializeJson(doc, payload);
  JsonObject obj = doc.as<JsonObject>();
  
  //Importation des données liées à l'ESP32_IN
  temperature = obj["TEMPERATURE"].isNull() ? temperature : obj["TEMPERATURE"];
  humidity = obj["HUMIDITY"].isNull() ? humidity : obj["HUMIDITY"];
  LDR = obj["LDR"].isNull() ? LDR : obj["LDR"];
  led = obj["LED"].isNull() ? led : obj["LED"];
  froid = obj["START"];//.isNull() ? start : obj["START"];
  chaud = obj["STOP"];//.isNull() ? stop : obj["STOP"];

  msgReceived = true;
  
  for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
  }
   Serial.println();

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
      // ... and resubscribe
      client.subscribe("ESP32_OUT");
      client.subscribe("ESP32_IN");
    
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
  
  pinMode(PIN_RED,   OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE,  OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED, OUTPUT);  
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // initialize OLED display with I2C address 0x3C
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("failed to start SSD1306 OLED"));
    while (1);
    
  }

  delay(2000);         // wait two seconds for initializing
  
}

//Cette fonction permet d'écrire le code que nous voulons mettre en évidence
void loop() {

  //Cette condition vérifie si le client est connecté ou pas  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  //Cette condition permet de mettre tout à 0 c'est-à-dire OFF le système
  if (led == 0)
  {
    digitalWrite(PIN_GREEN, LOW);
    digitalWrite(PIN_BLUE, LOW);
    digitalWrite(PIN_RED, LOW);
    digitalWrite(BUZZER, LOW);
    digitalWrite(LED, LOW);
    msgReceived= false;
    froid = false;
    chaud = false;
    ETATBP = "NOT ACTIF";
    ETATLED = "OFF";
    oled.clearDisplay();
    oled.display();
   
  }

  //Cette condition vérifie si un paramètre à changé parmis les données à transmettre afin de l'afficher
  if (msgReceived)
  {
    msgReceived = false;

    oled.clearDisplay();
    oled.setTextSize(1);         // set text size
    oled.setTextColor(WHITE);    // set text color
    oled.setCursor(0, 0);        // set position to display
    oled.println("TEMP  : ");    // set text
    oled.setCursor(50, 0);       // set position to display
    oled.print(temperature);     // set text
    oled.setCursor(70, 0);       // set position to display
    oled.print("°C");            // set text
    oled.setCursor(0, 10);       // set position to display
    oled.println("HUM   : ");    // set text
    oled.setCursor(50, 10);      // set position to display
    oled.print(humidity);        // set text
    oled.setCursor(70, 10);      // set position to display
    oled.print("%");             // set text
    oled.setCursor(0, 20);       // set position to display
    oled.println("LDR   : ");    // set text
    oled.setCursor(50, 20);      // set position to display
    oled.print(LDR);             // set text
    oled.setCursor(0, 30);       // set position to display
    oled.println("LED   : ");    // set text
    oled.setCursor(50, 30);      // set position to display
    oled.print(ETATLED);         // set text
    oled.setCursor(0, 40);       // set position to display
    oled.println("ETAT  : ");    // set text
    oled.setCursor(50, 40);      // set position to display
    oled.print(ETATRGB);         // set text
    oled.setCursor(0, 50);       // set position to display
    oled.println("BUTTON: ");    // set text
    oled.setCursor(50, 50);      // set position to display
    oled.print(ETATBP);          // set text
    oled.display();              // display on OLED 

    digitalWrite(LED, led);
    
    //Cette condition permet d'afficher l'état du bouton et celui de la LED
    if (led == 1){ETATLED ="ON"; ETATBP ="ACTIF";} else {ETATLED = "OFF"; ETATBP="NOT ACTIF";}
    
    //Cette condition vérifie si la température est bonne et allume le GREEN de la RGB
    if (25 <= temperature && temperature <= 30)
    {

      digitalWrite(BUZZER, LOW);
      digitalWrite(PIN_RED, LOW);
      digitalWrite(PIN_BLUE, LOW);
      digitalWrite(PIN_GREEN, HIGH);
      ETATRGB = "BON";
            
    }
    
    //Cette condition vérifie si la température est inférieur et allume le BLUE de la RGB
    else if (temperature < 25)
    {
      digitalWrite(PIN_BLUE, HIGH);
      digitalWrite(PIN_GREEN, LOW);
      digitalWrite(PIN_RED, LOW);
      digitalWrite(BUZZER, HIGH);
      ETATRGB = "INFERIEUR";

      oled.clearDisplay();
      oled.setTextSize(1);                 // set text size
      oled.setTextColor(WHITE);            // set text color
      oled.setCursor(0, 0);                // set position to display
      oled.println("  TEMPERATURE BASSE"); // set text
      oled.setCursor(0, 20);               // set position to display
      oled.println("  ACTIVEZ LE CHAUD");  // set text
      oled.setCursor(10, 40);              // set position to display
      oled.println("ETAT  : ");            // set text
      oled.setCursor(60, 40);              // set position to display
      oled.print(ETATRGB);                 // set text
      oled.display();                      // display on OLED    
     
      
      //Cette boucle permet de chauffer le local lorsqu'on appui sur le bouton chaud
      if (chaud && !chaudButton) chaudButton = true;
      if (chaudButton) 
      {   
          digitalWrite(PIN_RED, LOW);
          digitalWrite(PIN_GREEN, LOW);
          digitalWrite(PIN_BLUE, HIGH);
          digitalWrite(BUZZER, LOW);
          
          oled.clearDisplay();
          oled.setTextSize(1);                  // set text size
          oled.setTextColor(WHITE);             // set text color
          oled.setCursor(0, 0);                 // set position to display
          oled.println("    CHAUD ACTIVE");     // set text
          oled.setCursor(0, 20);                // set position to display
          oled.println("  REGULATION DE LA ");  // set text
          oled.setCursor(0, 40);                // set position to display
          oled.println("TEMPERATURE EN COURS"); // set text
          oled.display();                       // display on OLED    
      }
      
    }
    if (chaudButton && temperature >= 25) chaudButton = false;
    
    //Cette condition vérifie si la température est supérieur et allume le RED de la RGB
    else if (temperature > 30)
    {

      digitalWrite(PIN_RED, HIGH);
      digitalWrite(PIN_GREEN, LOW);
      digitalWrite(PIN_BLUE, LOW);
      digitalWrite(BUZZER, HIGH);
      ETATRGB = "SUPERIEUR";

      oled.clearDisplay();
      oled.setTextSize(1);                   // set text size
      oled.setTextColor(WHITE);              // set text color
      oled.setCursor(0, 0);                  // set position to display
      oled.println("  TEMPERATURE ELEVEE");  // set text
      oled.setCursor(0, 20);                 // set position to display
      oled.println("   ACTIVEZ LA CLIM");    // set text
      oled.setCursor(0, 40);                 // set position to display
      oled.println("  ETAT  : ");            // set text
      oled.setCursor(70, 40);                // set position to display
      oled.print(ETATRGB);                   // set text
      oled.display();                        // display on OLED 
      
      //Cette condition permet de refroidir le local lorsqu'on appui sur le bouton froid
      if (froid && !froidButton) froidButton = true;
      if (froidButton) 
      {
        digitalWrite(PIN_RED, HIGH);
        digitalWrite(PIN_GREEN, LOW);
        digitalWrite(PIN_BLUE, LOW);
        digitalWrite(BUZZER, LOW);

        oled.clearDisplay();
        oled.setTextSize(1);                  // set text size
        oled.setTextColor(WHITE);             // set text color
        oled.setCursor(0, 0);                 // set position to display
        oled.println("    CLIM ACTIVEE");     // set text
        oled.setCursor(0, 20);                // set position to display
        oled.println("  REGULATION DE LA ");  // set text
        oled.setCursor(0, 40);                // set position to display
        oled.println("TEMPERATURE EN COURS"); // set text
        oled.display();                       // display on OLED    
      }
      
    }
    if (froidButton && temperature <= 30) froidButton = false;

  }
  
  //Cette partie de code permet d'envoyer l'etat de la led, rgb et bouton au node-red
    unsigned long now = millis();
    if (now - lastMsg > 2000) 
    {
      lastMsg = now;
      ++value;
      DynamicJsonDocument doc(256);

      doc["ETATRGB"] = ETATRGB;
      doc["ETATLED"] = ETATLED;
      doc["ETATBP"]  =  ETATBP;
          
      char json_string[256];
      serializeJson(doc, json_string);
      client.publish("ESP32_OUT_ETAT", json_string); 
    }
}