#ifndef LOGGER_TASK_HPP
#define LOGGER_TASK_HPP
#include "system_queues.hpp"
#include "system_config.h"
#include <Arduino.h>

//#define LOG_DEBUG(SOMETHING) Serial.println(SOMETHING)
#define LOG_DEBUG

//#define LOG_DEBUG(SOME_STRING)LoggerQueue::instance().getQueue().enqueue(SOME_STRING)

void launchLoggerTask();

#endif