#include <Arduino.h>
#include "PlatformHardware.h"
#include "SPC.h"

// Variables globales pour les composants hardware
ReemasDigital digital;
ReemasRom reemasRom(512, 0, 4);
ReemasFs reemasFs;
ReemasRfid reemasRfid(IN_12, IN_13, IN_14); // Utilisation des définitions de SPC.h

// Variables pour le test
unsigned long lastCheckTime = 0;
const unsigned long CHECK_INTERVAL = 100; // Vérifier toutes les 100ms
bool lastCardState = false;
unsigned long cardDetectedTime = 0;
const unsigned long CARD_PROCESSING_TIMEOUT = 2000; // 2 secondes pour traiter une carte

// Déclarations de fonctions
void testCardProcessing(uint32_t cardId);
void printRfidDebugInfo();
void resetRfidSystem();

void setup()
{
  // Initialisation de la communication série pour debug
  Serial.begin(115200);
  delay(1000); // Attendre que le port série soit prêt

  Serial.println("=== TEST SERVICE RFID ===");
  Serial.println("Initialisation...");

  // Configuration des pins de sortie pour feedback visuel
  pinMode(HW_STATE, OUTPUT);  // LED_BPT
  pinMode(BUZ_Alarm, OUTPUT); // BUZZER
  pinMode(OUT_1, OUTPUT);     // BARRIER_OPEN pour test

  // État initial des sorties
  digitalWrite(HW_STATE, LOW);
  digitalWrite(BUZ_Alarm, LOW);
  digitalWrite(OUT_1, LOW);

  // Initialisation du service RFID
  Serial.println("Initialisation du service RFID...");
  reemasRfid.begin();

  Serial.println("Service RFID initialisé");
  Serial.printf("Pins utilisées: DATA0=%d, DATA1=%d, CP=%d\n", IN_12, IN_13, IN_14);

  // Test initial des pins
  Serial.println("Test des pins d'entrée RFID:");
  Serial.printf("DATA0 (pin %d): %d\n", IN_12, digitalRead(IN_12));
  Serial.printf("DATA1 (pin %d): %d\n", IN_13, digitalRead(IN_13));
  Serial.printf("CP (pin %d): %d\n", IN_14, digitalRead(IN_14));

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



    // Vérifier si une carte est détectée
    bool currentCardState = reemasRfid.isCardDetected();

    // Nouvelle carte détectée
    if (currentCardState && !lastCardState)
    {
      cardDetectedTime = currentTime;
      lastCardState = true;

      // Feedback visuel et sonore
      digitalWrite(HW_STATE, HIGH);
      digitalWrite(BUZ_Alarm, HIGH);
      delay(100);
      digitalWrite(BUZ_Alarm, LOW);

      // Affichage des informations de la carte
      Serial.println("\n*** CARTE DÉTECTÉE ***");
      Serial.printf("Temps: %lu ms\n", currentTime);
      Serial.printf("ID Carte: %u (0x%08X)\n", reemasRfid.getCardId(), reemasRfid.getCardId());
      Serial.printf("Validité: %s\n", reemasRfid.isValid() ? "VALIDE" : "INVALIDE");

      if (reemasRfid.isValid())
      {
        Serial.println("✓ Carte acceptée");
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
      digitalWrite(HW_STATE, LOW);
      lastCardState = false;
    }
  }

  // Commandes série pour debug (optionnel)
  if (Serial.available())
  {
    String command = Serial.readString();
    command.trim();

    if (command == "debug")
    {
      printRfidDebugInfo();
    }
    else if (command == "reset")
    {
      resetRfidSystem();
    }
    else if (command == "help")
    {
      Serial.println("Commandes disponibles:");
      Serial.println("  debug - Afficher infos debug RFID");
      Serial.println("  reset - Reset du système RFID");
      Serial.println("  help  - Afficher cette aide");
    }
  }
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
    digitalWrite(OUT_1, HIGH); // BARRIER_OPEN
    delay(1000);
    digitalWrite(OUT_1, LOW);

    // Feedback positif - 3 bips courts
    for (int i = 0; i < 3; i++)
    {
      digitalWrite(BUZ_Alarm, HIGH);
      delay(200);
      digitalWrite(BUZ_Alarm, LOW);
      delay(200);
    }
  }
  else
  {
    Serial.println("Carte invalide: Accès refusé");

    // Feedback négatif - 1 bip long
    digitalWrite(BUZ_Alarm, HIGH);
    delay(1000);
    digitalWrite(BUZ_Alarm, LOW);
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
  Serial.printf("Temps système: %lu ms\n", millis());

  // État des pins
  Serial.println("État des pins RFID:");
  Serial.printf("  DATA0 (pin %d): %d\n", IN_12, digitalRead(IN_12));
  Serial.printf("  DATA1 (pin %d): %d\n", IN_13, digitalRead(IN_13));
  Serial.printf("  CP (pin %d): %d\n", IN_14, digitalRead(IN_14));

  // État des sorties
  Serial.println("État des sorties:");
  Serial.printf("  LED (pin %d): %d\n", HW_STATE, digitalRead(HW_STATE));
  Serial.printf("  BUZZER (pin %d): %d\n", BUZ_Alarm, digitalRead(BUZ_Alarm));

  Serial.println("=================\n");
}

// Fonction appelable pour reset manuel du système RFID
void resetRfidSystem()
{
  Serial.println("Reset du système RFID...");
  reemasRfid.reset();
  digitalWrite(HW_STATE, LOW);
  digitalWrite(BUZ_Alarm, LOW);
  digitalWrite(OUT_1, LOW);
  lastCardState = false;
  cardDetectedTime = 0;
  Serial.println("Reset terminé - Système prêt");
}