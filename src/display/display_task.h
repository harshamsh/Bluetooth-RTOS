#pragma once

void displayTask(void *pvParameters);

// Expose shared variables for metadata and audio level
extern char metadata[81];
extern volatile int audio_level;