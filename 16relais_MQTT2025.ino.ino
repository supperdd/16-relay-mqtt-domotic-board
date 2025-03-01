#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include "auth.h"  // Fichier contenant les identifiants MQTT

// ====== Définition des broches ======
const int nombreBoutons = 4;
const int entrees[nombreBoutons] = {2, 3, 4, 5};
const int sorties[nombreBoutons] = {22, 23, 24, 25};

// ====== Configuration réseau ======
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 100);

// Initialisation Ethernet et MQTT
EthernetClient ethClient;
PubSubClient client(ethClient);

// Mémorisation des états des relais
bool etatSorties[nombreBoutons] = {LOW, LOW, LOW, LOW};

// Timer pour éviter de bloquer en cas de coupure MQTT
unsigned long dernierEssaiMQTT = 0;
const unsigned long intervalMQTT = 10000;  // Essai toutes les 10 sec

void setup() {
    Serial.begin(115200);

    // Initialisation Ethernet
    Ethernet.begin(mac, ip);
    Serial.println("Connexion Ethernet OK");

    // Configuration du client MQTT
    client.setServer(serverHostname, 1883);
    client.setCallback(callbackMQTT);  // Fonction appelée quand un message est reçu

    // Initialisation des entrées et sorties
    for (int i = 0; i < nombreBoutons; i++) {
        pinMode(entrees[i], INPUT_PULLUP);
        pinMode(sorties[i], OUTPUT);
        digitalWrite(sorties[i], LOW);
    }

    // Connexion au broker MQTT
    reconnectMQTT();
}

void loop() {
    // Vérification de la connexion MQTT sans bloquer le programme
    if (!client.connected()) {
        if (millis() - dernierEssaiMQTT > intervalMQTT) {
            dernierEssaiMQTT = millis();
            reconnectMQTT();
        }
    } else {
        client.loop();  // Maintient la connexion MQTT active
    }

    // Gestion des boutons et des relais en continu
    for (int i = 0; i < nombreBoutons; i++) {
        bool etatBouton = digitalRead(entrees[i]) == LOW;  // Bouton pressé

        // Si le bouton est pressé, il prend le dessus sur MQTT
        if (etatBouton && !etatSorties[i]) {
            etatSorties[i] = HIGH;
            digitalWrite(sorties[i], HIGH);
            envoyerMQTT(i, HIGH);
        } else if (!etatBouton && etatSorties[i]) {
            etatSorties[i] = LOW;
            digitalWrite(sorties[i], LOW);
            envoyerMQTT(i, LOW);
        }
    }
}

// ====== Fonction pour envoyer un message MQTT ======
void envoyerMQTT(int index, bool etat) {
    char topic[40];
    sprintf(topic, "atmega/RB/r83/pub/r%d", index + 1);
    char message[10];
    sprintf(message, "%s", etat ? "ON" : "OFF");

    if (client.publish(topic, message)) {
        Serial.print("MQTT envoyé: ");
        Serial.print(topic);
        Serial.print(" -> ");
        Serial.println(message);
    } else {
        Serial.println("Échec de l'envoi MQTT");
    }
}

// ====== Fonction pour recevoir les commandes MQTT ======
void callbackMQTT(char* topic, byte* payload, unsigned int length) {
    char message[length + 1];
    strncpy(message, (char*)payload, length);
    message[length] = '\0';

    Serial.print("Message reçu sur MQTT: ");
    Serial.print(topic);
    Serial.print(" -> ");
    Serial.println(message);

    for (int i = 0; i < nombreBoutons; i++) {
        char topicAttendu[40];
        sprintf(topicAttendu, "atmega/RB/r83/sub/r%d", i + 1);

        if (strcmp(topic, topicAttendu) == 0) {  // Si c'est un topic correspondant
            if (strcmp(message, "ON") == 0) {
                etatSorties[i] = HIGH;
                digitalWrite(sorties[i], HIGH);
            } else if (strcmp(message, "OFF") == 0) {
                etatSorties[i] = LOW;
                digitalWrite(sorties[i], LOW);
            }
            envoyerMQTT(i, etatSorties[i]);  // Publier le nouvel état
        }
    }
}

// ====== Fonction pour reconnecter MQTT ======
void reconnectMQTT() {
    Serial.println("Tentative de reconnexion au broker MQTT...");
    
    if (client.connect("ArduinoMega", serverUsername.c_str(), serverPassword.c_str())) {
        Serial.println("MQTT reconnecté !");
        
        // Réabonnement aux topics
        for (int i = 0; i < nombreBoutons; i++) {
            char topicSub[40];
            sprintf(topicSub, "atmega/RB/r83/sub/r%d", i + 1);
            client.subscribe(topicSub);
            Serial.print("Abonné à : ");
            Serial.println(topicSub);
        }

        // Renvoi des états des relais après reconnexion
        for (int i = 0; i < nombreBoutons; i++) {
            envoyerMQTT(i, etatSorties[i]);
        }
    } else {
        Serial.println("Échec de reconnexion MQTT.");
    }
}
