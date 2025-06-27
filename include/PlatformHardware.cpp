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

// ===================================================================
// ReemasSerial Implementation
// ===================================================================

ReemasSerial::ReemasSerial(HardwareSerial& s){
    hSerial = &s;
}

ReemasSerial::ReemasSerial(HardwareSerial& s, size_t maxBufferRx){
    hSerial = &s;
    hSerial->setRxBufferSize(maxBufferRx);
}

int ReemasSerial::availableForReading(){
    return hSerial->available();
}

int ReemasSerial::availableForWriting(){
    return hSerial->availableForWrite();
}

int ReemasSerial::read(char* buffer, int sizeBuffer){
    return hSerial->read(buffer,sizeBuffer);
}

int ReemasSerial::write(char* buffer, int sizeBuffer){
    return hSerial->write(buffer, sizeBuffer);
}

bool ReemasSerial::isErrorReading(){
    return false;// to be reviewed
}

bool ReemasSerial::isErrorWriting(){
    return hSerial->getWriteError();
}

void ReemasSerial::flush(){
    hSerial->flush(false);
}

// ===================================================================
// ReemasTimer Implementation
// ===================================================================

ReemasTimer::ReemasTimer(hw_timer_t& ht){
    _timerComponent = &ht;
}

void ReemasTimer::AttachInterrupt(void (*fn)(void), bool edge){
    timerAttachInterrupt(this->_timerComponent, fn, edge);
}

void ReemasTimer::AlarmWrite( int count, bool loop){
    timerAlarmWrite(this->_timerComponent, count,  loop);
}

void ReemasTimer::AlarmEnable(){
    timerAlarmEnable(this->_timerComponent);
}

void ReemasTimer::AlarmDisable(){
    timerAlarmDisable(this->_timerComponent);
}

hw_timer_t* ReemasTimer::GetArduinoTimer(){
    return _timerComponent;
}

// ===================================================================
// ReemasDigital Implementation
// ===================================================================

int ReemasDigital::ReadDigital(int gpio){
    return digitalRead(gpio);
}

void ReemasDigital::WriteDigital(int gpio, int value){
    digitalWrite(gpio,value);
}

// ===================================================================
// ReemasRom Implementation
// ===================================================================

ReemasRom::ReemasRom(int eepromSize,int beginAddress, int valueSize){
     EEPROM.begin(eepromSize);
     this->_eepromSize = eepromSize;
     this->_beginAddress = beginAddress;
     this->_valueSize = valueSize;
}

int ReemasRom::GetRomSize(){
    return this->_eepromSize;
}

int ReemasRom::GetBeginAddress(){
    return this->_beginAddress;
}

int ReemasRom::GetValueSize(){
    return this->_valueSize;
}

void ReemasRom::SaveByteToROM(char address,char data){
    EEPROM.write(address, data);
    EEPROM.commit();
}

char ReemasRom::ReadByteFromROM(char address){
    return (char)EEPROM.read(address);
}

void ReemasRom::SaveToROM(int index,int value){
    for(int i=0; i < this->_valueSize ; i++){ //COUNTER_SIZE
      char data = (value >> (8*i)) & 0xff;
      SaveByteToROM((char)(this->_beginAddress + (this->_valueSize * index) + i),data); //(char)(ADDRESS_BEGIN_COUNTER+ (COUNTER_SIZE * index) + i)
    }
}
    
int ReemasRom::ReadFromROM(int index){
    int value=0;
    for(int i= this->_valueSize -1; i>=0 ; i--){
      int data = ReadByteFromROM((char)(this->_beginAddress + (this->_valueSize * index) + i));
      value |= (data << (8*i)) ;
    }
    return value;
}

// ===================================================================
// ReemasFs Implementation
// ===================================================================

ReemasFs::ReemasFs(){
    //_fs = fs;
    if(!LittleFS.begin())
        isFsOk =false;
    else
        isFsOk =true;
}

ReemasFs::~ReemasFs(){
    //delete(file);
    LittleFS.end();
    isFsOk =false;
    //delete(_fs); 
}

bool ReemasFs::OpenFile(const std::string& pathname, bool isReadOnly /*= true*/){
   if(isFsOk){
        if(isReadOnly)
            file = LittleFS.open(String(pathname.c_str()),"r");
        else
            file = LittleFS.open(String(pathname.c_str()),"w");
        size = file.available();
        return true;
    }
    return false;
}

void ReemasFs::CloseFile(){
    file.close();
}

int ReemasFs::GetFileSize(){
    return size;
}

void ReemasFs::Write(const std::string& s) {
    file.write((uint8_t*)(s.c_str()),(size_t)s.size());
}

void ReemasFs::Write(const char* SendBuffer, const int size) {
    file.write((uint8_t*)(SendBuffer),(size_t)size);
}

std::string ReemasFs::Read() {
    std::string s= std::string(file.readStringUntil('\n').c_str());
    return s;
}

int ReemasFs::Read(char * Receptionbuffer, int sizeBuffer) {
    return file.readBytes(Receptionbuffer,sizeBuffer);
}

