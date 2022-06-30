#include "system_init.hpp"
#include <Arduino.h>
#include "system_config.h"

void initPeripheral()
{
    Serial.begin(kSerialBaud);

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(kSystemRelayPin, OUTPUT);
}