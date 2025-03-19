#include <WiFi.h>

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
    Serial.println(WiFi.localIP()); // Affiche l'adresse IP de l'ESP32
    
    // Démarrer le serveur TCP
    server.begin();
}

void loop() {
    if (!client) {
        client = server.available();  // Accepte une connexion client
    } else {
        if (client.available()) {
            String message = client.readStringUntil('\n'); // Lire jusqu'au retour à la ligne
            Serial.print("Recept: ");
            Serial.println(message);

            // Répondre au client
            client.println("ping");
            client.stop();
        }
    }
}