// ===================================================================
// ReemasRfid Implementation - Version Améliorée
// ===================================================================

ReemasRfid *ReemasRfid::_instance = nullptr;

ReemasRfid::ReemasRfid(uint8_t data0Pin, uint8_t data1Pin, uint8_t cpPin)
    : _data0Pin(data0Pin), _data1Pin(data1Pin), _cpPin(cpPin),
      _currentStateId(StateId::IDLE), _cardData(0), _lastBitTime(0),
      _isValid(false), _cardId(0), _bitCount(0), _cardDetected(false),
      _errorCount(0), _lastCardTime(0)
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
  
  reset();
  
#ifdef DEBUG_RFID
  Serial.println("ReemasRfid initialized");
  Serial.printf("Pins - DATA0: %d, DATA1: %d, CP: %d\n", _data0Pin, _data1Pin, _cpPin);
#endif
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
  return _currentStateId == StateId::PROCESSING && _isValid;
}

uint32_t ReemasRfid::getCardId() const
{
  return _cardId;
}

bool ReemasRfid::isValid() const
{
  return _isValid;
}

bool ReemasRfid::hasNewCard()
{
  bool result = _cardDetected && _isValid;
  if (result)
  {
    _cardDetected = false; // Clear flag after reading
#ifdef DEBUG_RFID
    Serial.println("New card flag cleared");
#endif
  }
  return result;
}

uint32_t ReemasRfid::getErrorCount() const
{
  return _errorCount;
}

void ReemasRfid::clearErrorCount()
{
  _errorCount = 0;
}

void ReemasRfid::reset()
{
  noInterrupts();
  _bitCount = 0;
  _cardData = 0;
  _cardId = 0;
  _cardDetected = false;
  _isValid = false;
  _currentStateId = StateId::IDLE;
  _lastBitTime = 0;
  interrupts();
  
#ifdef DEBUG_RFID
  Serial.println("ReemasRfid reset");
#endif
}

void ReemasRfid::checkTimeout()
{
  if (_currentStateId == StateId::RECEIVING)
  {
    handleEvent(Event::TIMEOUT);
  }
}

void IRAM_ATTR ReemasRfid::data0Interrupt()
{
  if (_instance)
  {
#ifdef DEBUG_RFID_VERBOSE
    Serial.println("DATA0 interrupt");
#endif
    _instance->addBit(0);
  }
}

void IRAM_ATTR ReemasRfid::data1Interrupt()
{
  if (_instance)
  {
#ifdef DEBUG_RFID_VERBOSE
    Serial.println("DATA1 interrupt");
#endif
    _instance->addBit(1);
  }
}

void IRAM_ATTR ReemasRfid::cpInterrupt()
{
  if (_instance)
  {
#ifdef DEBUG_RFID_VERBOSE
    Serial.println("CP interrupt");
#endif
    _instance->handleEvent(Event::CARD_DETECTED);
  }
}

void ReemasRfid::addBit(bool bit)
{
  unsigned long currentTime = millis();
  
  // Vérification du timeout entre les bits (Wiegand standard: < 2ms)
  if (_bitCount > 0 && (currentTime - _lastBitTime) > INTER_BIT_TIMEOUT_MS)
  {
#ifdef DEBUG_RFID
    Serial.printf("Inter-bit timeout detected. Resetting. Gap: %lu ms\n", currentTime - _lastBitTime);
#endif
    _bitCount = 0;
    _cardData = 0;
    _errorCount++;
  }
  
  if (_bitCount < WIEGAND_BIT_COUNT)
  {
    _cardData <<= 1;
    _cardData |= bit ? 1 : 0;
    _bitCount++;
    _lastBitTime = currentTime;
    
    // Transition vers RECEIVING si c'est le premier bit
    if (_bitCount == 1)
    {
      _currentStateId = StateId::RECEIVING;
#ifdef DEBUG_RFID
      Serial.println("First bit received - switching to RECEIVING state");
#endif
    }
    
#ifdef DEBUG_RFID_VERBOSE
    Serial.printf("Bit %d: %d, Total bits: %d, Data: 0x%010llX\n", 
                  _bitCount, bit, _bitCount, _cardData);
#endif
    
    // Si on a reçu tous les bits, traiter
    if (_bitCount == WIEGAND_BIT_COUNT)
    {
#ifdef DEBUG_RFID
      Serial.printf("All %d bits received: 0x%010llX\n", WIEGAND_BIT_COUNT, _cardData);
#endif
      handleEvent(Event::BIT_RECEIVED);
    }
  }
  else
  {
    // Trop de bits reçus - erreur
#ifdef DEBUG_RFID
    Serial.println("Too many bits received - resetting");
#endif
    _errorCount++;
    reset();
  }
}

