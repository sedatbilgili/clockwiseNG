#include "DisplayRuntime.h"

#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <new>

#include "Clockface.h"
#include "PanelConfig.h"
#include "gfx/assets.h"
#include <CWPreferences.h>
#include <ezButton.h>

#ifndef CW_PANEL_TYPE
#define CW_PANEL_TYPE HOME_PANEL
#endif

#ifndef CW_DEBUG_HUB75_PINS
#define CW_DEBUG_HUB75_PINS 0
#endif

#define SCREEN_AUTO 0
#define SCREEN_OFF 1
#define SCREEN_ON 2

#define BRIGHTNESS_STEPS 10
#define AUTO_BRIGHT_STEP_HYSTERESIS 24
#define FADE_INTERVAL 20
#define FADE_STEP 1
#define SCREEN_MODE_ICON_SIZE 9
#define SCREEN_MODE_ICON_X 1
#define SCREEN_MODE_ICON_Y 9
#define SCREEN_MODE_ICON_VISIBLE_MS 1000UL

namespace
{
#if CW_PANEL_TYPE == OGUZALI_PANEL
ezButton button(33, INTERNAL_PULLDOWN);
#endif

uint8_t screenMode = SCREEN_AUTO;
bool screenModeChange = false;

alignas(MatrixPanel_I2S_DMA) uint8_t dmaDisplayStorage[sizeof(MatrixPanel_I2S_DMA)];
alignas(Clockface) uint8_t clockfaceStorage[sizeof(Clockface)];
bool dmaDisplayConstructed = false;
bool clockfaceConstructed = false;

long autoBrightMillis = 0;
uint8_t currentBrightness = 0;
uint8_t targetBrightness = 0;
unsigned long lastFadeMillis = 0;
uint8_t lastStep = 255;
bool isDark = false;
float filteredLdr = 0;
unsigned long screenModeIconStartedMillis = 0;
bool screenModeIconVisible = false;
const unsigned short *screenModeIcon = nullptr;

void startScreenModeIconOverlay(uint8_t mode)
{
  screenModeIconStartedMillis = millis();
  screenModeIconVisible = true;
  if (mode == SCREEN_ON)
  {
    screenModeIcon = onIcon;
    return;
  }

  if (mode == SCREEN_AUTO)
  {
    screenModeIcon = autoIcon;
    return;
  }

  // SCREEN_OFF has no dedicated icon asset, keep the area clear.
  screenModeIcon = nullptr;
}

void applyScreenMode(uint8_t newMode)
{
  uint8_t boundedMode = newMode > SCREEN_ON ? SCREEN_AUTO : newMode;
  if (boundedMode == screenMode)
  {
    return;
  }

  screenMode = boundedMode;
  screenModeChange = true;
  startScreenModeIconOverlay(screenMode);
}

void renderScreenModeIconOverlay(MatrixPanel_I2S_DMA *dmaDisplay)
{
  if (!screenModeIconVisible)
  {
    return;
  }

  if (millis() - screenModeIconStartedMillis >= SCREEN_MODE_ICON_VISIBLE_MS)
  {
    screenModeIconVisible = false;
    return;
  }

  // Always clear the icon box before drawing so quick mode changes do not overlap.
  dmaDisplay->fillRect(
    SCREEN_MODE_ICON_X,
    SCREEN_MODE_ICON_Y,
    SCREEN_MODE_ICON_SIZE,
    SCREEN_MODE_ICON_SIZE,
    _MASK);

  if (screenModeIcon == nullptr)
  {
    return;
  }

  for (uint8_t y = 0; y < SCREEN_MODE_ICON_SIZE; ++y)
  {
    for (uint8_t x = 0; x < SCREEN_MODE_ICON_SIZE; ++x)
    {
      const uint16_t color = screenModeIcon[static_cast<size_t>(y) * SCREEN_MODE_ICON_SIZE + x];
      if (color == _MASK)
      {
        continue;
      }

      dmaDisplay->drawPixel(SCREEN_MODE_ICON_X + x, SCREEN_MODE_ICON_Y + y, color);
    }
  }
}

void printHub75Pins(const HUB75_I2S_CFG &mxconfig)
{
#if CW_DEBUG_HUB75_PINS
  Serial.println("HUB75 pin config:");
  Serial.printf("  r1: %d\n", mxconfig.gpio.r1);
  Serial.printf("  r2: %d\n", mxconfig.gpio.r2);
  Serial.printf("  g1: %d\n", mxconfig.gpio.g1);
  Serial.printf("  g2: %d\n", mxconfig.gpio.g2);
  Serial.printf("  b1: %d\n", mxconfig.gpio.b1);
  Serial.printf("  b2: %d\n", mxconfig.gpio.b2);
  Serial.printf("  a: %d\n", mxconfig.gpio.a);
  Serial.printf("  b: %d\n", mxconfig.gpio.b);
  Serial.printf("  c: %d\n", mxconfig.gpio.c);
  Serial.printf("  d: %d\n", mxconfig.gpio.d);
  Serial.printf("  e: %d\n", mxconfig.gpio.e);
  Serial.printf("  clk: %d\n", mxconfig.gpio.clk);
  Serial.printf("  lat: %d\n", mxconfig.gpio.lat);
#else
  (void)mxconfig;
#endif
}

void updateBrightnessSmooth(MatrixPanel_I2S_DMA *dmaDisplay)
{
  if (millis() - lastFadeMillis < FADE_INTERVAL)
  {
    return;
  }

  lastFadeMillis = millis();

  if (currentBrightness < targetBrightness)
  {
    currentBrightness += FADE_STEP;
    if (currentBrightness > targetBrightness)
    {
      currentBrightness = targetBrightness;
    }
  }
  else if (currentBrightness > targetBrightness)
  {
    currentBrightness -= FADE_STEP;
    if (currentBrightness < targetBrightness)
    {
      currentBrightness = targetBrightness;
    }
  }
  else
  {
    return;
  }

  dmaDisplay->setBrightness8(currentBrightness);
}

uint8_t brightnessForStep(uint8_t step, uint8_t maxBright)
{
  return map(step, 1, BRIGHTNESS_STEPS - 1, 4, maxBright);
}

void automaticBrightControl()
{
  uint16_t raw = analogRead(ClockwiseParams::getInstance()->ldrPin);

  filteredLdr = 0.8f * filteredLdr + 0.2f * raw;
  uint16_t ldr = static_cast<uint16_t>(filteredLdr);

  uint16_t ldrMin = ClockwiseParams::getInstance()->autoBrightMin;
  uint16_t ldrMax = ClockwiseParams::getInstance()->autoBrightMax;
  uint8_t maxBright = ClockwiseParams::getInstance()->displayBright;

  if (ldrMin >= ldrMax)
  {
    return;
  }

  if (!isDark && ldr < ldrMin)
  {
    isDark = true;
    targetBrightness = 0;
    return;
  }

  if (isDark && ldr > ldrMin + 150)
  {
    isDark = false;
  }

  if (isDark)
  {
    lastStep = 255;
    return;
  }

  if (ldr > ldrMax) ldr = ldrMax;
  if (ldr < ldrMin) ldr = ldrMin;

  if (millis() - autoBrightMillis < 500)
  {
    return;
  }
  autoBrightMillis = millis();

  uint8_t step = map(ldr, ldrMin, ldrMax, 1, BRIGHTNESS_STEPS - 1);

  if (lastStep == 255)
  {
    lastStep = step;
    targetBrightness = brightnessForStep(step, maxBright);
    return;
  }

  int32_t range = ldrMax - ldrMin;
  int32_t stepWidth = max<int32_t>(1, range / (BRIGHTNESS_STEPS - 1));
  int32_t hysteresis = min<int32_t>(AUTO_BRIGHT_STEP_HYSTERESIS, stepWidth / 3);
  int32_t currentStepMin = ldrMin + (static_cast<int32_t>(lastStep - 1) * stepWidth);
  int32_t currentStepMax = currentStepMin + stepWidth;

  if (ldr < currentStepMin - hysteresis && lastStep > 1)
  {
    lastStep--;
  }
  else if (ldr > currentStepMax + hysteresis && lastStep < BRIGHTNESS_STEPS - 1)
  {
    lastStep++;
  }
  else
  {
    return;
  }

  targetBrightness = brightnessForStep(lastStep, maxBright);
}
} // namespace

