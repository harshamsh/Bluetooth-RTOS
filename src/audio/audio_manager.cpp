#include "audio_manager.h"
#include <Arduino.h>




TaskHandle_t audioTaskHandle = NULL;
QueueHandle_t audioCommandQueue = NULL;
SemaphoreHandle_t audioMutex = NULL;

void audioTask(void *pvParameters) {
    AudioCommand cmd = CMD_STOP;
    while (1) {
        if (xQueueReceive(audioCommandQueue, &cmd, portMAX_DELAY) == pdTRUE) {
            if (xSemaphoreTake(audioMutex, portMAX_DELAY) == pdTRUE) {
            if (cmd == CMD_PLAY) {
                Serial.println("Audio: Playing...");
            } else if (cmd == CMD_PAUSE) {
                Serial.println("Audio: Paused.");
            } else if (cmd == CMD_STOP) {
                Serial.println("Audio: Stopped.");
            } else if (cmd == CMD_REWIND) {
                Serial.println("Audio: Rewinding.");
            } else if (cmd == CMD_NEXT) {
                Serial.println("Audio: Next.");
            } else if (cmd == CMD_PREVIOUS) {
                Serial.println("Audio: Previous.");
            } else if (cmd == CMD_MAX) {
                Serial.println("Audio: Max.");
            }
             xSemaphoreGive(audioMutex); // <-- Unlock
            }
        }
    }
}
