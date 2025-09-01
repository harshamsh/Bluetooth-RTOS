#include <Arduino.h>
#include "audio/audio_manager.h"
#include "ble/ble_manager.h"
#include "bt/bt_manager.h"

TaskHandle_t btTaskHandle = NULL;

TaskHandle_t bleTaskHandle = NULL;

void setup() {
  Serial.begin(115200);

  audioCommandQueue = xQueueCreate(4, sizeof(AudioCommand));
  if (audioCommandQueue == NULL) {
    Serial.println("Error creating audio command queue!");
    while (1);
  }


  audioMutex = xSemaphoreCreateMutex(); // <-- Add this line
  if (audioMutex == NULL) {
    Serial.println("Error creating audio mutex!");
    while (1);
  }

  xTaskCreatePinnedToCore(audioTask, "AudioTask", 2048, NULL, 2, &audioTaskHandle, 1); // Priority 2
  // xTaskCreatePinnedToCore(bleTask, "BLETask", 8192, NULL, 1, &bleTaskHandle, 1);       // Priority 1
  // xTaskCreatePinnedToCore(btTask, "BTTask", 8192, NULL, 1, &btTaskHandle, 1);         // Priority 1

  setupBLE();
  setupBT();

}

void loop() {}