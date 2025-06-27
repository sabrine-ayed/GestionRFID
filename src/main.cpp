#include <Arduino.h>
#include "PlatformHardware.h"
// DO NOT include the .cpp file - it should be compiled separately
#include "PlatformHardware.cpp" 

// Configuration des pins RFID
#define DATA0_PIN    22   // Pin pour DATA0 (changez selon votre branchement)
#define DATA1_PIN    23   // Pin pour DATA1 (changez selon votre branchement) 
#define CP_PIN       21   // Pin pour Card Present (changez selon votre branchement)

// LED pour indication visuelle (optionnel)
#define LED_PIN      2  // LED intégrée Arduino/ESP32

// Configuration EEPROM
#define EEPROM_SIZE     512
#define EEPROM_BEGIN    0
#define EEPROM_VALUE_SIZE 4

// Instances des composants hardware
ReemasDigital* digital = nullptr;
ReemasTimer* timer = nullptr;
ReemasSerial* serial = nullptr;
ReemasRom* rom = nullptr;
ReemasFs* fileSystem = nullptr;  // Renamed from 'fs' to avoid namespace conflict
ReemasRfid* rfidReader = nullptr;
ArduinoHardware* hardware = nullptr;

// Timer hardware pour ReemasTimer
hw_timer_t* hwTimer = nullptr;

// Variables pour le test
unsigned long lastCheck = 0;
unsigned long lastStatusPrint = 0;
const unsigned long CHECK_INTERVAL = 10;      // Vérifier toutes les 10ms
const unsigned long STATUS_INTERVAL = 5000;   // Afficher le statut toutes les 5s

// Forward declarations
void testOtherComponents(uint32_t cardId);
void printDetailedHardwareStatus();
void resetHardwareRfid();
void testPinsViaHardware();

void setup() {
    // Initialisation série
    Serial.begin(115200);
    while (!Serial) {
        delay(10); // Attendre que le port série soit prêt
    }
    
    Serial.println("=================================");
    Serial.println("   TEST ARDUINO HARDWARE RFID");
    Serial.println("=================================");
    Serial.printf("Pins utilisées - DATA0: %d, DATA1: %d, CP: %d\n", DATA0_PIN, DATA1_PIN, CP_PIN);
    
    // Configuration LED (optionnel)
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    // Initialisation des composants hardware
    Serial.println("Initialisation des composants...");
    
    // 1. Digital
    digital = new ReemasDigital();
    Serial.println("✓ ReemasDigital créé");
    
    // 2. Timer (nécessite un timer hardware)
    hwTimer = timerBegin(0, 80, true); // Timer 0, prescaler 80, count up
    timer = new ReemasTimer(*hwTimer);
    Serial.println("✓ ReemasTimer créé");
    
    // 3. Serial
    serial = new ReemasSerial(Serial);
    Serial.println("✓ ReemasSerial créé");
    
    // 4. ROM/EEPROM
    rom = new ReemasRom(EEPROM_SIZE, EEPROM_BEGIN, EEPROM_VALUE_SIZE);
    Serial.println("✓ ReemasRom créé");
    
    // 5. File System
    fileSystem = new ReemasFs();
    Serial.println("✓ ReemasFs créé");
    
    // 6. RFID
    rfidReader = new ReemasRfid(DATA0_PIN, DATA1_PIN, CP_PIN);
    Serial.println("✓ ReemasRfid créé");
    
    // 7. Création du hardware principal
    hardware = new ArduinoHardware(*digital, *timer, *serial, *rom, *fileSystem, *rfidReader);
    Serial.println("✓ ArduinoHardware créé");
    
    // Initialisation du lecteur RFID via le hardware
    hardware->rfid->begin();
    Serial.println("✓ Lecteur RFID initialisé via ArduinoHardware");
    
    Serial.println("---------------------------------");
    Serial.println("Système prêt !");
    Serial.println("Accès RFID via : hardware->rfid");
    Serial.println("Approchez une carte RFID...");
    Serial.println("---------------------------------");
    
    lastCheck = millis();
    lastStatusPrint = millis();
}

