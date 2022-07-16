#include "system_init.hpp"
#include <Arduino.h>
#include "system_config.h"


namespace
{
    u8 ledStatus = 0x00;
    uint32_t prevTime = 0x00;

    constexpr u32 kTogglePeriod = 100;
}
namespace systemns
{
    void initPeripheral()
    {
        Serial.begin(kSerialBaud);

        pinMode(LED_BUILTIN, OUTPUT);
        pinMode(kSystemRelayPin, OUTPUT);
        digitalWrite(LED_BUILTIN,ledStatus);
    }

    void heartbeat()
    {
        auto current = millis();
        if(current - prevTime < kTogglePeriod)
            return;

        prevTime = current;
        ledStatus = ledStatus? LOW:HIGH;
        digitalWrite(LED_BUILTIN,ledStatus);
    }
}