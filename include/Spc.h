#ifndef SPC
#define SPC


#define BSP_SPC

#ifdef BSP_SPC
    #include "BspSPC.h"
#else
    #include "BspCarteEssaie.h"
#endif

// Digital Outputs
#define LED_BPT                 HW_STATE 
#define EN_RS485_1              EN_485_1
#define BARRIER_OPEN            OUT_1
#define BARRIER_CLOSE           OUT_2
#define TRAFIC_LIGHT            OUT_3
#define DOOR_LOCK               OUT_4
#define FAN                     OUT_5
#define ALARM                   OUT_6
#define PARK_STATE              OUT_7
#define BUZZER                  BUZ_Alarm//LED_BPT for debug purpose, release => BUZ_Alarm

// Digital Inputs

// input rfid
#define DATA0_PIN               IN_12
#define DATA1_PIN               IN_13
#define CP_PIN                  IN_14

#define LOOP_B1                 IN_1
#define LOOP_B2                 IN_2
#define PHOTOCELL               IN_3
#define DOOR_SENSOR             IN_4
//#define ACT_PROX                IN_5
//#define ACT_UHF                 IN_6
//#define BPT                     IN_7
#define HELP                    IN_8
#define PWR_FAIL                IN_9
#define TEMP                    IN_10
#define AUX1_IN               IN_11

#define MAX_CYCLIC_BYTES         8     // 2 inputs + 2 outputs + 1 variable for requests + 3 bytes for alarm and errors
//#define MAX_SERIAL_OUTPUT      2
//#define LOCAL_SLAVE_ADDRESS      2
// Constant

#define TWENTY_SECOND                       20000000
#define ONE_SECOND                           1000000
#define HALF_SECOND                           500000
#define ONE_Milli_SECOND                        1000
#define TEN_Milli_SECOND                       10000


#define MAX_HEART_BEAT_TIMER_LOAD          7
#define MAX_INPUT_TIMER_COUNT              3     // 3 ms
#define MAX_PROGRAM_TIMER_COUNT           50    // 50 ms

#define CONFIG_HAS_WIFI_POSITION           0


//#define business SpcSerialReemasExecuter::business
// Acyclic commands
enum MasterCommands
{
  GetParameters       =  7,
  SetParameters       =  8,
  GetCounters         =  9,
  SetCounter          = 10,
  ResetCounter        = 11,
  FactoryReset        = 12,
  BlinkLed            = 13,
  LedOn               = 14,
  LedOff              = 15,
  OpenGate            = 16,
  CloseGate           = 17,
  GetParkingMode      = 18,
  SetParkingMode      = 19,
  Reboot              = 20,
  GetTicketRequest    = 21,
  SetTicketValidation = 22,
  GetStationActivation= 23
};


#endif