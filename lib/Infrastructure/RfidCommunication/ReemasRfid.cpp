#include "ReemasRfid.hpp"

ReemasRfid *ReemasRfid::_instance = nullptr;

ReemasRfid::ReemasRfid(BSP &bsp, uint8_t data0Pin, uint8_t data1Pin, uint8_t cpPin)
    : _bsp(bsp), _data0Pin(data0Pin), _data1Pin(data1Pin), _cpPin(cpPin),
      _currentState(new IdleState()), _currentStateId(StateId::IDLE),
      _cardData(0), _lastBitTime(0), _isValid(false), _cardId(0), _bitCount(0)
{
  _instance = this;
}

void ReemasRfid::begin()
{
  _bsp.pinMode(_data0Pin, INPUT_PULLUP);
  _bsp.pinMode(_data1Pin, INPUT_PULLUP);
  _bsp.pinMode(_cpPin, INPUT_PULLUP);
  _bsp.attachInterrupt(_data0Pin, data0Interrupt, FALLING);
  _bsp.attachInterrupt(_data1Pin, data1Interrupt, FALLING);
  _bsp.attachInterrupt(_cpPin, cpInterrupt, FALLING);
}

void ReemasRfid::handleEvent(Event event)
{
  _currentState->handleEvent(*this, event);
}

bool ReemasRfid::isCardDetected() const
{
  return _currentStateId == StateId::PROCESSING;
}

uint32_t ReemasRfid::getCardId() const
{
  return _cardId;
}

bool ReemasRfid::isValid() const
{
  return _isValid;
}

void IRAM_ATTR ReemasRfid::data0Interrupt()
{
  if (_instance)
  {
    _instance->addBit(0);
  }
}

void IRAM_ATTR ReemasRfid::data1Interrupt()
{
  if (_instance)
  {
    _instance->addBit(1);
  }
}

void IRAM_ATTR ReemasRfid::cpInterrupt()
{
  if (_instance)
  {
    _instance->handleEvent(Event::CARD_DETECTED);
  }
}

void ReemasRfid::addBit(bool bit)
{
  if (_bitCount < WIEGAND_BIT_COUNT)
  {
    _cardData <<= 1;
    _cardData |= bit ? 1 : 0;
    _bitCount++;
    _lastBitTime = _bsp.millis();

    if (_bitCount == WIEGAND_BIT_COUNT)
    {
      handleEvent(Event::BIT_RECEIVED);
    }
  }
}

bool ReemasRfid::validateParity()
{
  uint64_t data = _cardData & 0x3FFFFFFFFULL; // 34 bits
  bool evenParity = (_cardData >> 33) & 1;
  bool oddParity = _cardData & 1;

  uint8_t calculatedEvenParity = 0;
  uint8_t calculatedOddParity = 1; // Initialize to 1 for odd parity

  // Calculate even parity (bits 2-17)
  for (int i = 1; i <= 16; i++)
  {
    calculatedEvenParity ^= (data >> (32 - i)) & 1;
  }

  // Calculate odd parity (bits 18-33)
  for (int i = 17; i <= 32; i++)
  {
    calculatedOddParity ^= (data >> (32 - i)) & 1;
  }

  _bsp.println("Parity Validation Details:");
  _bsp.printf("Received Even Parity: %d, Calculated: %d\n", evenParity, calculatedEvenParity);
  _bsp.printf("Received Odd Parity: %d, Calculated: %d\n", oddParity, calculatedOddParity);
  _bsp.printf("Even Parity Valid: %s, Odd Parity Valid: %s\n",
              evenParity == calculatedEvenParity ? "Yes" : "No",
              oddParity == calculatedOddParity ? "Yes" : "No");

  return (evenParity == calculatedEvenParity) && (oddParity == calculatedOddParity);
}

void ReemasRfid::changeState(RfidState *newState, StateId newStateId)
{
  delete _currentState;
  _currentState = newState;
  _currentStateId = newStateId;
}

void IdleState::handleEvent(ReemasRfid &rfid, ReemasRfid::Event event)
{
  switch (event)
  {
  case ReemasRfid::Event::BIT_RECEIVED:
    rfid.changeState(new ReceivingState(), ReemasRfid::StateId::RECEIVING);
    break;
  case ReemasRfid::Event::CARD_DETECTED:
    rfid._bsp.println("Card detected in idle state");
    break;
  default:
    break;
  }
}

void ReceivingState::handleEvent(ReemasRfid &rfid, ReemasRfid::Event event)
{
  switch (event)
  {
  case ReemasRfid::Event::BIT_RECEIVED:
    if (rfid._bitCount == ReemasRfid::WIEGAND_BIT_COUNT)
    {
      rfid.changeState(new ProcessingState(), ReemasRfid::StateId::PROCESSING);
      rfid.handleEvent(ReemasRfid::Event::TIMEOUT);
    }
    break;
  case ReemasRfid::Event::TIMEOUT:
    if (rfid._bsp.millis() - rfid._lastBitTime >= ReemasRfid::TIMEOUT_MS)
    {
      rfid.changeState(new ProcessingState(), ReemasRfid::StateId::PROCESSING);
      rfid.handleEvent(ReemasRfid::Event::TIMEOUT);
    }
    break;
  default:
    break;
  }
}

void ProcessingState::handleEvent(ReemasRfid &rfid, ReemasRfid::Event event)
{
  switch (event)
  {
  case ReemasRfid::Event::TIMEOUT:
  rfid._bsp.printf("Complete bits received: %010llX\n", rfid._cardData);
  rfid._isValid = rfid.validateParity();

  if (rfid._isValid) {
    rfid._cardId = (rfid._cardData >> 1) & 0xFFFFFFFF;
    rfid._bsp.println("Valid tag detected!");
    rfid._bsp.printf("Card ID: %u (0x%08X)\n", rfid._cardId, rfid._cardId);

    // Marquer que la carte est prête pour traitement FSM
    rfid._cardDetected = true;

    // PAS de reset ici ! attendre que la FSM traite la carte
  } else {
    rfid._bsp.println("Parity error, invalid tag.");
    rfid.handleEvent(ReemasRfid::Event::INVALID_PARITY);
  }

  rfid.changeState(new IdleState(), ReemasRfid::StateId::IDLE);
  break;

  case ReemasRfid::Event::INVALID_PARITY:
    rfid._cardId = 0;
    rfid._isValid = false;
    break;
  default:
    break;
  }
}
void ReemasRfid::reset() {
  _bitCount = 0;
  _cardData = 0;
  _cardId = 0;
  _cardDetected = false;
  _isValid = false;
}
