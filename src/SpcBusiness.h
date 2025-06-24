#ifndef SPC_BUSINESS
#define SPC_BUSINESS
#include <Arduino.h>
#include <Infrastructure.h>
#include <ReemasSpcConfig.hpp>
 #include <LittleFS.h>
#include <../lib/Infrastructure/RfidCommunication/ReemasRfid.hpp>
#include<EEPROM.h>
#pragma region Definition

#define MAX_HARD_INPUTS        13
//#define MAX_HARD_INPUTS        10
#define MAX_HARD_OUTPUTS        9
#define MAX_SOFT_INPUTS         2
#define MAX_SOFT_OUTPUTS        1

#define INPUT_B1                0
#define INPUT_B2                1
#define INPUT_PHOTOCELL         2
#define INPUT_DOOR_SENSOR       3
#define INPUT_ACT_PROX          4
#define INPUT_ACT_UHF           5
#define INPUT_BPT               6
#define INPUT_HELP              7
#define INPUT_PWR_FAIL          8
#define INPUT_TEMP              9
//input rfid
#define INPUT_D0                10
#define INPUT_D1                11
#define INPUT_CP                12

//#define INPUT_AUX1            10


#define OUTPUT_LED_BPT           0
#define OUTPUT_BARRIER_OPEN      1
#define OUTPUT_BARRIER_CLOSE     2
#define OUTPUT_TRAFIC_LIGHT      3
#define OUTPUT_DOOR_LOCK         4 
#define OUTPUT_FAN               5
#define OUTPUT_ALARM             6
#define OUTPUT_PARK_STATE        7
#define OUTPUT_BUZZER            8

//#define OUTPUT_EN_RS485_1        1

//#define OPEN_DOOR_DELAY        5 //  5s

//#define PULSE_BARRIER_DELAY   500 // 500 ms 

// Control Logic Input
#define IS_ACTIVATED           1 // HIGH
#define IS_RELEASED            0 // LOW

#define IS_OPENED              0 // LOW
#define IS_CLOSED              1 // HIGH

// Control Logic Output

#define OPEN                   1 // HIGH
#define CLOSE                  0 // LOW
#define END_TEMPO              0
#define ACTIVE                 1
#define DEACTIVE               0
#define YES                    1
#define NO                     0

//#define DELAY_AFTER_DOOR_OPEN            1    //  1s
//#define ALARM_DELAY                     10    // 30s
//#define REQUEST_DELAY                    2    //  2s
//#define ALARM_DELAY_AFTER_DOOR_OPENED   20    // 20s
#define TEMPO_STATE                   100    // 1s

#define BEEP_SHORT_DELAY         5  //  50ms
#define BEEP_SMALL_DELAY        30  // 300ms
#define BEEP_MEDIUM_DELAY       50  // 500ms
#define BEEP_LARGE_DELAY       200  // 2s
#define BEEP_INFINTE          -100
#define BEEP_STOP                0

#define LED_ON_DELAY            70  // 700 ms
#define LED_OFF_DELAY           20  // 200 ms

//#define BEEP_COUNT                6

#define ADDRESS_BEGIN_COUNTER     0
#define COUNTER_SIZE              4                  // int32
#define EEPROM_SIZE               COUNTER_SIZE * 2   // defined two counters

#define COUNTER_INDEX             0

#define CONFIG_HAS_IRD_POSITION        1 // has infrared sensor (bit1)
#define CONFIG_HAS_TR_FAN_POSITION     2 // has transistor output for fan (bit2)

#pragma endregion 

#pragma region ENUM

enum CycleStates{
  Disabled = 1,
  FirstSlot,
  SecondSlot,
};

enum BeepModes {
    NotInitialized = 1,
    Silent ,
    Short,
    Small,
    Medium,
    Large,
    OneShortOneLarge,
    OneSmallOneLarge,
    OneMediumOneLarge,
    OneShortOneMedium,
    OneSmallOneMedium,
};

enum AlarmStates {
  
  NoAlarm =         0,
  ViolationAlarm =  1,
  IntrusionAlarm =  2,
  PowerAlarm =      4,
  GateAlarm =       8
};


enum GateStates {
  Idle = 0,
  CarBeforeGate,
  FreeB1,
  RFIDChecking,
  Valid,
  CarOnGate,
  CarAfterGate,
  Alarm,
 

};

enum SpcFlux {
  Entrance,
  Exit,
};

enum ParkStates{
  Indefined=0,
  Standard_Mode,
  POS_Off_Mode,
  Free_Mode,
  Close_Mode
};

enum SpcTimeout{
  CarOnB1 = 100,
  RFIDCheck = 110,
  MovingToB2 = 120,
  CarOnB2 = 130
};
#pragma endregion

#pragma region CLASS

