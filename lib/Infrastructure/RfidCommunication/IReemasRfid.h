#ifndef IRFID_COMMUNICATION_HPP
#define IRFID_COMMUNICATION_HPP

#include <Arduino.h>
#include "BSP.hpp"

class RfidState;

// Interface pour la communication RFID
class IReemasRfid
{
public:
  enum class Event
  {
    BIT_RECEIVED,
    TIMEOUT,
    CARD_DETECTED,
    INVALID_PARITY
  };

  enum class StateId
  {
    IDLE,
    RECEIVING,
    PROCESSING
  };

  virtual ~IReemasRfid() = default;
  
  // Méthodes publiques de l'interface
  virtual void begin() = 0;
  virtual void handleEvent(Event event) = 0;
  virtual bool isCardDetected() const = 0;
  virtual uint32_t getCardId() const = 0;
  virtual bool isValid() const = 0;
  virtual void reset() = 0;
};
#endif