/*#include "PlatformHardware.h"
#include "RfidDatabase.h"
#include"../lib/Spc/SpcBusiness.h"
#include "ReemasProtocol.h"
//#include "Infrastructure.h"
//class ReemasSerial
#include "ReemasRfid.h"*/
#include "PlatformHardware.h"
#include <EEPROM.h>
#include <LittleFS.h>

ReemasSerial::ReemasSerial(HardwareSerial &s)
{
  hSerial = &s;
}

ReemasSerial::ReemasSerial(HardwareSerial &s, size_t maxBufferRx)
{
  hSerial = &s;
  hSerial->setRxBufferSize(maxBufferRx);
}

int ReemasSerial::availableForReading()
{
  return hSerial->available();
}
int ReemasSerial::availableForWriting()
{
  return hSerial->availableForWrite();
}
int ReemasSerial::read(char *buffer, int sizeBuffer)
{
  return hSerial->read(buffer, sizeBuffer);
}
int ReemasSerial::write(char *buffer, int sizeBuffer)
{
  return hSerial->write(buffer, sizeBuffer);
}
bool ReemasSerial::isErrorReading()
{
  return false; // to be reviewed
}
bool ReemasSerial::isErrorWriting()
{
  return hSerial->getWriteError();
}

void ReemasSerial::flush()
{
  hSerial->flush(false);
}

// class ReemasTimer
ReemasTimer::ReemasTimer(hw_timer_t &ht)
{
  _timerComponent = &ht;
}

void ReemasTimer::AttachInterrupt(void (*fn)(void), bool edge)
{
  timerAttachInterrupt(this->_timerComponent, fn, edge);
}

void ReemasTimer::AlarmWrite(int count, bool loop)
{
  timerAlarmWrite(this->_timerComponent, count, loop);
}
void ReemasTimer::AlarmEnable()
{
  timerAlarmEnable(this->_timerComponent);
}
void ReemasTimer::AlarmDisable()
{
  timerAlarmDisable(this->_timerComponent);
}

hw_timer_t *ReemasTimer::GetArduinoTimer()
{
  return _timerComponent;
}

// class DigitalIO
int ReemasDigital::ReadDigital(int gpio)
{
  return digitalRead(gpio);
}

void ReemasDigital::WriteDigital(int gpio, int value)
{
  digitalWrite(gpio, value);
}

// class ReemasRom

ReemasRom::ReemasRom(int eepromSize, int beginAddress, int valueSize)
{
  EEPROM.begin(eepromSize);
  this->_eepromSize = eepromSize;
  this->_beginAddress = beginAddress;
  this->_valueSize = valueSize;
}
int ReemasRom::GetRomSize()
{
  return this->_eepromSize;
}

int ReemasRom::GetBeginAddress()
{
  return this->_beginAddress;
}

int ReemasRom::GetValueSize()
{
  return this->_valueSize;
}

void ReemasRom::SaveByteToROM(char address, char data)
{
  EEPROM.write(address, data);
  EEPROM.commit();
}
char ReemasRom::ReadByteFromROM(char address)
{
  return (char)EEPROM.read(address);
}

void ReemasRom::SaveToROM(int index, int value)
{
  for (int i = 0; i < this->_valueSize; i++)
  { // COUNTER_SIZE
    char data = (value >> (8 * i)) & 0xff;
    SaveByteToROM((char)(this->_beginAddress + (this->_valueSize * index) + i), data); //(char)(ADDRESS_BEGIN_COUNTER+ (COUNTER_SIZE * index) + i)
  }
}

int ReemasRom::ReadFromROM(int index)
{
  int value = 0;
  for (int i = this->_valueSize - 1; i >= 0; i--)
  {
    int data = ReadByteFromROM((char)(this->_beginAddress + (this->_valueSize * index) + i));
    value |= (data << (8 * i));
  }
  return value;
}

// class ReemasFs

