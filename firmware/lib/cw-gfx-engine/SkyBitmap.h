#pragma once

#include <Adafruit_GFX.h>

static const uint16_t SKY_PIXEL_COLOR = 0x0007;
static const uint16_t DEFAULT_SKY_DAY_COLOR = 0x000e;

inline uint16_t &activeSkyColorRef() {
  static uint16_t activeSkyColor = DEFAULT_SKY_DAY_COLOR;
  return activeSkyColor;
}

inline uint16_t getActiveSkyColor() {
  return activeSkyColorRef();
}

inline void setActiveSkyColor(uint16_t color) {
  activeSkyColorRef() = color;
}

inline void drawBitmapWithActiveSky(Adafruit_GFX *display, int16_t x, int16_t y, const unsigned short *bitmap, int16_t width, int16_t height) {
  static uint16_t rowBuffer[64];

  for (int16_t row = 0; row < height; row++) {
    for (int16_t col = 0; col < width; col++) {
      uint16_t pixel = bitmap[(row * width) + col];
      rowBuffer[col] = (pixel == SKY_PIXEL_COLOR) ? getActiveSkyColor() : pixel;
    }
    display->drawRGBBitmap(x, y + row, rowBuffer, width, 1);
  }
}
