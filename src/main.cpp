#include <Arduino.h>
#include "PlatformHardware.h"
#include "SPC.h"

// Variables globales pour les composants hardware
ReemasDigital digital;
hw_timer_t *timer0 = NULL;
HardwareSerial serial0(0);
ReemasRom reemasRom(512, 0, 4); // 512 bytes EEPROM, start at 0, 4 bytes per value
ReemasFs reemasFs;
ReemasRfid reemasRfid(DATA0_PIN, DATA1_PIN, CP_PIN); // Pins définis dans SPC.h

// Déclaration des pointeurs - initialisation dans setup()
ReemasTimer *reemasTimer = nullptr;
ReemasSerial *reemasSerial = nullptr;
ArduinoHardware *hardware = nullptr;

// Variables pour le test
unsigned long lastCheckTime = 0;
const unsigned long CHECK_INTERVAL = 100; // Vérifier toutes les 100ms
bool lastCardState = false;
unsigned long cardDetectedTime = 0;
const unsigned long CARD_PROCESSING_TIMEOUT = 2000; // 2 secondes pour traiter une carte

void setup()
{
  // Initialisation de la communication série pour debug
  Serial.begin(115200);
  Serial.println("=== TEST SERVICE RFID ===");
  Serial.println("Initialisation...");

  // Initialisation du timer
  timer0 = timerBegin(0, 80, true);
  reemasTimer = new ReemasTimer(*timer0);

  // Initialisation du serial
  serial0.begin(115200);
  reemasSerial = new ReemasSerial(serial0);

  // Création de l'infrastructure complète
  hardware = new ArduinoHardware(digital, *reemasTimer, *reemasSerial, reemasRom, reemasFs, reemasRfid);

  // Configuration des pins de sortie pour feedback visuel
  pinMode(LED_BPT, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Initialisation du service RFID
  reemasRfid.begin();
  Serial.println("Service RFID initialisé");
  Serial.printf("Pins utilisées: DATA0=%d, DATA1=%d, CP=%d\n", DATA0_PIN, DATA1_PIN, CP_PIN);

  // Test initial des pins
  Serial.println("Test des pins d'entrée:");
  Serial.printf("DATA0 (pin %d): %d\n", DATA0_PIN, digitalRead(DATA0_PIN));
  Serial.printf("DATA1 (pin %d): %d\n", DATA1_PIN, digitalRead(DATA1_PIN));
  Serial.printf("CP (pin %d): %d\n", CP_PIN, digitalRead(CP_PIN));

  Serial.println("Système prêt - Présentez une carte RFID...");
  Serial.println("==========================================");
}

void loop()
{
  unsigned long currentTime = millis();

  // Vérification périodique du timeout RFID
  if (currentTime - lastCheckTime >= CHECK_INTERVAL)
  {
    lastCheckTime = currentTime;

    // Vérifier le timeout pour la réception des données
    reemasRfid.checkTimeout();

    // Vérifier si une carte est détectée
    bool currentCardState = reemasRfid.isCardDetected();

    // Nouvelle carte détectée
    if (currentCardState && !lastCardState)
    {
      cardDetectedTime = currentTime;
      lastCardState = true;

      // Feedback visuel et sonore
      digitalWrite(LED_BPT, HIGH);
      digitalWrite(BUZZER, HIGH);
      delay(100);
      digitalWrite(BUZZER, LOW);

      // Affichage des informations de la carte
      Serial.println("\n*** CARTE DÉTECTÉE ***");
      Serial.printf("Temps: %lu ms\n", currentTime);
      Serial.printf("ID Carte: %u (0x%08X)\n", reemasRfid.getCardId(), reemasRfid.getCardId());
      Serial.printf("Validité: %s\n", reemasRfid.isValid() ? "VALIDE" : "INVALIDE");

      if (reemasRfid.isValid())
      {
        Serial.println("✓ Carte acceptée");
        // Ici vous pouvez ajouter votre logique métier
        // Exemple: vérifier dans une base de données, ouvrir une barrière, etc.
        testCardProcessing(reemasRfid.getCardId());
      }
      else
      {
        Serial.println("✗ Carte rejetée (erreur de parité)");
      }
      Serial.println("**********************\n");
    }

    // Gestion du timeout de traitement de carte
    if (currentCardState && (currentTime - cardDetectedTime >= CARD_PROCESSING_TIMEOUT))
    {
      Serial.println("Timeout: Fin du traitement de la carte");
      reemasRfid.markCardProcessed();
      digitalWrite(LED_BPT, LOW);
      lastCardState = false;
    }
  }

  // Autres tâches du système peuvent être ajoutées ici
  // ...
}

// Fonction de test pour simuler le traitement d'une carte
void testCardProcessing(uint32_t cardId)
{
  Serial.println("--- Simulation traitement carte ---");

  // Simulation d'une vérification en base de données
  delay(500); // Simule le temps de traitement

  // Exemple de logique métier simple
  if (cardId > 0)
  {
    Serial.printf("Carte ID %u: Accès autorisé\n", cardId);

    // Simulation d'ouverture de barrière
    Serial.println("Commande: Ouverture barrière");
    digitalWrite(BARRIER_OPEN, HIGH);
    delay(1000);
    digitalWrite(BARRIER_OPEN, LOW);

    // Feedback positif
    for (int i = 0; i < 3; i++)
    {
      digitalWrite(BUZZER, HIGH);
      delay(200);
      digitalWrite(BUZZER, LOW);
      delay(200);
    }
  }
  else
  {
    Serial.println("Carte invalide: Accès refusé");

    // Feedback négatif
    digitalWrite(BUZZER, HIGH);
    delay(1000);
    digitalWrite(BUZZER, LOW);
  }

  Serial.println("--- Fin traitement carte ---");
}

// Fonction utilitaire pour debug avancé
void printRfidDebugInfo()
{
  Serial.println("\n=== DEBUG RFID ===");
  Serial.printf("État actuel: %s\n", reemasRfid.isCardDetected() ? "Carte présente" : "Pas de carte");
  Serial.printf("Dernière carte valide: %s\n", reemasRfid.isValid() ? "Oui" : "Non");
  Serial.printf("ID dernière carte: %u\n", reemasRfid.getCardId());

  // État des pins
  Serial.println("État des pins:");
  Serial.printf("  DATA0: %d\n", digitalRead(DATA0_PIN));
  Serial.printf("  DATA1: %d\n", digitalRead(DATA1_PIN));
  Serial.printf("  CP: %d\n", digitalRead(CP_PIN));
  Serial.println("=================\n");
}

// Fonction appelable pour reset manuel du système RFID
void resetRfidSystem()
{
  Serial.println("Reset du système RFID...");
  reemasRfid.reset();
  digitalWrite(LED_BPT, LOW);
  digitalWrite(BUZZER, LOW);
  lastCardState = false;
  Serial.println("Reset terminé");
}