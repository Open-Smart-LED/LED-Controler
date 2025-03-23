#include <WiFi.h>
#include <NeoPixelBus.h>

const uint8_t PIN_LED = 6;  // GPIO connecté aux LED
uint16_t numLeds = 0;
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0800KbpsMethod> *strip = nullptr;

const char* ssid = "Freebox-4ACC21";
const char* password = "imbibere2-evici3.-suberat?-exultetis";

WiFiServer server(2025);  // Serveur TCP sur le port 2025
WiFiClient client;

void setup() {
    Serial.begin(115200);
    
    // Connexion au Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    // Wi-Fi connecté
    Serial.println("\nConnecté !");
    Serial.print("Adresse IP: ");
    Serial.println(WiFi.localIP());

    // Démarrer le serveur TCP
    server.begin();

    // Attendre une connexion client pour recevoir le nombre de LEDs
    while (!client) {
        client = server.available();
        delay(100);
    }
    Serial.println("Client connecté, en attente du nombre de LEDs...");

    // Lire le nombre de LEDs
    if (client.available()) {
        numLeds = client.readStringUntil('\n').toInt();
        Serial.print("Nombre de LEDs : ");
        Serial.println(numLeds);

        if (numLeds > 0) {
            strip = new NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0800KbpsMethod>(numLeds, PIN_LED);
            strip->Begin();
            strip->Show();  // Éteindre les LEDs au démarrage
            Serial.println("LED strip initialisé !");
        }
    }
}

void loop() {
    if (!client) {
        client = server.available();  // Accepte une nouvelle connexion client
    } else {
        if (client.available()) {
            String colorSet = client.readStringUntil('\n');
            Serial.print("Reçu : ");
            Serial.println(colorSet);

            // Conversion de la couleur
            int r, g, b;
            if (sscanf(colorSet.c_str(), "%d,%d,%d", &r, &g, &b) == 3) {
                RgbColor newColor(r, g, b);

                // Appliquer la couleur sur toutes les LEDs
                for (int i = 0; i < numLeds; i++) {
                    strip->SetPixelColor(i, newColor);
                }
                strip->Show();
                Serial.println("Couleur appliquée !");
            } else {
                Serial.println("Erreur : format couleur invalide (attendu : R,G,B)");
            }
        }
    }
}