ReemasFs::ReemasFs()
{
  //_fs = fs;
  if (!LittleFS.begin())
    isFsOk = false;
  else
    isFsOk = true;
}
ReemasFs::~ReemasFs()
{
  // delete(file);
  LittleFS.end();
  isFsOk = false;
  // delete(_fs);
}
bool ReemasFs::OpenFile(const std::string &pathname, bool isReadOnly /*= true*/)
{
  if (isFsOk)
  {
    if (isReadOnly)
      file = LittleFS.open(String(pathname.c_str()), "r");
    else
      file = LittleFS.open(String(pathname.c_str()), "w");
    size = file.available();
    return true;
  }
  return false;
}

void ReemasFs::CloseFile()
{
  file.close();
}

int ReemasFs::GetFileSize()
{
  return size;
}
void ReemasFs::Write(const std::string &s)
{
  file.write((uint8_t *)(s.c_str()), (size_t)s.size());
}
void ReemasFs::Write(const char *SendBuffer, const int size)
{
  file.write((uint8_t *)(SendBuffer), (size_t)size);
}
std::string ReemasFs::Read()
{
  std::string s = std::string(file.readStringUntil('\n').c_str());
  return s;
}
int ReemasFs::Read(char *Receptionbuffer, int sizeBuffer)
{
  return file.readBytes(Receptionbuffer, sizeBuffer);
}

// ============================================================================
// IMPLÉMENTATION RFID CORRIGÉE
// ============================================================================

ReemasRfid *ReemasRfid::_instance = nullptr;

ReemasRfid::ReemasRfid(uint8_t data0Pin, uint8_t data1Pin, uint8_t cpPin)
    : _data0Pin(data0Pin), _data1Pin(data1Pin), _cpPin(cpPin),
      _currentStateId(StateId::IDLE), _cardData(0), _lastBitTime(0),
      _isValid(false), _cardId(0), _bitCount(0)
{
  _instance = this;
}

void ReemasRfid::begin()
{
  pinMode(_data0Pin, INPUT_PULLUP);
  pinMode(_data1Pin, INPUT_PULLUP);
  pinMode(_cpPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(_data0Pin), data0Interrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(_data1Pin), data1Interrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(_cpPin), cpInterrupt, FALLING);
}

void ReemasRfid::handleEvent(Event event)
{
  switch (_currentStateId)
  {
  case StateId::IDLE:
    processIdleState(event);
    break;
  case StateId::RECEIVING:
    processReceivingState(event);
    break;
  case StateId::PROCESSING:
    processProcessingState(event);
    break;
  }
}

bool ReemasRfid::isCardDetected() const
{
  return _cardDetected;
}

uint32_t ReemasRfid::getCardId() const
{
  return _cardId;
}

bool ReemasRfid::isValid() const
{
  return _isValid;
}

void ReemasRfid::reset()
{
  _bitCount = 0;
  _cardData = 0;
  _cardId = 0;
  _cardDetected = false;
  _isValid = false;
  _currentStateId = StateId::IDLE;
}

// NOUVELLE MÉTHODE : Vérification périodique du timeout
void ReemasRfid::checkTimeout()
{
  if (_currentStateId == StateId::RECEIVING &&
      millis() - _lastBitTime >= TIMEOUT_MS)
  {
    handleEvent(Event::TIMEOUT);
  }
}

