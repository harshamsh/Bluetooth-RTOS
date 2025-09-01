#include "ble_manager.h"
#include "audio/audio_manager.h"
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID "12345678-1234-1234-1234-1234567890ab"
#define CHAR_CONTROL_UUID "12345678-1234-1234-1234-1234567890ac"
#define CHAR_STATUS_UUID "12345678-1234-1234-1234-1234567890ad"

BLECharacteristic *statusCharacteristic = nullptr;

class ControlCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) override {
    std::string value = pCharacteristic->getValue();
    Serial.print("Received command: ");
    Serial.println(value.c_str());
    AudioCommand cmd;
    if (value == "PLAY") {
      statusCharacteristic->setValue("Playing");
      statusCharacteristic->notify();
      cmd = CMD_PLAY;
    } else if (value == "PAUSE") {
      statusCharacteristic->setValue("Paused");
      statusCharacteristic->notify();
      cmd = CMD_PAUSE;
    } else if (value == "STOP") {
      statusCharacteristic->setValue("Stopped");
      statusCharacteristic->notify();
      cmd = CMD_STOP;
    } else if (value == "REWIND") {
      statusCharacteristic->setValue("Rewinding");
      statusCharacteristic->notify();
      cmd = CMD_REWIND;
    } else if (value == "NEXT") {
      statusCharacteristic->setValue("Next");
      statusCharacteristic->notify();
      cmd = CMD_NEXT;
    } else if (value == "PREVIOUS") {
      statusCharacteristic->setValue("Previous");
      statusCharacteristic->notify();
      cmd = CMD_PREVIOUS;
    } else if (value == "MAX") {
      statusCharacteristic->setValue("Max");
      statusCharacteristic->notify();
      cmd = CMD_MAX;
    } else {
      statusCharacteristic->setValue("Unknown command");
      statusCharacteristic->notify();
      return;
    }
    if (audioCommandQueue != NULL) {
      xQueueSend(audioCommandQueue, &cmd, portMAX_DELAY);
    }
  }
};

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) override {
    Serial.println("BLE client connected");
    statusCharacteristic->setValue("Connected");
    statusCharacteristic->notify();
  }
  void onDisconnect(BLEServer *pServer) override {
    Serial.println("BLE client disconnected");
  }
};

void setupBLE() {
  BLEDevice::init("MSHAudioPlayer");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *controlCharacteristic = pService->createCharacteristic(
      CHAR_CONTROL_UUID,
      BLECharacteristic::PROPERTY_WRITE);
  controlCharacteristic->setCallbacks(new ControlCallbacks());

  statusCharacteristic = pService->createCharacteristic(
      CHAR_STATUS_UUID,
      BLECharacteristic::PROPERTY_NOTIFY);
  statusCharacteristic->addDescriptor(new BLE2902());

  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();

  Serial.println("BLE AudioPlayer advertising...");
  // Serial.print("BLE MAC Address: ");
  // Serial.println(BLEDevice::getAddress().toString().c_str());
}

void bleTask(void *pvParameters) {
    setupBLE(); // Initialize BLE
    while (1) {
        // You can add BLE-related periodic checks or notifications here
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}