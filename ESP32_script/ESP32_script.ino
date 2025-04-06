#include <WiFi.h>
#include <NeoPixelBus.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <base64.h>
#include "mbedtls/base64.h"
#include <Wire.h>
#include "Adafruit_AS7341.h"

// Wifi values
const char* ssid = "Freebox-4ACC21";
const char* password = "imbibere2-evici3.-suberat?-exultetis";

// API values
const char* ConfigURL = "http://192.168.1.109:1908/config/";
const char* TestURL = "http://192.168.1.109:1908/";
const char* GetColorURL = "http://192.168.1.109:1908/rgb/";
const char* jwtURL = "http://192.168.1.109:1908/jwt/";

// Initialization JWT token
String jwtToken = "";

// Default LED GPIO
int ledPin = 10;
int nbLed = 10;

// Use http
//WiFiClientSecure client;
WiFiClient client;


NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0800KbpsMethod>* strip = nullptr;
Adafruit_AS7341 as7341;

void setup() {

  // ------------------------------------ //
  // ------------ Basic setup ------------//
  // ------------------------------------ //
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

  // ------------------------------------- //
  // ------------- Connection -------------//
  // ------------------------------------- //
  WiFi.begin(ssid, password);
  Serial.print("Connexion au Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnecté au Wi-Fi !");
  Serial.print("Adresse IP: ");
  Serial.println(WiFi.localIP());

  // ------------------------------------- //
  // --------------- Sensor ---------------//
  // ------------------------------------- //

  // ------------------------------------- //
  // ------------- Get config -------------//
  // ------------------------------------- //

  HTTPClient http;
  http.begin(ConfigURL);
  int httpResponseCode = http.GET();
  Serial.println(httpResponseCode); //debug

  // Vérifie si la requête a réussi
  if (httpResponseCode <= 0) {
    Serial.println("Erreur de connexion");
  }

  String response = http.getString();
  Serial.println("Réponse brute : " + response);//debug

  // Parse le JSON
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, response);

  if (error) {
    Serial.print("Erreur de parsing JSON : ");
    Serial.println(error.c_str());
  }

  nbLed = doc["LEDs"];
  ledPin = doc["GPIO"];

  Serial.println("Nombre de LEDs : " + String(nbLed));
  Serial.println("GPIO LED : " + String(ledPin));
  http.end();


  if (nbLed > 0) {
    strip = new NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0800KbpsMethod>(nbLed, ledPin);
    strip->Begin();
    strip->Show();  // Éteindre les LEDs au démarrage
    Serial.println("LED strip initialized !");
  }
  digitalWrite(2, LOW);

}

void loop() {

  // Test if Wifi is connect
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi not connect !");
    digitalWrite(2, HIGH);
    delay(1500);
    digitalWrite(2, LOW);
    return;
  }

  HTTPClient http;
  http.begin(GetColorURL);
  int httpResponseCode = http.GET();
  Serial.println(httpResponseCode); //debug

  // Vérifie si la requête a réussi
  if (httpResponseCode < 0) {
    Serial.println("Erreur de connexion");
    return
  }

  String response = http.getString();
  Serial.println("Réponse brute : " + response);//debug

  // Parse le JSON
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, response);

  if (error) {
    Serial.print("Erreur de parsing JSON : ");
    Serial.println(error.c_str());
    return
  }

  int r = doc["red"];
  int g = doc["green"];
  int b = doc["blue"];
  http.end();

  RgbColor newColor(r, g, b);

  // Appliquer la couleur sur toutes les LEDs
  for (int i = 0; i < nbLed; i++) {
    strip->SetPixelColor(i, newColor);
  }
  strip->Show();
  Serial.println("Couleur appliquée !");
  
  delay(180000);
}

void getTokenJWT() {
  HTTPClient http;
  http.begin(client, jwtURL);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<200> jsonDoc;
  jsonDoc["espID"] = "IW2F4LBWN0";
  String jsonString;
  serializeJson(jsonDoc, jsonString);

  int httpResponseCode = http.POST(jsonString);
  Serial.println(httpResponseCode);

  if (httpResponseCode != 200) {
    Serial.println(http.getString());
    http.end();
    return;
  }

  String response = http.getString();
  http.end();

  StaticJsonDocument<512> resDoc;
  DeserializationError error = deserializeJson(resDoc, response);
  if (error || !resDoc.containsKey("token")) {
    Serial.println("Erreur parsing JSON ou token absent");
    return;
  }

  jwtToken = resDoc["token"].as<String>();
}