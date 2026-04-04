#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

// Clockface
#include <Clockface.h>
// Commons
#include <WiFiController.h>
#include <CWDateTime.h>
#include <CWPreferences.h>
#include <CWWebServer.h>
#include <StatusController.h>
#include <ezButton.h>
#include "PanelConfig.h"

#ifndef CW_PANEL_TYPE
#define CW_PANEL_TYPE HOME_PANEL
#endif

#ifndef CW_DEBUG_HUB75_PINS
#define CW_DEBUG_HUB75_PINS 0
#endif

#define SCREEN_AUTO 0
#define SCREEN_OFF 1
#define SCREEN_ON 2

#define ESP32_LED_BUILTIN 2

#if CW_PANEL_TYPE == OGUZALI_PANEL
ezButton button(39);
#endif

uint8_t screenMode = SCREEN_AUTO;
bool screenModeChange = false;

#define BRIGHTNESS_STEPS 10
#define AUTO_BRIGHT_STEP_HYSTERESIS 24

MatrixPanel_I2S_DMA *dma_display = nullptr;

Clockface *clockface;

WiFiController wifi;
CWDateTime cwDateTime;

long autoBrightMillis = 0;
uint8_t currentBrightness = 0;
uint8_t targetBrightness = 0;

static void setupArduinoOta()
{
  ArduinoOTA.setHostname("clockwise");

  ArduinoOTA.onStart([]() {
    Serial.println("[OTA] Update started");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\n[OTA] Update finished");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("[OTA] Progress: %u%%\r", (progress * 100U) / total);
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[OTA] Error[%u]\n", error);
  });

  ArduinoOTA.begin();
  Serial.println("[OTA] Ready");
}

static void printHub75Pins(const HUB75_I2S_CFG &mxconfig)
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

void displaySetup(uint8_t displayBright, uint8_t displayRotation)
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

  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(displayBright);
  currentBrightness = displayBright;
  targetBrightness = displayBright;
  dma_display->clearScreen();
  dma_display->setRotation(displayRotation);
}

unsigned long lastFadeMillis = 0;

#define FADE_INTERVAL 20
#define FADE_STEP 1

void updateBrightnessSmooth()
{
  if (millis() - lastFadeMillis < FADE_INTERVAL)
    return;

  lastFadeMillis = millis();

  if (currentBrightness < targetBrightness)
  {
    currentBrightness += FADE_STEP;
    if (currentBrightness > targetBrightness)
      currentBrightness = targetBrightness;
  }
  else if (currentBrightness > targetBrightness)
  {
    currentBrightness -= FADE_STEP;
    if (currentBrightness < targetBrightness)
      currentBrightness = targetBrightness;
  }
  else
  {
    return;
  }

  dma_display->setBrightness8(currentBrightness);
}


uint8_t lastStep = 255;
bool isDark = false;
static float filteredLdr = 0;

static uint8_t brightnessForStep(uint8_t step, uint8_t maxBright)
{
  return map(step, 1, BRIGHTNESS_STEPS - 1, 4, maxBright);
}


void automaticBrightControl()
{
  uint16_t raw = analogRead(ClockwiseParams::getInstance()->ldrPin);

  filteredLdr = 0.8 * filteredLdr + 0.2 * raw;
  uint16_t ldr = (uint16_t)filteredLdr;

  uint16_t ldrMin = ClockwiseParams::getInstance()->autoBrightMin;
  uint16_t ldrMax = ClockwiseParams::getInstance()->autoBrightMax;
  uint8_t maxBright = ClockwiseParams::getInstance()->displayBright;

  if (ldrMin >= ldrMax)
    return;

  // dark mode
  if (!isDark && ldr < ldrMin)
  {
    isDark = true;
    //dma_display->setBrightness8(0);
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

  if (millis() - autoBrightMillis < 500) return;
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
  int32_t currentStepMin = ldrMin + ((int32_t)(lastStep - 1) * stepWidth);
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
  


void setup()
{
  Serial.begin(115200);
  pinMode(ESP32_LED_BUILTIN, OUTPUT);

#if CW_PANEL_TYPE == OGUZALI_PANEL
  button.setDebounceTime(50);
#endif

  StatusController::getInstance()->blink_led(5, 100);

  ClockwiseParams::getInstance()->load();
  screenMode = ClockwiseParams::getInstance()->screenMode;
  if (screenMode > SCREEN_ON)
    screenMode = SCREEN_AUTO;
  screenModeChange = true;

  pinMode(ClockwiseParams::getInstance()->ldrPin, INPUT);

  displaySetup(ClockwiseParams::getInstance()->displayBright, ClockwiseParams::getInstance()->displayRotation);
  clockface = new Clockface(dma_display);


  StatusController::getInstance()->clockwiseLogo();
  delay(1000);

  StatusController::getInstance()->wifiConnecting();
  if (wifi.begin())
  {
    setupArduinoOta();
    StatusController::getInstance()->ntpConnecting();
    cwDateTime.begin(ClockwiseParams::getInstance()->timeZone.c_str(),
        ClockwiseParams::getInstance()->use24hFormat,
        ClockwiseParams::getInstance()->ntpServer.c_str(),
        ClockwiseParams::getInstance()->manualPosix.c_str());
    clockface->setup(&cwDateTime);
  }
}

void loop()
{
  wifi.handleImprovWiFi();

  if (wifi.isConnected())
  {
    ArduinoOTA.handle();
    ClockwiseWebServer::getInstance()->handleHttpRequest();
    ezt::events();
  }

  if (wifi.connectionSucessfulOnce)
  {
    clockface->update();
  }

  uint8_t desiredScreenMode = ClockwiseParams::getInstance()->screenMode;
  if (desiredScreenMode > SCREEN_ON)
    desiredScreenMode = SCREEN_AUTO;

  if (desiredScreenMode != screenMode)
  {
    screenMode = desiredScreenMode;
    screenModeChange = true;
  }
  
  if(screenModeChange)
  {
    if(screenMode == SCREEN_OFF)
    {
      dma_display->setBrightness8(0);
      screenModeChange = false;
    }
    if(screenMode == SCREEN_ON)
    {
      uint8_t setBright = ClockwiseParams::getInstance()->displayBright;
      //dma_display->setBrightness8(setBright);
      targetBrightness = setBright;
      screenModeChange = false;
    }
  }
  if(screenMode == SCREEN_AUTO)
  {
    if(screenModeChange)
    {
      screenModeChange = false;
    }
    automaticBrightControl();
  }

  updateBrightnessSmooth();

#if CW_PANEL_TYPE == OGUZALI_PANEL
  button.loop();

  if(button.isPressed())
  {
    screenMode = (screenMode+1)%3;
    ClockwiseParams::getInstance()->screenMode = screenMode;
    ClockwiseParams::getInstance()->save();
    screenModeChange = true;
  }
#endif
}