struct SpcException{
  unsigned char code;
  GateStates state ;
};

class SpcBusiness{
  
 //SpcBusiness(BSP& bsp);
  public:
  
 //RfidCommunication rfid;  // 🔹 instance pour le RFID
   // BSP bsp;
   //uint32_t lastCardId = 0;
   // uint32_t lastReadTime = 0;
   // static constexpr uint32_t READ_INTERVAL = 3000;
     //RfidDatabase _db;
     RfidDatabase& database();

  // en ms, adapte selon ton besoin
    //uint32_t cardId; // Declare without initialization
    bool isValid;    // Declare without initialization
    //RfidDatabase db;
    
    DebouncedInput * inputs[MAX_HARD_INPUTS];
    Output * outputs[MAX_HARD_OUTPUTS];
    IHardwareInfrastructure * hwr;
    GateStates gateState;
    //GateStates gateState;
    SpcException spcException;    
    BeepModes beepMode = BeepModes::NotInitialized;
    CycleStates buzzerState = CycleStates::Disabled;
    CycleStates ledState = CycleStates::Disabled;
    //int _beepTimer;
    //SpcFlux flux = SpcFlux::Entrance;
    SpcFlux flux = SpcFlux::Exit;
    ParkStates parkState;
    ParkStates requestedParkState;
    AlarmStates currentAlarm, lastAlarm;
    ReemasSpcConfig config;
    
    bool isRequestDone=false;
    bool isRequestOk=false;
    bool isTicketRequested =false;
    bool isStationActivationRequested =false;
    bool isRfidRequest =false;
    int inputTimerLoader=0;
    int programTimerLoader=0;
    int beepTimerCounter=0;
    int TempoDelayOpenBarrier;
    int TempoDelayCloseBarrier;
    int TempoIdle;
    int TempoBeforeGate;
    int TempoCheck;
    int TempoRequest;
    int tempoValid;
    int TempoOnGate;
    int TempoAfterGate;
    int TempoAlarm;
    int TempoFreeB1;
    int beepCounter=0;
    int tempoLed = 0;
    int  TempoRFIDCheck;
    bool hasWarned = false;
    bool alarmBuzzer2;
    bool closeGate2;
    bool openGate2;
  
    int carCounter=0;
    int totalCarCounter=0;
    unsigned char alarmsState=0;  // cumulatif alarm
                                  // |b7|b6|b5|b4|b3|b2|b1|b0| 
                                  //               |  |  |  |=> b0 : Violation Alarm
                                  //               |  |  | => b1 : Intrusion Alarm
                                  //               |  |=> b2 : Power Alarm
                                  //               |=> b3 : Gate Alarm

    //Rfidcheking
     bool isRfidActivated = false;
     bool isTestRfidCompleted = false;
     bool isRfidValid = false;

    inline bool isRfidTrue();
    inline bool isRfidRefused();
    void Initialize(ReemasSpcConfig& config);
    inline bool IsInitialized();
    inline void GetInputs();
    inline void SetOutputs();
    inline void ExecuteProgram();
    inline bool IsAnyAlarm();
    inline void AgregateOutputs();
    inline void TraitRfid();
    inline bool IsCheckDelayDone();
    inline bool processRfidCard() ;
    inline void GateManagerStandardMode();
    inline void GateManagerPosOffMode();
    inline void HelpManager();
    inline void FanManager();
    inline void ParkStateManager();
    inline void KioskDoorManager();
    inline bool ViolationAlarm();
    inline bool IntrusionAlarm();
    inline bool PowerAlarm();
    inline void BuzzerManager();
    inline void SetBuzzer(BeepModes mode,int count);
    inline bool GateAlarm();
    inline void ArmDelay(GateStates state);
    inline bool IsDelayDone(GateStates state);
    inline void ActiveStation();
    inline void DeactiveStation();
    inline void TraitRequest();
   
    inline bool IsValidRequest();
    inline bool IsNotValidRequest();
    inline void OpenBarrier();
    inline void CloseBarrier();
    inline void MonitorBarrier();
    inline void UpdateCarCount();
    inline void LoadCarCount();
    inline void ResetCarCount();
    inline void StopAlarm();
    inline void LedBlinkingManager();
    
  private:
    bool alarmBuzzer1;
   //  bool alarmBuzzer2;
      bool alarmBuzzer;
    bool closeGate1;
    bool openGate1;
    bool isInit = NO;
    int lowPeriodBuzzer;
    int highPeriodBuzzer;
    //int lowPeriodLed;
    //int highPeriodLed;
    bool isBlinking;
    bool hasIrdSensor= true;
    bool hasTransistorForFan= false;
    bool canClose = true;
 };

#pragma endregion

