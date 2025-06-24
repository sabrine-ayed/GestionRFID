#ifndef hardware
#define hardware

#include <Infrastructure.h>
#include <Arduino.h>
#include <EEPROM.h>
#include <LittleFS.h>



#pragma region ReemasSerial
class RfidDatabase;
class ReemasSerial: public ISerial{
    private :
        HardwareSerial *hSerial;
    public:
    ReemasSerial(HardwareSerial& s);
    ReemasSerial(HardwareSerial& s, size_t maxBufferRx);
    virtual int availableForReading();
    virtual int availableForWriting();
    virtual int read(char* buffer, int sizeBuffer);
    virtual int write(char* buffer, int sizeBuffer);
    virtual bool isErrorReading();
    virtual bool isErrorWriting();
    virtual void flush();
        
    static const int SEND_BUFFER_SIZE = 100;
    static const int RECEIVE_BUFFER_SIZE = 100;
    char sendBuffer[SEND_BUFFER_SIZE];
    char receiveBuffer[RECEIVE_BUFFER_SIZE];
    int sendIndex=0;
    int receiveIndex=0;
};

#pragma endregion

#pragma region ReemasTimer

class ReemasTimer:public ITimer{
  public:
    virtual void AttachInterrupt(void (*fn)(void), bool edge);
    virtual void AlarmWrite(int count, bool loop);
    virtual void AlarmEnable();
    virtual void AlarmDisable();
    ReemasTimer(hw_timer_t& ht);
    hw_timer_t* GetArduinoTimer();
  private :
    hw_timer_t* _timerComponent;
};

#pragma endregion

#pragma region ReemasDigital

class ReemasDigital: public IDigital{
    public:
        int ReadDigital(int gpio);
        void WriteDigital(int gpio, int value);
};

#pragma endregion

#pragma region ReemasRom

class ReemasRom : public IRom{
  public:
    virtual void SaveByteToROM(char address,char data);
    virtual void SaveToROM(int index,int value);
    virtual char ReadByteFromROM(char address);
    virtual int ReadFromROM(int index);
    ReemasRom(int eepromSize,int beginAddress, int valueSize);
    int GetRomSize();
    int GetBeginAddress();
    int GetValueSize();
  private:
    int _eepromSize;
    int _beginAddress;
    int _valueSize;  
};

#pragma endregion

#pragma region ReemasFile

class ReemasFs : public IFile{
    public:
        virtual void Write(const std::string& s);    
        virtual void Write(const char* SendBuffer,const int size) ;
        virtual std::string Read() ;
        virtual int Read(char * Receptionbuffer, int sizeBuffer) ;
        virtual bool OpenFile(const std::string& pathname, bool isReadOnly = true);
        virtual void CloseFile();
        virtual int GetFileSize();
        ReemasFs();
        //ReemasFs(FS& fs);
        ~ReemasFs();
    private :
        //FS _fs;
        File file;
        bool isFsOk = false; 
        int size = 0;
};

#pragma endregion

#pragma region ReemasRfid

class ReemasRfid : public IRfid {
public:

    virtual void pinMode(uint8_t pin, uint8_t mode) override {
        ::pinMode(pin, mode);
    }

    virtual void attachInterrupt(uint8_t pin, void (*ISR)(), int mode) override {
        ::attachInterrupt(digitalPinToInterrupt(pin), ISR, mode);
    }

    virtual unsigned long millis() override {
        return ::millis();
    }

};

#pragma endregion


#pragma region ArduinoHardware

class ArduinoHardware : public IHardwareInfrastructure{
    public:
    ArduinoHardware(ReemasDigital& d, ReemasTimer& t, ReemasSerial& s, ReemasRom& r, ReemasFs& f, ReemasRfid& rf)
        : IHardwareInfrastructure(s, r, d, t, f, rf ) {}
};


#pragma endregion

#endif


