#include "heartbeat_task.hpp"
#include <Arduino_FreeRTOS.h>
#include <Arduino.h>
#include "logger_task.hpp"

static void blinkerTask(void *pvParameters)
{
    for (;;)
    {
        digitalWrite(LED_BUILTIN, HIGH);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        digitalWrite(LED_BUILTIN, LOW);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void launchHeartBeatTask()
{
    xTaskCreate(blinkerTask, "Blink", 64, NULL, 0, NULL);
}