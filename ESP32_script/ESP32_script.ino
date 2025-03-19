#include <WiFi.h>

const char *ssid = "OpnSmartLED_ESP32";
const char *password = "password";

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  Serial.print("Adresse IP de l'ESP32: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  delay(500);
}
