#include <Arduino.h>
#include "audio/audio_manager.h"
#include "ble/ble_manager.h"
#include "bt/bt_manager.h"

TaskHandle_t btTaskHandle = NULL;
extern void displayTask(void *pvParameters);
TaskHandle_t displayTaskHandle = NULL;

#define PIN_PLAY  32
#define PIN_PAUSE 33
#define PIN_NEXT  34

void setup() {
  pinMode(PIN_PLAY, INPUT_PULLUP);
  pinMode(PIN_PAUSE, INPUT_PULLUP);
  pinMode(PIN_NEXT, INPUT_PULLUP);
  Serial.begin(115200);

  audioCommandQueue = xQueueCreate(4, sizeof(AudioCommand));
  if (!audioCommandQueue) { Serial.println("Error creating audio command queue!"); while (1); }

  audioMutex = xSemaphoreCreateMutex();
  if (!audioMutex) { Serial.println("Error creating audio mutex!"); while (1); }

  xTaskCreatePinnedToCore(audioTask, "AudioTask", 2048, NULL, 2, &audioTaskHandle, 1);
  xTaskCreatePinnedToCore(displayTask, "DisplayTask", 4096, NULL, 1, &displayTaskHandle, 1);

  setupBLE();
  setupBT();   // or xTaskCreatePinnedToCore(btTask, ...) if you prefer a task
}

void loop() {
  if (digitalRead(PIN_PLAY) == LOW)  { bt_play();  Serial.println("Play pressed");  delay(200); }
  if (digitalRead(PIN_PAUSE) == LOW) { bt_pause(); Serial.println("Pause pressed"); delay(200); }
  // if (digitalRead(PIN_NEXT) == LOW) { bt_next(); delay(200); }
}
