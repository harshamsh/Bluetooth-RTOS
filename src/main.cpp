#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

TaskHandle_t ButtonTaskHandle = NULL;
SemaphoreHandle_t buttonSemaphore;
volatile int buttonCount = 0;
const int buttonPin = 23; // Change to your button pin

void IRAM_ATTR handleButtonInterrupt() { // ISR to handle button press
  xSemaphoreGiveFromISR(buttonSemaphore, NULL);
}

void ButtonTask(void *pvParameters) {
  pinMode(buttonPin, INPUT_PULLUP);
  while (1) {
    if (xSemaphoreTake(buttonSemaphore, portMAX_DELAY) == pdTRUE) {
      buttonCount++;
      Serial.print("Button pressed! Count: ");
      Serial.println(buttonCount);
      vTaskDelay(500 / portTICK_PERIOD_MS); // Debounce
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  buttonSemaphore = xSemaphoreCreateBinary();
  if (buttonSemaphore == NULL) {
    Serial.println("Error creating semaphore");
    while (1);
  }

  pinMode(buttonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPin), handleButtonInterrupt, FALLING);

  xTaskCreatePinnedToCore(
    ButtonTask,
    "ButtonTask",
    2048,
    NULL,
    1,
    &ButtonTaskHandle,
    0
  );
}

void loop() {}