#pragma region INLINE METHOD
/*inline bool SpcBusiness::isRfidTrue() {
  return isRfidRequest= (inputs[INPUT_ACT_PROX]->state == IS_ACTIVATED || inputs[INPUT_ACT_UHF]->state == IS_ACTIVATED) &&
         inputs[INPUT_BPT]->state == IS_ACTIVATED;
}
inline bool SpcBusiness::isRfidRefused() {
  return isRfidRequest= (inputs[INPUT_ACT_PROX]->state == IS_ACTIVATED || inputs[INPUT_ACT_UHF]->state == IS_ACTIVATED) &&
         inputs[INPUT_BPT]->state == IS_RELEASED;
} */

inline void SpcBusiness::SetOutputs(){

  for (int i=0 ; i< MAX_HARD_OUTPUTS; i++){
    hwr->digital->WriteDigital(outputs[i]->gpioNumber, outputs[i]->state);
  }

}

inline void SpcBusiness::GetInputs(){

  for (int i=0 ; i< MAX_HARD_INPUTS; i++){

    if(inputs[i]->debouceIndex >= MAX_DEBOUNCE ){
      inputs[i]->debouceIndex = 0;
    }
    inputs[i]->debounces[inputs[i]->debouceIndex] = hwr->digital->ReadDigital(inputs[i]->gpioNumber);
    inputs[i]->debouceIndex++;  
    bool wstate = inputs[i]->debounces[0];
    bool stable = true;
    for (int j=1 ; j< MAX_DEBOUNCE; j++){
      if(wstate != inputs[i]->debounces[j])  
      {
        stable = false;
        break;
      }           
      wstate = inputs[i]->debounces[j];
    }
    if (stable)
      inputs[i]->state = inputs[i]->debounces[0];
  }
}

inline bool SpcBusiness::IsAnyAlarm() {
  PowerAlarm(); // non bloquant
  IntrusionAlarm(); // non bloquant
  ViolationAlarm(); // non bloquant
  int gAlarm = GateAlarm(); // non bloquant
  if (alarmsState == 0) {
    StopAlarm();
    return false;
  } else {
    if (!gAlarm) {
      switch (currentAlarm) {
        case AlarmStates::ViolationAlarm:
          if (currentAlarm != lastAlarm) SetBuzzer(BeepModes::Large, 20);
          break;
        case AlarmStates::IntrusionAlarm:
          if (currentAlarm != lastAlarm) SetBuzzer(BeepModes::Small, 20);
          break;
        case AlarmStates::PowerAlarm:
          if (currentAlarm != lastAlarm) SetBuzzer(BeepModes::OneMediumOneLarge, BEEP_INFINTE);
          break;
        default:
          break;
      }
    }
    lastAlarm = currentAlarm;
  }
  return gAlarm;
}

inline bool SpcBusiness::ViolationAlarm() {
  bool result = false;
  if (parkState == ParkStates::Standard_Mode &&
      inputs[INPUT_B2]->state == IS_ACTIVATED &&
      !isRfidRequest &&
      (gateState == GateStates::Idle || gateState == GateStates::CarBeforeGate || gateState == GateStates::FreeB1 || gateState == GateStates::CarOnGate)) {
    currentAlarm = AlarmStates::ViolationAlarm;
    alarmsState |= AlarmStates::ViolationAlarm;
    result = true;
  } else if (parkState == ParkStates::POS_Off_Mode &&
             gateState == GateStates::Idle &&
             inputs[INPUT_B2]->state == IS_ACTIVATED) {
    currentAlarm = AlarmStates::ViolationAlarm;
    alarmsState |= AlarmStates::ViolationAlarm;
    result = true;
  } else {
    alarmsState &= ~AlarmStates::ViolationAlarm;
    result = false;
  }
  return result;
}

inline bool SpcBusiness::IntrusionAlarm(){
  if(inputs[INPUT_DOOR_SENSOR]->state == IS_ACTIVATED && !isRfidRequest)// revoir la condition
  {
    currentAlarm = AlarmStates::IntrusionAlarm;
    alarmsState |= AlarmStates::IntrusionAlarm;
    //beepMode = BeepModes::TwoLarge;
    return true;
  }
  alarmsState &= ~AlarmStates::IntrusionAlarm;  
  return false;
}

inline bool SpcBusiness::PowerAlarm(){
  if(inputs[INPUT_PWR_FAIL]->state == IS_ACTIVATED){
    currentAlarm = AlarmStates::PowerAlarm;
    alarmsState |= AlarmStates::PowerAlarm;
    //beepMode = BeepModes::OneMediumOneLarge;
    return true;
  }
  alarmsState &= ~AlarmStates::PowerAlarm;  
  return false;
}


