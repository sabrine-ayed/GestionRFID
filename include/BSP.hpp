#ifndef BSP_HPP
#define BSP_HPP

#include <Arduino.h>

class BSP
{
public:
  static const int DATA0_PIN = 22; // Corresponds to GPIO22
  static const int DATA1_PIN = 23; // Corresponds to GPIO23
  static const int CP_PIN = 21;    // Corresponds to GPIO21

  void initSerial(unsigned long baudRate);
  void println(const char *message);
  void print(const char *message);
  void print(uint32_t value, int base = DEC);
  void printf(const char *format, ...);

  void pinMode(uint8_t pin, uint8_t mode);
  void attachInterrupt(uint8_t pin, void (*ISR)(), int mode);

  unsigned long millis();
};

#endif // BSP_HPP