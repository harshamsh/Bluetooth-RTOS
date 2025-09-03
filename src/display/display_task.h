#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

void displayTask(void *pvParameters);

// Expose shared variables for metadata and audio level
extern char track_metadata[81];
extern volatile int audio_level;
extern SemaphoreHandle_t displayMutex;

extern QueueHandle_t metadataQueue;
void updateMetadataFromQueue();