inline bool SpcBusiness::GateAlarm() {
  if(gateState== GateStates::Alarm)
  {
    currentAlarm = AlarmStates::GateAlarm;
    alarmsState |= AlarmStates::GateAlarm;


    switch (spcException.state) {

      case GateStates::CarBeforeGate:

        // car reversing (stop alarm)
        if (inputs[INPUT_B1]->state == IS_RELEASED) {
          gateState= GateStates::Idle;
          return false;
        }
        //// Timeout keeps alarm active
        if (IsDelayDone(GateStates::CarBeforeGate)) {
          return true; 
        }
        //En POS_Off_Mode
        if(parkState == ParkStates::POS_Off_Mode && 
         inputs[INPUT_B2]->state == IS_ACTIVATED )
          {
             gateState= GateStates::CarOnGate; 
             return false;
          }
        break;

      case GateStates::RFIDChecking:
        if (inputs[INPUT_B1]->state == IS_RELEASED) {
          return false;
        }
        if (inputs[INPUT_PHOTOCELL]->state == IS_RELEASED) {
               gateState = GateStates::CarBeforeGate; 
                return false; 
            }

         
         /*if ((inputs[INPUT_B1]->state == IS_ACTIVATED) && (inputs[INPUT_PHOTOCELL]->state = IS_ACTIVATED )  && (db.isTagValid(cardId))) {

          gateState = GateStates::CarOnGate; 
          return false; // Clear alarm state, system doesn't block
         }
            */
       
        if ((inputs[INPUT_B1]->state == IS_ACTIVATED) && (inputs[INPUT_PHOTOCELL]->state = IS_ACTIVATED)/* &&  (!(db.isTagValid(cardId)))*/) {
          SetBuzzer(BeepModes::Short, 3); // Trigger short buzzer
          gateState = GateStates::CarBeforeGate; // Return to CarBeforeGate
          return false; // Clear alarm state, system doesn't block
          

          if(inputs[INPUT_B1]->state == IS_RELEASED){
        gateState= GateStates::FreeB1; 
        return false;
      }

          break;
        }
        
case GateStates::Valid:
      if(inputs[INPUT_B2]->state == IS_ACTIVATED){
        gateState= GateStates::Valid; 
        return false;
      }
      if(inputs[INPUT_B1]->state == IS_RELEASED){
        gateState= GateStates::FreeB1; 
        return false;
      }
      //Signalisation Buzzer interrompue (300msON – 300ms OFF) 
      //Durée signalisation 10s
    if(currentAlarm != lastAlarm)
        SetBuzzer(BeepModes::Medium,10);
      return true;
      break;

      case GateStates::CarOnGate:
       if(inputs[INPUT_B2]->state == IS_RELEASED){
        gateState= GateStates::CarOnGate;
        return false;
      }
      
      if(inputs[INPUT_B2]->state == IS_ACTIVATED){
        gateState= GateStates::CarOnGate; 
        return false;
      }
      if (IsDelayDone(GateStates::CarOnGate)) {
          return true; 
        }
      //Signalisation Buzzer interrompue (300msON – 300ms OFF) 
      //Durée signalisation 10s
      if(currentAlarm != lastAlarm)
        SetBuzzer(BeepModes::Medium,10);
      return true;
      break;
    default:
      break;
    }
    return true;
  }
  alarmsState &= ~AlarmStates::GateAlarm;  
  return false;
}


inline void SpcBusiness::StopAlarm(){
  currentAlarm = AlarmStates::NoAlarm;
  lastAlarm = currentAlarm;
  SetBuzzer(BeepModes::Silent,0);
  //desactiv Alarm
  /*currentAlarm = AlarmStates::NoAlarm;
  lastAlarm = AlarmStates::NoAlarm;
  alarmsState = 0;
  SetBuzzer(BeepModes::Silent, 0);
  alarmBuzzer1 = DEACTIVE;
  alarmBuzzer2 = DEACTIVE;
  outputs[OUTPUT_BUZZER]->state = DEACTIVE;*/
  
}

inline void SpcBusiness::ActiveStation(){
  isTicketRequested = false;
  isRequestDone = false;
  isRequestOk =false;
  isStationActivationRequested = true;
  isBlinking =true;
}

inline void SpcBusiness::DeactiveStation(){
  isTicketRequested = false;
  isRequestDone = false;
  isRequestOk =false;
  isStationActivationRequested = false;
  isBlinking = false;
  isRfidRequest =false;
}



inline bool SpcBusiness::IsValidRequest(){
  // is ok returned from host
  return (isRequestDone && isRequestOk) || isRfidRequest ;
}

inline bool SpcBusiness::IsNotValidRequest(){
  // is ok returned from host
  return isRequestDone && !isRequestOk ;
}

inline void SpcBusiness::OpenBarrier(){
  openGate1 = OPEN;
  closeGate1 = CLOSE;
  TempoDelayOpenBarrier= config.PwOpenGate;
}

