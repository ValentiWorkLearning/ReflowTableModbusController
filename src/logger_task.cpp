#include "logger_task.hpp"
#include "system_queues.hpp"
#include "system_config.h"

#include <Arduino.h>

static void loggerTask(void *pvParameters)
{
    for (;;)
    {
        char* pLogged{nullptr};
        bool hasMessage = LoggerQueue::instance().getQueue().dequeue(&pLogged);
        while(hasMessage)
        {
            hasMessage = LoggerQueue::instance().getQueue().dequeue(&pLogged);
        }
        taskYIELD();
    }
}

void launchLoggerTask()
{
    xTaskCreate(loggerTask, "Logger", 128, NULL, 1, NULL);
}