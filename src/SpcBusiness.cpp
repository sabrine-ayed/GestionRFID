#include "SpcBusiness.h"
#include <cmath>
#include<EEPROM.h>
#include <../lib/Infrastructure/RfidCommunication/ReemasRfid.hpp>



void SpcBusiness::Initialize(ReemasSpcConfig& config){
    if (!LittleFS.begin()) {
       Serial.println("error to initialize littleFS ");
    }
    this->config = config;
    
    //db.begin(); // Assuming RfidDatabase has an initialize method
    //rfid.begin();
    isInit = YES;
    this->config = config;
    TempoDelayOpenBarrier = config.PwOpenGate;
    TempoDelayCloseBarrier = config.PwCloseGate;
    TempoBeforeGate = config.ToBeforeGate;
    tempoValid = config.ToValid;
    TempoRFIDCheck = config.ToRFIDCheck; 
    TempoOnGate = config.ToOnGate;
    TempoAfterGate = config.ToAfterGate;
    hasIrdSensor = (config.BitsConfig & (int)std::pow(2,CONFIG_HAS_IRD_POSITION)) >> CONFIG_HAS_IRD_POSITION ;
    hasTransistorForFan= (config.BitsConfig & (int)std::pow(2,CONFIG_HAS_TR_FAN_POSITION)) >> CONFIG_HAS_TR_FAN_POSITION ;
    LoadCarCount();
    currentAlarm = AlarmStates::NoAlarm;
    lastAlarm = AlarmStates::NoAlarm;
    beepMode = BeepModes::Silent;
    alarmsState = 0;
    DeactiveStation();
   // rfid.Initialize(bsp, BSP::DATA0_PIN, BSP::DATA1_PIN, BSP::CP_PIN);
    SetBuzzer(BeepModes::Short,3);
    
}

/*
TempoDelayOpenBarrier = pulse width duration for opening (default = 500ms)
TempoDelayCloseBarrier = pulse width duration for closing (default = 500ms)
TempoIdle = time out to go to idle mode (default = 1s)
TempoBeforeGate = time out when car still before gate without any action (default = 30s)
TempoRequest = time out for valid response (default = 200ms)
tempoValid = time out for car still before gate after validation (default = 10s)
TempoOnGate = time out for car still on gate (default = 10s)
TempoAfterGate = time out to close gate after car passing (default = 2s)
*/