inline void SpcBusiness::CloseBarrier(){
  if(!hasIrdSensor ||
     (hasIrdSensor && inputs[INPUT_PHOTOCELL]->state))
    {
      canClose = true;
      openGate1 = CLOSE;
      closeGate1 = OPEN;
      TempoDelayCloseBarrier= config.PwCloseGate;
    }
  else
    canClose = false;
}

inline void SpcBusiness::MonitorBarrier(){
  if(!canClose)
    CloseBarrier();
  if((parkState == ParkStates::Standard_Mode || parkState == ParkStates::POS_Off_Mode) 
      && TempoDelayOpenBarrier == 0 
      && TempoDelayCloseBarrier == 0
    )
    {
      openGate1 = CLOSE;
      closeGate1 = CLOSE;
      openGate2 = CLOSE;
      closeGate2 = CLOSE;
    }
}

inline void SpcBusiness::LoadCarCount(){
  totalCarCounter= hwr->rom->ReadFromROM(COUNTER_INDEX);
}

inline void SpcBusiness::UpdateCarCount(){
  carCounter++;
  totalCarCounter++;
  hwr->rom->SaveToROM(COUNTER_INDEX,totalCarCounter);
  /*switch (flux)
  {
  case SpcFlux::Entrance:
    carCounter++;
    totalCarCounter++;
    hwr->rom->SaveToROM(COUNTER_INDEX,totalCarCounter);
    break;
  case SpcFlux::Exit:
    carCounter--;
    hwr->rom->SaveToROM(COUNTER_INDEX,carCounter);
    break;
  default:
    break;
  }*/
}


inline void SpcBusiness::ResetCarCount() {
 
  carCounter = 0;
  totalCarCounter = 0;
  hwr->rom->SaveToROM(COUNTER_INDEX, totalCarCounter);
  
}
inline void SpcBusiness::ArmDelay(GateStates state) {
  switch (state) {
    case GateStates::Idle:
      TempoIdle = TEMPO_STATE;
      break;
    case GateStates::CarBeforeGate:
      TempoBeforeGate = config.ToBeforeGate;
      break;
    case GateStates::FreeB1:
      TempoFreeB1 = TEMPO_STATE;
      break;
    case GateStates::RFIDChecking:
      TempoRFIDCheck = config.ToRFIDCheck; 
      break;
      case GateStates::Valid:
      tempoValid = config.ToValid; 
      break;
    case GateStates::CarOnGate:
      TempoOnGate = config.ToOnGate;
      break;
    case GateStates::CarAfterGate:
      TempoAfterGate = config.ToAfterGate;
      break;
    case GateStates::Alarm:
      // Pas de temporisation pour Alarm
      break;
  
      break;
    default:
      break;
  }
}




inline bool SpcBusiness::IsCheckDelayDone() {
  return TempoCheck == END_TEMPO;
}


inline bool SpcBusiness::IsDelayDone(GateStates state){
  switch (state)
  {
  case GateStates::Idle :
    if(TempoIdle == END_TEMPO)
      return true;
    else
      return false;
    break;
  case GateStates::CarBeforeGate :
    if(TempoBeforeGate == END_TEMPO)
      return true;
    else
      return false;
    break;
  case GateStates::FreeB1 :
    if(TempoFreeB1 == END_TEMPO)
      return true;
    else
      return false;
    break;
  case GateStates::RFIDChecking:
    if(TempoCheck == END_TEMPO)
      return true;
    else
      return false;
      break;
      case GateStates::Valid:
    if(tempoValid == END_TEMPO)
      return true;
    else
      return false;
      break;
     
  case GateStates::CarOnGate :
    if(TempoOnGate == END_TEMPO)
      return true;
    else
      return false;
    break;
  case GateStates::CarAfterGate :
    if(TempoAfterGate == END_TEMPO)
      return true;
    else
      return false;
    break;
  case GateStates::Alarm : 
    //if(TempoAlarm == END_TEMPO)
    //  return true;
    //else
    //  return false;
    break;
  default:
    break;
  }
  return false;
}




bool SpcBusiness::processRfidCard() {
   /* rfid.handleEvent(RfidCommunication::Event::TIMEOUT);
    if (rfid.isCardDetected() && rfid.isValid()) {
        uint32_t cardId = rfid.getCardId();
        uint32_t now = millis();
        if (bsp.database().isTagValid(cardId)) {
           // bsp.printf("✅ Tag valide reçu : %u (0x%08X)\n", cardId, cardId);
            return true;
        } else {
           // bsp.printf("❌ Tag invalide : %u (0x%08X)\n", cardId, cardId);
        }
    }*/
    return false;
}


