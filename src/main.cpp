#include <Arduino.h>
#include "../include/BSP.hpp"

#include "../lib/Infrastructure/RfidCommunication/RfidCommunication.hpp"
#include "../lib/Infrastructure/RfidCommunication/RfidCommunication.cpp"

BSP bsp;
RfidCommunication rfid(bsp, BSP::DATA0_PIN, BSP::DATA1_PIN, BSP::CP_PIN);

void setup()
{
  bsp.initSerial(115200);
  bsp.println("Starting RFID reader...");

  rfid.begin();
}

void loop()
{
  rfid.handleEvent(RfidCommunication::Event::TIMEOUT);

  if (rfid.isCardDetected() && rfid.isValid())
  {
    uint32_t cardId = rfid.getCardId();
    bsp.printf("Valid RFID Tag received: %u (0x%08X)\n", cardId, cardId);
  }

  delay(10);
}