void loop() {
    unsigned long currentTime = millis();
    
    // Vérification périodique du timeout - Cast to ReemasRfid to access specific methods
    if (currentTime - lastCheck >= CHECK_INTERVAL) {
        lastCheck = currentTime;
        
        // Cast to access ReemasRfid-specific methods
        ReemasRfid* rfid = static_cast<ReemasRfid*>(hardware->rfid);
        rfid->checkTimeout();
        
        // Vérifier s'il y a une nouvelle carte
        if (rfid->hasNewCard()) {
            uint32_t cardId = rfid->getCardId();
            
            // Extraction des informations de la carte
            uint16_t facilityCode = (cardId >> 16) & 0xFF;
            uint16_t cardNumber = cardId & 0xFFFF;
            
            // Affichage des informations
            Serial.println("╔════════════════════════════════╗");
            Serial.println("║        NOUVELLE CARTE          ║");
            Serial.println("║    (via ArduinoHardware)       ║");
            Serial.println("╠════════════════════════════════╣");
            Serial.printf("║ ID Complet : %-10u (0x%08X) ║\n", cardId, cardId);
            Serial.printf("║ Code Site  : %-18u ║\n", facilityCode);
            Serial.printf("║ N° Carte   : %-18u ║\n", cardNumber);
            Serial.println("╚════════════════════════════════╝");
            
            // Test des autres composants (optionnel)
            testOtherComponents(cardId);
            
            // Clignoter la LED
            for (int i = 0; i < 6; i++) {
                hardware->digital->WriteDigital(LED_PIN, 1);
                delay(100);
                hardware->digital->WriteDigital(LED_PIN, 0);
                delay(100);
            }
        }
    }
    
    // Affichage périodique du statut (debug)
    if (currentTime - lastStatusPrint >= STATUS_INTERVAL) {
        lastStatusPrint = currentTime;
        
        Serial.println("\n--- STATUT ARDUINO HARDWARE ---");
        Serial.printf("Carte détectée : %s\n", hardware->rfid->isCardDetected() ? "OUI" : "NON");
        Serial.printf("Données valides: %s\n", hardware->rfid->isValid() ? "OUI" : "NON");
        
        // Cast to access ReemasRfid-specific methods
        ReemasRfid* rfid = static_cast<ReemasRfid*>(hardware->rfid);
        Serial.printf("Nb erreurs     : %lu\n", rfid->getErrorCount());
        Serial.printf("Uptime         : %lu ms\n", currentTime);
        Serial.println("------------------------------\n");
    }
    
    // Petite pause pour éviter de surcharger le processeur
    delay(1);
}

// Test des autres composants du hardware
void testOtherComponents(uint32_t cardId) {
    Serial.println("\n>>> TEST AUTRES COMPOSANTS <<<");
    
    // Test Digital I/O
    int ledState = hardware->digital->ReadDigital(LED_PIN);
    Serial.printf("État LED (pin %d): %s\n", LED_PIN, ledState ? "HIGH" : "LOW");
    
    // Test ROM - Sauvegarder l'ID de la carte
    Serial.printf("Sauvegarde ID carte en ROM: %u\n", cardId);
    hardware->rom->SaveToROM(0, cardId);
    
    // Lecture de vérification
    int savedId = hardware->rom->ReadFromROM(0);
    Serial.printf("ID lu depuis ROM: %u %s\n", savedId, 
                  (savedId == (int)cardId) ? "✓" : "✗");
    
    // Test File System (optionnel)
    if (hardware->file->OpenFile("/test_rfid.txt", false)) {
        String data = "CardID:" + String(cardId) + "\n";
        hardware->file->Write(data.c_str(), data.length());
        hardware->file->CloseFile();
        Serial.println("Données RFID sauvées dans fichier ✓");
    } else {
        Serial.println("Erreur ouverture fichier ✗");
    }
    
    Serial.println(">>> FIN TEST COMPOSANTS <<<\n");
}

// Fonctions utilitaires pour debug avancé
void printDetailedHardwareStatus() {
    Serial.println("\n╔══════════════════════════════════╗");
    Serial.println("║      DEBUG ARDUINO HARDWARE      ║");
    Serial.println("╠══════════════════════════════════╣");
    Serial.printf("║ RFID - Détectée : %-14s ║\n", hardware->rfid->isCardDetected() ? "OUI" : "NON");
    Serial.printf("║ RFID - Valide   : %-14s ║\n", hardware->rfid->isValid() ? "OUI" : "NON");
    Serial.printf("║ RFID - ID       : 0x%08X      ║\n", hardware->rfid->getCardId());
    
    // Cast to access ReemasRfid and ReemasRom specific methods
    ReemasRfid* rfid = static_cast<ReemasRfid*>(hardware->rfid);
    ReemasRom* romImpl = static_cast<ReemasRom*>(hardware->rom);
    
    Serial.printf("║ RFID - Erreurs  : %-14lu ║\n", rfid->getErrorCount());
    Serial.printf("║ ROM - Taille    : %-14d ║\n", romImpl->GetRomSize());
    Serial.printf("║ ROM - Début     : %-14d ║\n", romImpl->GetBeginAddress());
    Serial.println("╚══════════════════════════════════╝\n");
}

// Fonction pour reset manuel du système
void resetHardwareRfid() {
    Serial.println(">>> RESET MANUEL VIA ARDUINO HARDWARE <<<");
    hardware->rfid->reset();
    
    // Cast to access ReemasRfid-specific methods
    ReemasRfid* rfid = static_cast<ReemasRfid*>(hardware->rfid);
    rfid->clearErrorCount();
    Serial.println(">>> RESET TERMINÉ <<<\n");
}

// Test complet des pins via ArduinoHardware
void testPinsViaHardware() {
    Serial.println("\n>>> TEST DES PINS VIA ARDUINO HARDWARE <<<");
    Serial.printf("DATA0 (pin %d): %s\n", DATA0_PIN, 
                  hardware->digital->ReadDigital(DATA0_PIN) ? "HIGH" : "LOW");
    Serial.printf("DATA1 (pin %d): %s\n", DATA1_PIN, 
                  hardware->digital->ReadDigital(DATA1_PIN) ? "HIGH" : "LOW");
    Serial.printf("CP (pin %d)   : %s\n", CP_PIN, 
                  hardware->digital->ReadDigital(CP_PIN) ? "HIGH" : "LOW");
    Serial.println(">>> FIN TEST PINS <<<\n");
}