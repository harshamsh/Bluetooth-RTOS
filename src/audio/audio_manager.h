#pragma once

#include <Arduino.h> // <-- Add this line first
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

extern SemaphoreHandle_t audioMutex;


enum AudioCommand {
    CMD_PLAY,
    CMD_PAUSE,
    CMD_STOP,
    CMD_REWIND,
    CMD_NEXT,
    CMD_PREVIOUS,
    CMD_MAX
};

extern TaskHandle_t audioTaskHandle;
extern QueueHandle_t audioCommandQueue;

void audioTask(void *pvParameters);