inline void SpcBusiness::GateManagerStandardMode() {
  switch (gateState) {
    case GateStates::Idle:
      if (inputs[INPUT_B1]->state == IS_ACTIVATED) {
        ActiveStation();
        ArmDelay(GateStates::CarBeforeGate);
        gateState = GateStates::CarBeforeGate;
      }
      break;

    case GateStates::CarBeforeGate:
      if (inputs[INPUT_B1]->state == IS_RELEASED) {
        ArmDelay(GateStates::Idle);
        gateState = GateStates::FreeB1;
        break;
      }
      if (inputs[INPUT_B1]->state == IS_ACTIVATED 
         ) {
          bool isRfidActivated = true;
        ArmDelay(GateStates::RFIDChecking);
        outputs[OUTPUT_TRAFIC_LIGHT]->state = CLOSE;
        gateState = GateStates::RFIDChecking;
        break;
      }
  
      if (IsDelayDone(GateStates::CarBeforeGate)) {
        spcException = SpcException{SpcTimeout::CarOnB1, gateState};
        gateState = GateStates::Alarm;
        break;
      }
      break;

    case GateStates::FreeB1:
      if (IsDelayDone(GateStates::Idle)) {
        DeactiveStation();
        outputs[OUTPUT_TRAFIC_LIGHT]->state = CLOSE;
        gateState = GateStates::Idle;
      }
      break;
case GateStates::RFIDChecking:
      if (processRfidCard()) {
      
          OpenBarrier();
          ArmDelay(GateStates::Valid);
          gateState = GateStates::Valid;
         break;
        } 
        
       if (!processRfidCard()) {
          SetBuzzer(BeepModes::Short, 2);
          spcException = SpcException{SpcTimeout::RFIDCheck, gateState};
          gateState = GateStates::Alarm;
           break;
        }
       
      
      break;
   

case GateStates::Valid :
    if(inputs[INPUT_B2]->state== IS_ACTIVATED){
      ArmDelay(GateStates::CarOnGate);
      gateState = GateStates::CarOnGate;
      break;
    }
    if(IsDelayDone(GateStates::Valid)){
      spcException= SpcException{SpcTimeout::MovingToB2,gateState};
      gateState= GateStates::Alarm;
      break;
    }
    break;

    case GateStates::CarOnGate:
      if ( (inputs[INPUT_PHOTOCELL]->state == IS_RELEASED) 
      && (inputs[INPUT_B2]->state == IS_ACTIVATED) 
      && (inputs[INPUT_B1]->state == IS_RELEASED)) 
      {
        ArmDelay(GateStates::CarAfterGate);
        gateState = GateStates::CarAfterGate;
        break;
      }
      if (IsDelayDone(GateStates::CarOnGate)) {
        spcException = SpcException{SpcTimeout::CarOnB2, gateState};
        gateState = GateStates::Alarm;
        break;
      }
      break;

    case GateStates::CarAfterGate:
      if (IsDelayDone(GateStates::CarAfterGate)) {
        CloseBarrier();
        UpdateCarCount();
        DeactiveStation();
        outputs[OUTPUT_TRAFIC_LIGHT]->state = CLOSE;
        gateState = GateStates::Idle;
        break;
      }
    case GateStates::Alarm:
     // this state is managed by Alarm manager
      break;

    default:
      break;
  }
}





inline void SpcBusiness::GateManagerPosOffMode(){
  switch (gateState)
  {
  case GateStates::Idle :
    if(inputs[INPUT_B1]->state == IS_ACTIVATED)
    {
      OpenBarrier();
      ArmDelay(GateStates::CarBeforeGate);
      outputs[OUTPUT_TRAFIC_LIGHT]->state = OPEN;
      gateState = GateStates::CarBeforeGate;
    }
    break;
  case GateStates::CarBeforeGate :
    if(inputs[INPUT_B2]->state == IS_ACTIVATED)
    {
      ArmDelay(GateStates::CarOnGate);  
      gateState = GateStates::CarOnGate;
      break;
    }
    if(inputs[INPUT_B1]->state == IS_RELEASED)
    {
      ArmDelay(GateStates::Idle);
      outputs[OUTPUT_TRAFIC_LIGHT]->state = CLOSE;
      gateState= GateStates::FreeB1;
      break;
    }
    if(IsDelayDone(GateStates::CarBeforeGate))
    {
      spcException ={SpcTimeout::CarOnB1,gateState};
      gateState = GateStates::Alarm;
      break;
    }
    break;
  case GateStates::CarOnGate :
    if(inputs[INPUT_B2]->state == IS_RELEASED)
    {
      ArmDelay(GateStates::CarAfterGate);
      gateState = GateStates::CarAfterGate;
      break;
    }
    if(IsDelayDone(GateStates::CarOnGate))
    {
      spcException ={SpcTimeout::CarOnB2,gateState};
      gateState = GateStates::Alarm;
      break;
    }
    break;
  case GateStates::CarAfterGate :
    if(IsDelayDone(GateStates::CarAfterGate))
    {
      CloseBarrier();
      outputs[OUTPUT_TRAFIC_LIGHT]->state = CLOSE;
      UpdateCarCount();
      gateState = GateStates::Idle;
    }
    break;
  case GateStates::FreeB1 :
    if(IsDelayDone(GateStates::Idle)){
      DeactiveStation();
      CloseBarrier();
      gateState = GateStates::Idle;
    }
    break;
  case GateStates::Alarm :
    // this state is managed by Alarm manager
    break;      
  default:
    break;
  }
}





