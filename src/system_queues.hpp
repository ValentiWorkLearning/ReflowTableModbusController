#ifndef SYSTEM_QUEUES
#define SYSTEM_QUEUES

#include "internal_messages.hpp"

#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include "system_config.h"

template <typename TTypedQueeue, u8 QueueSize = 10, TickType_t kQueueWaitTimeout = portMAX_DELAY>
class TypedQueue
{

public:
    TypedQueue() noexcept
        : messageQueue{xQueueCreate(QueueSize, sizeof(TTypedQueeue))}
    {
    }
    void enqueue(const TTypedQueeue &toPush) noexcept
    {
        xQueueSend(messageQueue, &toPush, kQueueWaitTimeout);
    }

    bool dequeue(TTypedQueeue *output) noexcept
    {
        return xQueueReceive(messageQueue,
                             output,
                             kQueueWaitTimeout) == pdTRUE;
    }

private:
    QueueHandle_t messageQueue;
};

class ThermalControlMessageQueue
{

public:
    using TThermalQueueInstance = TypedQueue<ThermalControlMessage,10,10>;

    static ThermalControlMessageQueue &instance() noexcept
    {
        static ThermalControlMessageQueue meInstance;
        return meInstance;
    }

    const TThermalQueueInstance &getQueue() const noexcept
    {
        return messageQueue;
    }

    TThermalQueueInstance &getQueue() noexcept
    {
        return messageQueue;
    }

private:
    TThermalQueueInstance messageQueue;
};

class LoggerQueue
{
public:
    using TLoggerQueueInstance = TypedQueue<char*, 10>;

    static LoggerQueue &instance() noexcept
    {
        static LoggerQueue meInstance;
        return meInstance;
    }

    const TLoggerQueueInstance &getQueue() const noexcept
    {
        return messageQueue;
    }

    TLoggerQueueInstance &getQueue() noexcept
    {
        return messageQueue;
    }

private:
    TLoggerQueueInstance messageQueue;
};

#endif