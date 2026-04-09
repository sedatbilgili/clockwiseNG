#pragma once

#include <stdint.h>

class MatrixPanel_I2S_DMA;
class Clockface;

namespace DisplayRuntime
{
void initialize(uint8_t initialScreenMode, uint8_t ldrPin);
MatrixPanel_I2S_DMA *createDisplay(uint8_t displayBright, uint8_t displayRotation);
Clockface *createClockface(MatrixPanel_I2S_DMA *dmaDisplay);
void updateScreenMode(MatrixPanel_I2S_DMA *dmaDisplay);
void handlePanelButton();
}
