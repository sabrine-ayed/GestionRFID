#ifndef _REEMAS_DOMAIN_SPC_CONFIG
#define _REEMAS_DOMAIN_SPC_CONFIG

#include <sstream>

struct ReemasSpcConfig{
    // all times are 10 based unit, in ms. for example if value is 50 then the real time is 500 ms
    std::string BaudRateCh1= "9600";
    std::string BaudRateCh2= "9600";
    short PwOpenGate = 200;     // pulse width for open gate command (ms)
    short PwCloseGate = 50;    // pulse width for close gate command (ms)
    short ToViolation = 1000;  // time out for violation detection
    short ToIntrusion = 1000;  // time out for intrusion detection
    
    short ToCheck = 1000;
    short ToBeforeGate = 3000; // time out when car still before gate without any action (default = 30s)
    short ToRFIDCheck = 3000;    // time out for valid response (default = 200ms)
    short ToRfidRequest = 500;      // time out for valid response (default = 200ms)
    short ToValid = 3000;      // time out for car still before gate after validation -blocked car- (default = 10s)
    short ToOnGate = 1000;     // time out for car still on gate -blocked car-(default = 10s)
    short ToAfterGate = 300;   // time out to close gate after car passing (default = 2s)
    short Temperature = 40;     // threshold of temperature (°C)
    std::string SlaveIP="";
    short SlavePort =0; 
    std::string BrokerIP="";
    short BrokerPort=0; 
    short SlaveAddress = 2; 
    short PwBlinking= 50;      // pulse width for blinking led (ms)
    std::string Wifi_SSID=  "";
    std::string Wifi_pwd = "xQ9Zks6N";
    //bool IsWifiActive = false;
    unsigned int BitsConfig = 0; // bit0 => IsWifiActive; bit1 => HasIR
};

#endif