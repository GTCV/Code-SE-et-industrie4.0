//#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <WiFiClientSecure.h>

String GAS_ID = "AKfycbx3z_KbseKH1U9EltJcHuZu8DEVLFsHdvmjkpoGGXttBAA4Jmj";

const char* fingerprint = "46 B2 C3 44 9C 59 09 8B 01 B6 F8 BD 4C FB 00 74 91 2F EF F6";
const char* host = "script.google.com";
const int httpsPort = 443;
const char* ssid = "Your_WiFi_Name_or_SSID";
const char* password = "Your_WiFi_Password";

WiFiClientSecure gsclient;

void sendData(int temp, int hum) {
  Serial.print("connecting to ");
  Serial.println(host);
  gsclient.setInsecure();
  if (!gsclient.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  if (gsclient.verify(fingerprint, host)) {
    Serial.println("certificate matches");
  }

  else {
    Serial.println("certificate doesn't match");
  }

  String strTemp = String(temp, DEC);
  String strHum = String(hum, DEC);
  Serial.println(strTemp);
  Serial.println(strHum);

  String url = "/macros/s/" + GAS_ID + "/exec?temperature=" + strTemp + "&humidity=" + strHum;
  Serial.print("requesting URL: ");
  Serial.println(url);
  gsclient.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" +
                 "User-Agent: BuildFailureDetectorESP8266\r\n" + "Connection: close\r\n\r\n");
  Serial.println("request sent");
  Serial.println("closing connection");
}

void setup(void) {
  Serial.begin(115200);
  randomSeed(analogRead(0));
}

void loop(void) {
  sendData(random(100), random(100));
  delay(5000);
}