bool ReemasRfid::validateParity()
{
  if (_bitCount != WIEGAND_BIT_COUNT) 
  {
#ifdef DEBUG_RFID
    Serial.printf("Invalid bit count: %d (expected %d)\n", _bitCount, WIEGAND_BIT_COUNT);
#endif
    return false;
  }
  
  // Extraction des bits de parité
  bool evenParity = (_cardData >> 33) & 1;  // MSB est la parité paire
  bool oddParity = _cardData & 1;           // LSB est la parité impaire
  
  // Calcul de la parité paire pour les bits 1-16 (après le bit de parité paire)
  uint8_t calculatedEvenParity = 0;
  for (int i = 1; i <= 16; i++)
  {
    calculatedEvenParity ^= (_cardData >> (33 - i)) & 1;
  }
  
  // Calcul de la parité impaire pour les bits 17-32 (avant le bit de parité impaire)
  uint8_t calculatedOddParity = 0;
  for (int i = 17; i <= 32; i++)
  {
    calculatedOddParity ^= (_cardData >> (33 - i)) & 1;
  }
  
  // Pour la parité impaire, on veut un nombre impair de 1s au total
  calculatedOddParity ^= 1;

#ifdef DEBUG_RFID
  Serial.println("=== Parity Validation ===");
  Serial.printf("Card Data: 0x%010llX\n", _cardData);
  Serial.printf("Even Parity - Received: %d, Calculated: %d %s\n", 
                evenParity, calculatedEvenParity, 
                (evenParity == calculatedEvenParity) ? "✓" : "✗");
  Serial.printf("Odd Parity  - Received: %d, Calculated: %d %s\n", 
                oddParity, calculatedOddParity,
                (oddParity == calculatedOddParity) ? "✓" : "✗");
#endif

  bool isValid = (evenParity == calculatedEvenParity) && (oddParity == calculatedOddParity);
  
  if (!isValid)
  {
    _errorCount++;
  }
  
  return isValid;
}

void ReemasRfid::processIdleState(Event event)
{
  switch (event)
  {
  case Event::BIT_RECEIVED:
    // Ne devrait pas arriver car on change d'état dans addBit()
    _currentStateId = StateId::RECEIVING;
#ifdef DEBUG_RFID
    Serial.println("Unexpected BIT_RECEIVED in IDLE - switching to RECEIVING");
#endif
    break;
    
  case Event::CARD_DETECTED:
#ifdef DEBUG_RFID
    Serial.println("Card presence detected");
#endif
    // Le CP peut être utilisé pour détecter la présence d'une carte
    // Mais on attend les données via DATA0/DATA1
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
      handleEvent(Event::TIMEOUT); // Traiter immédiatement
    }
    break;
    
  case Event::TIMEOUT:
    // Vérifier si on doit faire un timeout à cause d'une transmission incomplète
    if (millis() - _lastBitTime >= TIMEOUT_MS && _bitCount > 0 && _bitCount < WIEGAND_BIT_COUNT)
    {
#ifdef DEBUG_RFID
      Serial.printf("Transmission timeout. Bits received: %d/%d\n", _bitCount, WIEGAND_BIT_COUNT);
#endif
      _errorCount++;
      reset(); // Retour à IDLE
    }
    break;
    
  default:
    break;
  }
}

void ReemasRfid::processProcessingState(Event event)
{
  switch (event)
  {
  case Event::TIMEOUT:
  {
    unsigned long currentTime = millis();
    
#ifdef DEBUG_RFID
    Serial.printf("Processing transmission: 0x%010llX\n", _cardData);
#endif

    _isValid = validateParity();

    if (_isValid)
    {
      // Extraction de l'ID de la carte (bits 1-32, sans les bits de parité)
      _cardId = (_cardData >> 1) & 0xFFFFFFFF;
      
      // Vérifier si c'est une nouvelle carte (éviter les lectures multiples)
      if (_cardId != _lastCardId || (currentTime - _lastCardTime) > DUPLICATE_CARD_MS)
      {
        _cardDetected = true;
        _lastCardId = _cardId;
        _lastCardTime = currentTime;
        
#ifdef DEBUG_RFID
        Serial.println("=== VALID CARD DETECTED ===");
        Serial.printf("Card ID: %u (0x%08X)\n", _cardId, _cardId);
        Serial.printf("Facility Code: %u\n", (_cardId >> 16) & 0xFF);
        Serial.printf("Card Number: %u\n", _cardId & 0xFFFF);
        Serial.println("===========================");
#endif
      }
      else
      {
#ifdef DEBUG_RFID
        Serial.println("Duplicate card read ignored");
#endif
      }
    }
    else
    {
#ifdef DEBUG_RFID
      Serial.println("Parity error - invalid card");
#endif
      _cardId = 0;
    }

    // Toujours retourner à IDLE après traitement
    _currentStateId = StateId::IDLE;
    break;
  }

  case Event::INVALID_PARITY:
    // Ajouter un débounce pour éviter les changements d'état rapides
    if (millis() - _lastBitTime >= DEBOUNCE_MS)
    {
#ifdef DEBUG_RFID
      Serial.println("Invalid parity - resetting");
#endif
      _cardId = 0;
      _isValid = false;
      _errorCount++;
      reset();
    }
    break;
    
  default:
    break;
  }
}