inline void SpcBusiness::HelpManager(){
  // nothing to do here, can be managed
  // directly by the host on looking to
  // the appropriate input for help
}

inline void SpcBusiness::FanManager(){
  if(inputs[INPUT_TEMP]->state){
    if(!hasTransistorForFan)
    {
      outputs[OUTPUT_FAN]->state = ACTIVE;
    }
    else{

    }
  }
  else{
    if(!hasTransistorForFan)
    {
      outputs[OUTPUT_FAN]->state = DEACTIVE;
    }
    else{
      
    }
  }
    
}




inline void SpcBusiness::ParkStateManager(){
  if(parkState != requestedParkState)
    switch (requestedParkState)
    {
    case ParkStates::Standard_Mode :
      parkState = ParkStates::Standard_Mode;
      gateState= GateStates::Idle;
      outputs[OUTPUT_TRAFIC_LIGHT]->state = CLOSE;
      break;
    case ParkStates::POS_Off_Mode :
      parkState = ParkStates::POS_Off_Mode;
      gateState= GateStates::Idle;
      outputs[OUTPUT_TRAFIC_LIGHT]->state = CLOSE;
      break;
    case ParkStates::Free_Mode :
      parkState = ParkStates::Free_Mode;
      gateState= GateStates::Idle;
      outputs[OUTPUT_TRAFIC_LIGHT]->state = OPEN;
      break;
    case ParkStates::Close_Mode :
      parkState = ParkStates::Close_Mode;
      gateState= GateStates::Idle;
      outputs[OUTPUT_TRAFIC_LIGHT]->state = CLOSE;
      break;
    default:
      break;
    }
  
  switch(parkState){
    case ParkStates::Indefined:
    outputs[OUTPUT_PARK_STATE]->state = CLOSE;
    outputs[OUTPUT_TRAFIC_LIGHT]->state = CLOSE;
    closeGate1 = OPEN;
    openGate1 = CLOSE;
    break;
    case ParkStates::Standard_Mode:
      outputs[OUTPUT_PARK_STATE]->state = OPEN;
      GateManagerStandardMode();
    break;
    case ParkStates::Close_Mode:
      outputs[OUTPUT_PARK_STATE]->state = CLOSE;
      outputs[OUTPUT_TRAFIC_LIGHT]->state = CLOSE;
      closeGate1 = OPEN;
      openGate1 = CLOSE;
    break;
    case ParkStates::POS_Off_Mode :
      outputs[OUTPUT_PARK_STATE]->state = OPEN;
      GateManagerPosOffMode();
    break;
    case ParkStates::Free_Mode:
      outputs[OUTPUT_PARK_STATE]->state = OPEN;
      outputs[OUTPUT_TRAFIC_LIGHT]->state = OPEN;
      openGate1 = OPEN;
      closeGate1 = CLOSE;
    break;
    default:break;
  }
}

inline void SpcBusiness::KioskDoorManager(){

}

inline bool SpcBusiness::IsInitialized(){
  if(isInit) 
    return YES;
  else
  { // logic for end initialization (!!! this is a bad condition !!!)
    if(beepCounter == 0){
      isInit = YES;
      return YES;
    }
    else return NO;
  }
} 





inline void SpcBusiness::ExecuteProgram() {
  if (IsInitialized()) {
    BuzzerManager(); // Gestion du buzzer
    LedBlinkingManager(); // Si applicable
    if (!IsAnyAlarm()) {
      KioskDoorManager();
      ParkStateManager();
    }
    HelpManager();
    MonitorBarrier();
   
  }
}





inline void SpcBusiness::LedBlinkingManager(){
  switch (ledState)
  {
  case CycleStates::Disabled :
    outputs[OUTPUT_LED_BPT]->state = CLOSE;
    if(isBlinking){
      tempoLed = LED_ON_DELAY;
      ledState = CycleStates::FirstSlot;
      outputs[OUTPUT_LED_BPT]->state = OPEN;
    }
    break;
  case CycleStates::FirstSlot :
    outputs[OUTPUT_LED_BPT]->state = OPEN;
    if(tempoLed == 0){
      tempoLed = LED_OFF_DELAY; 
      ledState = CycleStates::SecondSlot;
    }
    break;
  case CycleStates::SecondSlot :
    outputs[OUTPUT_LED_BPT]->state = CLOSE;
    if(tempoLed == 0)
      ledState = CycleStates::Disabled;
    break;
  default:
    break;
  }
}

