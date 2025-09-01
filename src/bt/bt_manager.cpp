#include "bt_manager.h"
#include <Arduino.h>
#include <BluetoothA2DPSink.h>

BluetoothA2DPSink a2dp_sink;

void bt_audio_data_callback(const uint8_t *data, uint32_t len)
{
    // For now, just print the length of received audio data
    Serial.printf("Received audio data: %u bytes\n", len);
    // TODO: Send data to audio output (I2S, MAX98357A, etc.)
}

void bt_connection_state_changed(esp_a2d_connection_state_t state, void *ptr)
{
    if (state == ESP_A2D_CONNECTION_STATE_CONNECTED)
    {
        Serial.println("BT Device Connected!");
    }
    else if (state == ESP_A2D_CONNECTION_STATE_DISCONNECTED)
    {
        Serial.println("BT Device Disconnected!");
    }
}

void setupBT()
{
    // Initialize Bluetooth
    a2dp_sink.set_stream_reader(bt_audio_data_callback, false);
    a2dp_sink.set_on_connection_state_changed(bt_connection_state_changed);
    a2dp_sink.start("ESP32 Speaker");



    // const uint8_t* mac = esp_bt_dev_get_address();
    // char macStr[18];
    // sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
    //         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    // Serial.print("BT Classic MAC Address: ");
    // Serial.println(macStr);
}

void btTask(void *pvParameters)
{
    setupBT();
    while (1)
    {
        // Placeholder for BT event handling
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}