namespace DisplayRuntime
{
void initialize(uint8_t initialScreenMode, uint8_t ldrPin)
{
#if CW_PANEL_TYPE == OGUZALI_PANEL
  button.setDebounceTime(50);
#endif

  screenMode = initialScreenMode > SCREEN_ON ? SCREEN_AUTO : initialScreenMode;
  screenModeChange = true;
  screenModeIconVisible = false;
  screenModeIcon = nullptr;
  pinMode(ldrPin, INPUT);
}

MatrixPanel_I2S_DMA *createDisplay(uint8_t displayBright, uint8_t displayRotation)
{
  HUB75_I2S_CFG mxconfig(64, 64, 1);

  PanelPins p = panelConfigs[CW_PANEL_TYPE - 1];

  mxconfig.gpio.r1 = p.r1;
  mxconfig.gpio.r2 = p.r2;
  mxconfig.gpio.g1 = p.g1;
  mxconfig.gpio.g2 = p.g2;
  mxconfig.gpio.b1 = p.b1;
  mxconfig.gpio.b2 = p.b2;
  mxconfig.gpio.a = p.a;
  mxconfig.gpio.b = p.b;
  mxconfig.gpio.c = p.c;
  mxconfig.gpio.d = p.d;
  mxconfig.gpio.e = p.e;
  mxconfig.gpio.clk = p.clk;
  mxconfig.gpio.lat = p.lat;
  mxconfig.clkphase = false;

  printHub75Pins(mxconfig);

  MatrixPanel_I2S_DMA *dmaDisplay = reinterpret_cast<MatrixPanel_I2S_DMA *>(dmaDisplayStorage);
  if (!dmaDisplayConstructed)
  {
    dmaDisplay = new (dmaDisplayStorage) MatrixPanel_I2S_DMA(mxconfig);
    dmaDisplayConstructed = true;
  }

  dmaDisplay->begin();
  dmaDisplay->setBrightness8(displayBright);
  currentBrightness = displayBright;
  targetBrightness = displayBright;
  dmaDisplay->clearScreen();
  dmaDisplay->setRotation(displayRotation);
  return dmaDisplay;
}

Clockface *createClockface(MatrixPanel_I2S_DMA *dmaDisplay)
{
  Clockface *clockface = reinterpret_cast<Clockface *>(clockfaceStorage);
  if (!clockfaceConstructed)
  {
    clockface = new (clockfaceStorage) Clockface(dmaDisplay);
    clockfaceConstructed = true;
  }

  return clockface;
}

void updateScreenMode(MatrixPanel_I2S_DMA *dmaDisplay)
{
  uint8_t desiredScreenMode = ClockwiseParams::getInstance()->screenMode;
  if (desiredScreenMode > SCREEN_ON)
  {
    desiredScreenMode = SCREEN_AUTO;
  }

  if (desiredScreenMode != screenMode)
  {
    applyScreenMode(desiredScreenMode);
  }

  if (screenModeChange)
  {
    if (screenMode == SCREEN_OFF)
    {
      // Keep software brightness state in sync with forced panel-off brightness.
      targetBrightness = 0;
      currentBrightness = 0;
      dmaDisplay->setBrightness8(0);
      screenModeChange = false;
    }
    if (screenMode == SCREEN_ON)
    {
      uint8_t setBright = ClockwiseParams::getInstance()->displayBright;
      targetBrightness = setBright;
      screenModeChange = false;
    }
  }
  if (screenMode == SCREEN_AUTO)
  {
    if (screenModeChange)
    {
      autoBrightMillis = 0;
      lastStep = 255;
      screenModeChange = false;
    }
    automaticBrightControl();
  }

  updateBrightnessSmooth(dmaDisplay);
  renderScreenModeIconOverlay(dmaDisplay);
}

void handlePanelButton()
{
#if CW_PANEL_TYPE == OGUZALI_PANEL
  button.loop();

  if (button.isPressed())
  {
    applyScreenMode((screenMode + 1) % 3);
    ClockwiseParams::getInstance()->screenMode = screenMode;
    ClockwiseParams::getInstance()->save();
  }
#endif
}
} // namespace DisplayRuntime
