#include "bt_manager.h"
#include <Arduino.h>
#include <BluetoothA2DPSink.h>
#include <driver/i2s.h>
#include "display/display_task.h"

BluetoothA2DPSink a2dp_sink;
static uint32_t sample_count = 0;
static unsigned long last_time = 0;
static unsigned long last_data_time = 0;
static bool data_received = false;
bool mono_output = true;
extern char track_metadata[81];
extern volatile int audio_level;

void bt_play() { a2dp_sink.play(); }
void bt_pause() { a2dp_sink.pause(); }
void bt_next() { a2dp_sink.next(); }

void bt_audio_data_callback(const uint8_t *data, uint32_t len)
{
    // Mark that we're receiving data
    data_received = true;
    last_data_time = millis();

    int16_t *samples = (int16_t *)data;
    int num_samples = len / 2; // 2 bytes per sample (L+R interleaved)
    sample_count += num_samples;

    unsigned long now = millis();
    if (now - last_time >= 1000)
    { // Every second
        Serial.printf("Samples per second: %u\n", sample_count);
        sample_count = 0;
        last_time = now;
    }

    // Keep original serial prints for plotting
    Serial.print(">left:");
    Serial.println(samples[0]);
    Serial.print(">right:");
    Serial.println(samples[1]);
    
    if (mono_output)
    {
        // Convert to mono in-place
        for (int i = 0; i < num_samples; i += 2)
        {
            int16_t mono = (samples[i] / 2) + (samples[i + 1] / 2);
            samples[i] = mono;
            samples[i + 1] = mono;
        }
        Serial.print(">mono:");
        Serial.println(samples[0]);
    }
    
    int peak = 0;
    for (int i = 0; i < num_samples; ++i)
    {
        int v = abs(samples[i]);
        if (v > peak)
            peak = v;
    }
    audio_level = peak;

    size_t bytes_written = 0;
    esp_err_t err = i2s_write(I2S_NUM_0, data, len, &bytes_written, portMAX_DELAY);
    if (err != ESP_OK)
    {
        Serial.printf("I2S write failed: %d, bytes written: %d/%d\n", err, bytes_written, len);
    }
}

void avrc_metadata_callback(uint8_t id, const uint8_t *text)
{         
    if (text && strlen((const char*)text) > 0 && metadataQueue) {
        xQueueSend(metadataQueue, text, 0); // Non-blocking
    }                                                                                                                                                                                                                                                                                          
    // Serial.printf("AVRCP Metadata: %s\n", text);
    // strncpy(metadata, (const char *)text, 80);
    // metadata[80] = 0;
}

void bt_connection_state_changed(esp_a2d_connection_state_t state, void *ptr)
{
    if (state == ESP_A2D_CONNECTION_STATE_CONNECTED)
    {
        Serial.println("BT Device Connected!");
        Serial.println("Now try playing audio from your device...");
    }
    else if (state == ESP_A2D_CONNECTION_STATE_DISCONNECTED)
    {
        Serial.println("BT Device Disconnected!");
        data_received = false;
    }
}

void setupBT()
{
    Serial.println("Starting Bluetooth setup...");
    
    // Initialize Bluetooth
    a2dp_sink.set_stream_reader(bt_audio_data_callback, false);
    a2dp_sink.set_on_connection_state_changed(bt_connection_state_changed);
    a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
    
    Serial.println("Starting A2DP sink...");
    a2dp_sink.start("ESP32 Speaker");
    Serial.println("ESP32 Speaker advertising started");

    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 44100, // or 48000, match your source
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0};

    i2s_pin_config_t pin_config = {
        .bck_io_num = 26,   // Change to your wiring
        .ws_io_num = 25,    // Change to your wiring
        .data_out_num = 18,
        .data_in_num = I2S_PIN_NO_CHANGE};

    Serial.println("Installing I2S driver...");
    esp_err_t err;
    err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    if (err != ESP_OK)
    {
        Serial.printf("I2S driver install failed: %d\n", err);
        return;
    }
    Serial.println("I2S driver installed successfully");
    
    err = i2s_set_pin(I2S_NUM_0, &pin_config);
    if (err != ESP_OK)
    {
        Serial.printf("I2S set pin failed: %d\n", err);
        return;
    }
    Serial.println("I2S pins configured successfully");
    Serial.println("Bluetooth setup complete!");
}

void btTask(void *pvParameters)
{
    setupBT();
    
    while (1)
    {
        // Monitor connection status
        static unsigned long last_status_check = 0;
        if (millis() - last_status_check >= 10000) { // Every 10 seconds
            if (!data_received && last_data_time == 0) {
                Serial.println("Status: No audio data received yet. Is device connected and playing?");
            } else if (data_received && (millis() - last_data_time > 3000)) {
                Serial.println("Status: Audio stopped - no data for 3+ seconds");
            }
            last_status_check = millis();
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}