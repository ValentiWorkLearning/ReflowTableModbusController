#include "system_init.hpp"
#include <Arduino.h>
#include "system_config.h"
#include "logger_task.hpp"

void initPeripheral()
{
    Serial.begin(SERIAL_BAUDRATE);

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(SYSTEM_RELAY_PIN, OUTPUT);

    LOG_DEBUG("System init has been completed");

}