// NOUVELLE MÉTHODE : Marquer la carte comme traitée
void ReemasRfid::markCardProcessed()
{
  _cardDetected = false;
  _currentStateId = StateId::IDLE;
  reset();
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

// MÉTHODE CORRIGÉE : Protection contre les interruptions
void ReemasRfid::addBit(bool bit)
{
  // Protection critique
  noInterrupts();

  if (_bitCount < WIEGAND_BIT_COUNT)
  {
    _cardData <<= 1;
    _cardData |= bit ? 1 : 0;
    _bitCount++;
    _lastBitTime = millis();

    if (_bitCount == WIEGAND_BIT_COUNT)
    {
      interrupts(); // Réactiver avant l'événement
      handleEvent(Event::BIT_RECEIVED);
      return;
    }
  }

  interrupts(); // Réactiver les interruptions
}

// MÉTHODE CORRIGÉE : Validation de parité améliorée
bool ReemasRfid::validateParity()
{
  if (_bitCount != WIEGAND_BIT_COUNT)
  {
#ifdef DEBUG_RFID
    Serial.printf("Invalid bit count: %d (expected %d)\n", _bitCount, WIEGAND_BIT_COUNT);
#endif
    return false;
  }

  uint64_t data = _cardData;
  bool evenParity = (data >> 33) & 1;
  bool oddParity = data & 1;

  uint8_t calculatedEvenParity = 0;
  uint8_t calculatedOddParity = 1;

  // Calculate even parity (bits 1-16 after MSB parity bit)
  for (int i = 1; i <= 16; i++)
  {
    calculatedEvenParity ^= (data >> (33 - i)) & 1;
  }

  // Calculate odd parity (bits 17-32 before LSB parity bit)
  for (int i = 17; i <= 32; i++)
  {
    calculatedOddParity ^= (data >> (33 - i)) & 1;
  }

#ifdef DEBUG_RFID
  Serial.printf("Card Data: 0x%010llX\n", data);
  Serial.printf("Bit Count: %d\n", _bitCount);
  Serial.printf("Even Parity - Received: %d, Calculated: %d\n", evenParity, calculatedEvenParity);
  Serial.printf("Odd Parity - Received: %d, Calculated: %d\n", oddParity, calculatedOddParity);
#endif

  return (evenParity == calculatedEvenParity) && (oddParity == calculatedOddParity);
}

void ReemasRfid::processIdleState(Event event)
{
  switch (event)
  {
  case Event::BIT_RECEIVED:
    _currentStateId = StateId::RECEIVING;
    break;
  case Event::CARD_DETECTED:
#ifdef DEBUG_RFID
    Serial.println("Card detected in idle state");
#endif
    break;
  default:
    break;
  }
}

void ReemasRfid::processReceivingState(Event event)
{
  switch (event)
  {
  case Event::BIT_RECEIVED:
    if (_bitCount == WIEGAND_BIT_COUNT)
    {
      _currentStateId = StateId::PROCESSING;
      handleEvent(Event::TIMEOUT);
    }
    break;
  case Event::TIMEOUT:
    if (millis() - _lastBitTime >= TIMEOUT_MS)
    {
      _currentStateId = StateId::PROCESSING;
      handleEvent(Event::TIMEOUT);
    }
    break;
  default:
    break;
  }
}

// MÉTHODE CORRIGÉE : Gestion d'état améliorée
void ReemasRfid::processProcessingState(Event event)
{
  switch (event)
  {
  case Event::TIMEOUT:
#ifdef DEBUG_RFID
    Serial.printf("Complete bits received: %010llX\n", _cardData);
#endif

    _isValid = validateParity();

    if (_isValid && _bitCount == WIEGAND_BIT_COUNT)
    {
      _cardId = (_cardData >> 1) & 0xFFFFFFFF;

#ifdef DEBUG_RFID
      Serial.println("Valid tag detected!");
      Serial.printf("Card ID: %u (0x%08X)\n", _cardId, _cardId);
#endif
      _cardDetected = true;
      // Rester en état PROCESSING jusqu'à ce que markCardProcessed() soit appelée
    }
    else
    {
#ifdef DEBUG_RFID
      Serial.println("Parity error or incomplete data, invalid tag.");
#endif
      handleEvent(Event::INVALID_PARITY);
    }
    break;

  case Event::INVALID_PARITY:
    _cardId = 0;
    _isValid = false;
    _cardDetected = false;
    _currentStateId = StateId::IDLE;
    reset();
    break;

  default:
    break;
  }
}