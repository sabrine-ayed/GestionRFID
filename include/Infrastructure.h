#ifndef Infrastructure_HPP
#define Infrastructure_HPP
#include <string>
//#include <../../include/RfidDatabase.h>
//#include "RfidDatabase.h"
#define MAX_DEBOUNCE 3

/// @brief hight level abstraction defining a hardware component
class IHardwareComponent{
    
};
class RfidDatabase;
/// @brief Timer component service
class ITimer : public IHardwareComponent{
  public:
    virtual void AttachInterrupt(void (*fn)(void), bool edge)=0;
    virtual void AlarmWrite(int count, bool loop)=0;
    virtual void AlarmEnable()=0;
    virtual void AlarmDisable()=0;  
};

/// @brief serial component service
class ISerial : public IHardwareComponent{
 public:
    virtual int availableForReading()=0;
    virtual int availableForWriting()=0;
    virtual int read(char* buffer, int sizeBuffer)=0;
    virtual int write(char* buffer, int sizeBuffer)=0;
    virtual bool isErrorReading()=0;
    virtual bool isErrorWriting()=0;
    virtual void flush()=0;
};

/// @brief digital component service
class IDigital : public IHardwareComponent{
 public:
    virtual int ReadDigital(int gpio)=0;
    virtual void WriteDigital(int gpio, int value)=0;     

};

/// @brief analog component service
class IAnalog : public IHardwareComponent{
    

};

/// @brief ROM component service
class IRom : public IHardwareComponent{
  public:
    virtual void SaveByteToROM(char address,char data)=0;
    virtual void SaveToROM(int index,int value)=0;
    virtual char ReadByteFromROM(char address)=0;
    virtual int ReadFromROM(int index)=0;
};

class IFile : public IHardwareComponent{
  public:
    virtual void Write(const std::string& s) =0;    
    virtual void Write(const char* SendBuffer,const int size) =0;
    virtual std::string Read() =0;
    virtual int Read(char * Receptionbuffer, int sizeBuffer) =0;
    virtual bool OpenFile(const std::string& pathname, bool isReadOnly = true) = 0;
    virtual void CloseFile() = 0;
    virtual int GetFileSize() = 0;
    //virtual ~IFile()=0;
};

/// @brief RFID component service
class IReemasRfid : public IHardwareComponent
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
/// @brief abstract class define the hardware components needed for business
class IHardwareInfrastructure {
  public :
    ISerial *serial;
    IRom *rom;
    IDigital *digital;
    ITimer *timer;
    IFile *file;
    IReemasRfid *rfid;

    IHardwareInfrastructure(ISerial& s, IRom& r, IDigital& d, ITimer& t, IFile& f,IReemasRfid&  rf)
        : serial{&s}, rom{&r}, digital{&d}, timer{&t}, file{&f}, rfid{&rf} {}
    virtual ~IHardwareInfrastructure() = default;
};

struct DebouncedInput{
    bool state;
    bool debounces[MAX_DEBOUNCE];
    char debouceIndex;
    int gpioNumber;

    DebouncedInput(int wgpioNumber){
        
        for(int i=0; i < MAX_DEBOUNCE; i++)
        {
            debounces[i] = 0;
        }
        state = 0;   
        gpioNumber = wgpioNumber;
        debouceIndex = 0;
    }
};


struct Output{
    
    int gpioNumber;
    bool state;
    
    Output(int wgpio,bool wstate):
    gpioNumber{wgpio},state{wstate}
    {}
};

#endif