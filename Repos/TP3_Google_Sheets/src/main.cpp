#include <FS.h>
#include <DHT.h>
#include <WiFi.h>
//#include <Wire.h>
#include <DNSServer.h>
#include <HTTPClient.h>
#include <Adafruit_Sensor.h>
#define BUTTON_PIN 12 // ESP32 pin GIOP16, which connected to button
#define LED_PIN    2  // ESP32 pin GIOP18, which connected to led
// The below are variables, which can be changed
int button_state = 0;   // variable for reading the button status
int sensorValue;

// create an OLED display object connected to I2C
#define DHTPIN 26
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
bool ledstate;


const char *ssid = "cyrille";
const char *password = "12345678";
String GOOGLE_SCRIPT_ID = "AKfycbxx-f9xtYmHwykH8BdygzJI0UV01xqZB35mGT2qlLpk4wA9Q4lEqt2jxTyWPoqsGIm50A";

const char * root_ca=\
"-----BEGIN CERTIFICATE-----\n" \
"MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG\n" \
"A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\n" \
"b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw\n" \
"MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\n" \
"YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT\n" \
"aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ\n" \
"jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp\n" \
"xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp\n" \
"1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG\n" \
"snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ\n" \
"U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8\n" \
"9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E\n" \
"BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B\n" \
"AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz\n" \
"yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE\n" \
"38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP\n" \
"AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad\n" \
"DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME\n" \
"HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==\n" \
"-----END CERTIFICATE-----\n";


WiFiClientSecure client;

void sendData(String params) {
  
  HTTPClient http;
  String url="https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?"+params;
  Serial.println(url);
  Serial.println("Making a request");
  // Your Domain name with URL path or IP address with path
  http.begin(url); //Specify the URL and certificate
  // Send HTTP GET request
  int httpCode = http.GET();
  if (httpCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpCode);
    String payload = http.getString();
    Serial.println(payload);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpCode);
  }
  // Free resources
  http.end();
}

void setup() {

  Serial.begin(115200);
  delay(10);
  dht.begin();
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
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
String strTemp, strHum, strParameter;

void loop() {
    sensors_event_t event;
    button_state = digitalRead(BUTTON_PIN);
    delay(1000); // wait 100ms for next reading
   //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED && button_state == HIGH){
      digitalWrite(LED_PIN, HIGH);
      strHum = dht.readHumidity();
      strTemp = dht.readTemperature();
      sensorValue = analogRead(33); // read analog input pin 0
      strParameter = "temperature=" + strTemp + "&humidity=" + strHum + "&LDR=" + sensorValue;
      sendData(strParameter);
      Serial.print(F("Temperature: "));
      Serial.print(strTemp);
      Serial.println(F("°C"));
      Serial.print(F("Humidity: "));
      Serial.print(strHum);
      Serial.println(F("%"));
      Serial.print(F("LDR: "));
      Serial.print(sensorValue, DEC); // prints the value read
    }

    else {
      digitalWrite(LED_PIN, LOW);
    }
     lastTime = millis();
}