inline void SpcBusiness::BuzzerManager() {
  if (TempoAlarm > 0) {
    TempoAlarm--;
  }
  switch (buzzerState) {
    case CycleStates::Disabled:
      alarmBuzzer1 = DEACTIVE;
      if (beepMode != BeepModes::Silent && beepCounter != 0) {
        TempoAlarm = highPeriodBuzzer;
        buzzerState = CycleStates::FirstSlot;
        alarmBuzzer1 = ACTIVE;
      }
      break;
    case CycleStates::FirstSlot:
      alarmBuzzer1 = ACTIVE;
      if (TempoAlarm == 0) {
        TempoAlarm = lowPeriodBuzzer;
        buzzerState = CycleStates::SecondSlot;
      }
      break;
    case CycleStates::SecondSlot:
      alarmBuzzer1 = DEACTIVE;
      if (TempoAlarm == 0) {
        if (beepCounter != BEEP_INFINTE) {
          beepCounter--;
          if (beepCounter == 0) {
            beepMode = BeepModes::Silent;
          }
        }
        buzzerState = CycleStates::Disabled;
      }
      break;
  }
}




inline void SpcBusiness::SetBuzzer(BeepModes mode, int count) {
  if (mode == beepMode) return; 
  beepMode = mode;
  switch (mode) {
    case BeepModes::Silent:
      beepCounter = BEEP_STOP;
      highPeriodBuzzer = BEEP_STOP;
      lowPeriodBuzzer = BEEP_STOP;
      buzzerState = CycleStates::Disabled;
      alarmBuzzer1 = DEACTIVE; 
      break;
    case BeepModes::Short:
      beepCounter =  count ; 
      highPeriodBuzzer = 20;    
      lowPeriodBuzzer = 7;     
      buzzerState = CycleStates::Disabled;
      break;
    case BeepModes::Small:
      beepCounter = count;
      highPeriodBuzzer = BEEP_SMALL_DELAY;
      lowPeriodBuzzer = BEEP_SMALL_DELAY;
      buzzerState = CycleStates::Disabled;
      break;
    case BeepModes::Medium:
      beepCounter = count;
      highPeriodBuzzer = BEEP_MEDIUM_DELAY;
      lowPeriodBuzzer = BEEP_MEDIUM_DELAY;
      buzzerState = CycleStates::Disabled;
      break;
    case BeepModes::Large:
      beepCounter = count;
      highPeriodBuzzer = BEEP_LARGE_DELAY;
      lowPeriodBuzzer = BEEP_LARGE_DELAY;
      buzzerState = CycleStates::Disabled;
      break;
    case BeepModes::OneShortOneLarge:
      beepCounter = count;
      highPeriodBuzzer = BEEP_SHORT_DELAY;
      lowPeriodBuzzer = BEEP_LARGE_DELAY;
      buzzerState = CycleStates::Disabled;
      break;
    case BeepModes::OneSmallOneLarge:
      beepCounter = count;
      highPeriodBuzzer = BEEP_SMALL_DELAY;
      lowPeriodBuzzer = BEEP_LARGE_DELAY;
      buzzerState = CycleStates::Disabled;
      break;
    case BeepModes::OneMediumOneLarge:
      beepCounter = count;
      highPeriodBuzzer = BEEP_MEDIUM_DELAY;
      lowPeriodBuzzer = BEEP_LARGE_DELAY;
      buzzerState = CycleStates::Disabled;
      break;
    case BeepModes::OneShortOneMedium:
      beepCounter = count;
      highPeriodBuzzer = BEEP_SHORT_DELAY;
      lowPeriodBuzzer = BEEP_MEDIUM_DELAY;
      buzzerState = CycleStates::Disabled;
      break;
    case BeepModes::OneSmallOneMedium:
      beepCounter = count;
      highPeriodBuzzer = BEEP_SMALL_DELAY;
      lowPeriodBuzzer = BEEP_MEDIUM_DELAY;
      buzzerState = CycleStates::Disabled;
      break;
    default:
      beepMode = BeepModes::Silent; 
      break;
  }
}


inline void SpcBusiness::AgregateOutputs(){
  outputs[OUTPUT_BARRIER_OPEN]->state = openGate1 | openGate2;
  outputs[OUTPUT_BARRIER_CLOSE]->state = closeGate1 | closeGate2;
  outputs[OUTPUT_BUZZER]->state = /*alarmBuzzer1 |*/ alarmBuzzer2;
}

#pragma endregion

#endif