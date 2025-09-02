#include <Arduino.h>
#include <U8g2lib.h>

// Use correct constructor for your SSD1306 (I2C example)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// Shared metadata buffer (update from your AVRCP callback)
char metadata[81] = "No track info";
volatile int meta_scroll = 0;

// Shared audio level (update from audio callback)
volatile int audio_level = 0;

void displayTask(void *pvParameters) {
    Serial.println("Display task started");
    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.setContrast(255);
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 24, "Display OK!");
    u8g2.sendBuffer();
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Show for 1 second

    while (1) {
        u8g2.clearBuffer();

        // Draw scrolling metadata (top 2 lines)
        int meta_len = strlen(metadata);
        int scroll_pos = meta_scroll % (meta_len + 16); // 16 chars visible
        char line[17] = {0};
        if (meta_len > 16) {
            if (scroll_pos + 16 <= meta_len)
                strncpy(line, metadata + scroll_pos, 16);
            else {
                int first = meta_len - scroll_pos;
                strncpy(line, metadata + scroll_pos, first);
                strncpy(line + first, metadata, 16 - first);
            }
        } else {
            strncpy(line, metadata, 16);
        }
        u8g2.setFont(u8g2_font_6x12_tr);
        u8g2.drawStr(0, 12, line);

        // Draw audio visualization (simple VU bar)
        u8g2.drawFrame(0, 20, 128, 12);
        int bar_width = map(audio_level, 0, 32767, 0, 126);
        u8g2.drawBox(1, 21, bar_width, 10);

        u8g2.sendBuffer();

        meta_scroll++;
        vTaskDelay(150 / portTICK_PERIOD_MS); // ~7 FPS
    }
}