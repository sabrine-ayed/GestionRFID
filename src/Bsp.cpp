#include "BSP.hpp"

void BSP::initSerial(unsigned long baudRate)
{
  Serial.begin(baudRate);
}

void BSP::println(const char *message)
{
  Serial.println(message);
}

void BSP::print(const char *message)
{
  Serial.print(message);
}

void BSP::print(uint32_t value, int base)
{
  Serial.print(value, base);
}

void BSP::printf(const char *format, ...)
{
  char buf[128];
  va_list args;
  va_start(args, format);
  vsnprintf(buf, sizeof(buf), format, args);
  va_end(args);
  Serial.print(buf);
}

void BSP::pinMode(uint8_t pin, uint8_t mode)
{
  ::pinMode(pin, mode);
}

void BSP::attachInterrupt(uint8_t pin, void (*ISR)(), int mode)
{
  ::attachInterrupt(digitalPinToInterrupt(pin), ISR, mode);
}

unsigned long BSP::millis()
{
  return ::millis();
}