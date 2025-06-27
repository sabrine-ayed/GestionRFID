#include <Arduino.h>
#include "PlatformHardware.h"
#include "PlatformHardware.cpp"

// Configuration des pins RFID
#define DATA0_PIN    22   // Pin pour DATA0 (changez selon votre branchement)
#define DATA1_PIN    23   // Pin pour DATA1 (changez selon votre branchement) 
#define CP_PIN       21   // Pin pour Card Present (changez selon votre branchement)

// LED pour indication visuelle (optionnel)
#define LED_PIN      2  // LED intégrée Arduino/ESP32

// Instance RFID
ReemasRfid* rfidReader = nullptr;

// Variables pour le test
unsigned long lastCheck = 0;
unsigned long lastStatusPrint = 0;
const unsigned long CHECK_INTERVAL = 10;      // Vérifier toutes les 10ms
const unsigned long STATUS_INTERVAL = 5000;   // Afficher le statut toutes les 5s

void setup() {
    // Initialisation série
    Serial.begin(115200);
    while (!Serial) {
        delay(10); // Attendre que le port série soit prêt
    }
    
    Serial.println("=================================");
    Serial.println("   TEST REEMAS RFID - DEBUT");
    Serial.println("=================================");
    Serial.printf("Pins utilisées - DATA0: %d, DATA1: %d, CP: %d\n", DATA0_PIN, DATA1_PIN, CP_PIN);
    
    // Configuration LED (optionnel)
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    // Création et initialisation du lecteur RFID
    rfidReader = new ReemasRfid(DATA0_PIN, DATA1_PIN, CP_PIN);
    rfidReader->begin();
    
    Serial.println("Lecteur RFID initialisé");
    Serial.println("Approchez une carte RFID...");
    Serial.println("---------------------------------");
    
    lastCheck = millis();
    lastStatusPrint = millis();
}

void loop() {
    unsigned long currentTime = millis();
    
    // Vérification périodique du timeout
    if (currentTime - lastCheck >= CHECK_INTERVAL) {
        lastCheck = currentTime;
        rfidReader->checkTimeout();
        
        // Vérifier s'il y a une nouvelle carte
        if (rfidReader->hasNewCard()) {
            uint32_t cardId = rfidReader->getCardId();
            
            // Extraction des informations de la carte
            uint16_t facilityCode = (cardId >> 16) & 0xFF;
            uint16_t cardNumber = cardId & 0xFFFF;
            
            // Affichage des informations
            Serial.println("╔════════════════════════════════╗");
            Serial.println("║        NOUVELLE CARTE          ║");
            Serial.println("╠════════════════════════════════╣");
            Serial.printf("║ ID Complet : %-10u (0x%08X) ║\n", cardId, cardId);
            Serial.printf("║ Code Site  : %-18u ║\n", facilityCode);
            Serial.printf("║ N° Carte   : %-18u ║\n", cardNumber);
            Serial.println("╚════════════════════════════════╝");
            
            // Clignoter la LED
            for (int i = 0; i < 6; i++) {
                digitalWrite(LED_PIN, HIGH);
                delay(100);
                digitalWrite(LED_PIN, LOW);
                delay(100);
            }
        }
    }
    
    // Affichage périodique du statut (debug)
    if (currentTime - lastStatusPrint >= STATUS_INTERVAL) {
        lastStatusPrint = currentTime;
        
        Serial.println("\n--- STATUT LECTEUR RFID ---");
        Serial.printf("Carte détectée : %s\n", rfidReader->isCardDetected() ? "OUI" : "NON");
        Serial.printf("Données valides: %s\n", rfidReader->isValid() ? "OUI" : "NON");
        Serial.printf("Nb erreurs     : %lu\n", rfidReader->getErrorCount());
        Serial.printf("Uptime         : %lu ms\n", currentTime);
        Serial.println("---------------------------\n");
    }
    
    // Petite pause pour éviter de surcharger le processeur
    delay(1);
}

// Fonctions utilitaires pour debug avancé (optionnel)
void printDetailedStatus() {
    Serial.println("\n╔══════════════════════════════════╗");
    Serial.println("║         DEBUG DÉTAILLÉ           ║");
    Serial.println("╠══════════════════════════════════╣");
    Serial.printf("║ Carte détectée : %-15s ║\n", rfidReader->isCardDetected() ? "OUI" : "NON");
    Serial.printf("║ Données valides: %-15s ║\n", rfidReader->isValid() ? "OUI" : "NON");
    Serial.printf("║ ID Carte       : 0x%08X       ║\n", rfidReader->getCardId());
    Serial.printf("║ Erreurs        : %-15lu ║\n", rfidReader->getErrorCount());
    Serial.println("╚══════════════════════════════════╝\n");
}

// Fonction pour reset manuel (appelable depuis le moniteur série)
void resetRfid() {
    Serial.println(">>> RESET MANUEL DU LECTEUR RFID <<<");
    rfidReader->reset();
    rfidReader->clearErrorCount();
    Serial.println(">>> RESET TERMINÉ <<<\n");
}

// Fonction pour tester la connexion des pins
void testPins() {
    Serial.println("\n>>> TEST DES PINS RFID <<<");
    Serial.printf("DATA0 (pin %d): %s\n", DATA0_PIN, digitalRead(DATA0_PIN) ? "HIGH" : "LOW");
    Serial.printf("DATA1 (pin %d): %s\n", DATA1_PIN, digitalRead(DATA1_PIN) ? "HIGH" : "LOW");
    Serial.printf("CP (pin %d)   : %s\n", CP_PIN, digitalRead(CP_PIN) ? "HIGH" : "LOW");
    Serial.println(">>> FIN TEST PINS <<<\n");
}