#ifndef INTERNAL_MESSAGES_HPP
#define INTERNAL_MESSAGES_HPP
#include <Arduino.h>

struct ThermalControlMessage
{
    u8 id;
    u8 payload; 
};


constexpr u8 SET_TEMPERATURE = 0x12;
constexpr u8 EMERGENCY_STOP = 0x14;
constexpr u8 REQUEST_TEMPERATURE = 0x22; 

#endif