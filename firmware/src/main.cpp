#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <HTTPClient.h>
#include <Update.h>
#include <WiFiClientSecure.h>
#include <new>
#include "esp_bt.h"

// Clockface
#include "Clockface.h"
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

#ifndef CW_DEBUG_OTA
#define CW_DEBUG_OTA 1
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

static MatrixPanel_I2S_DMA *dma_display = nullptr;
static Clockface *clockface = nullptr;
alignas(MatrixPanel_I2S_DMA) static uint8_t dmaDisplayStorage[sizeof(MatrixPanel_I2S_DMA)];
alignas(Clockface) static uint8_t clockfaceStorage[sizeof(Clockface)];
static bool dmaDisplayConstructed = false;
static bool clockfaceConstructed = false;

WiFiController wifi;
CWDateTime cwDateTime;
volatile bool otaInProgress = false;
volatile uint8_t otaProgress = 0;
static unsigned long otaLastProgressMillis = 0;
static unsigned long otaLastProgressLogMillis = 0;
static uint8_t otaLastRenderedProgress = 255;
static unsigned long otaLastRenderMillis = 0;
static unsigned long otaStartMillis = 0;
static unsigned long otaLastStallLogMillis = 0;
static uint8_t otaLastLoggedProgress = 255;
static const unsigned long OTA_RENDER_INTERVAL_MS = 80UL;

long autoBrightMillis = 0;
uint8_t currentBrightness = 0;
uint8_t targetBrightness = 0;
static const unsigned long CLOCKFACE_UPDATE_INTERVAL_MS = 33UL;
static unsigned long lastClockfaceUpdateMillis = 0;
static const unsigned long EZT_EVENTS_INTERVAL_MS = 100UL;
static unsigned long lastEztEventsMillis = 0;

static void prepareForOta()
{
  ClockwiseWebServer::getInstance()->stopWebServer();
  MDNS.end();
}

static void renderOtaProgressIfNeeded(bool force = false)
{
  if (!otaInProgress)
  {
    return;
  }

  const uint8_t bucket = min<uint8_t>(10, otaProgress / 10);
  const unsigned long now = millis();
  if (!force)
  {
    if (bucket == otaLastRenderedProgress)
    {
      return;
    }

    if (now - otaLastRenderMillis < OTA_RENDER_INTERVAL_MS)
    {
      return;
    }
  }

  StatusController::getInstance()->otaUpdating(otaProgress);
  otaLastRenderedProgress = bucket;
  otaLastRenderMillis = now;
}

static void renderOtaProgressBucketIfChanged()
{
  if (!otaInProgress)
  {
    return;
  }

  const uint8_t bucket = min<uint8_t>(10, otaProgress / 10);
  if (bucket == otaLastRenderedProgress)
  {
    return;
  }

  StatusController::getInstance()->otaUpdating(otaProgress);
  otaLastRenderedProgress = bucket;
  otaLastRenderMillis = millis();
}

static void setupArduinoOta()
{
  ArduinoOTA.setHostname("clockwise");

  ArduinoOTA.onStart([]() {
    otaInProgress = true;
    otaProgress = 0;
    otaStartMillis = millis();
    otaLastProgressMillis = millis();
    otaLastProgressLogMillis = millis();
    otaLastRenderMillis = 0;
    otaLastStallLogMillis = millis();
    otaLastRenderedProgress = 255;
    otaLastLoggedProgress = 255;
    prepareForOta();
    StatusController::getInstance()->resetOtaUi();
    renderOtaProgressIfNeeded(true);
#if CW_DEBUG_OTA
    Serial.printf("[OTA] start | freeHeap=%u | rssi=%d\n",
      static_cast<unsigned int>(ESP.getFreeHeap()),
      WiFi.RSSI());
#endif
  });

  ArduinoOTA.onEnd([]() {
    otaInProgress = false;
    otaProgress = 100;
#if CW_DEBUG_OTA
    Serial.printf("[OTA] end | duration=%lu ms | freeHeap=%u | rssi=%d\n",
      millis() - otaStartMillis,
      static_cast<unsigned int>(ESP.getFreeHeap()),
      WiFi.RSSI());
#endif
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    otaProgress = (progress * 100U) / total;
    unsigned long now = millis();
    unsigned long delta = now - otaLastProgressMillis;
    otaLastProgressMillis = now;
    renderOtaProgressBucketIfChanged();

#if CW_DEBUG_OTA
    bool shouldLog = false;
    if (otaLastLoggedProgress == 255)
    {
      shouldLog = true;
    }
    else if (otaProgress >= otaLastLoggedProgress + 5)
    {
      shouldLog = true;
    }
    else if (now - otaLastProgressLogMillis >= 3000)
    {
      shouldLog = true;
    }

    if (shouldLog)
    {
      otaLastProgressLogMillis = now;
      otaLastLoggedProgress = otaProgress;
      Serial.printf("[OTA] progress=%u%% | delta=%lu ms | elapsed=%lu ms | freeHeap=%u | rssi=%d\n",
        static_cast<unsigned int>(otaProgress),
        delta,
        now - otaStartMillis,
        static_cast<unsigned int>(ESP.getFreeHeap()),
        WiFi.RSSI());
    }
#endif
  });

  ArduinoOTA.onError([](ota_error_t error) {
    otaInProgress = false;
#if CW_DEBUG_OTA
    Serial.printf("[OTA] error=%u | elapsed=%lu ms | freeHeap=%u | rssi=%d\n",
      static_cast<unsigned int>(error),
      millis() - otaStartMillis,
      static_cast<unsigned int>(ESP.getFreeHeap()),
      WiFi.RSSI());
#endif
    ESP.restart();
  });

  ArduinoOTA.begin();
#if CW_DEBUG_OTA
  Serial.println("[OTA] ready");
#endif
}

template <typename TClient>
static bool downloadAndApplyHttpOta(TClient &netClient, const String &url)
{
  HTTPClient http;
  http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);

#if CW_DEBUG_WEB_FREEZE
  unsigned long downloadStart = millis();
  Serial.printf("[HTTP OTA] begin: %s\n", url.c_str());
#endif

  if (!http.begin(netClient, url))
  {
    Serial.println("[HTTP OTA] Failed to begin request");
    return false;
  }

  int httpCode = http.GET();
#if CW_DEBUG_WEB_FREEZE
  Serial.printf("[HTTP OTA] GET code: %d\n", httpCode);
#endif
  if (httpCode != HTTP_CODE_OK)
  {
    Serial.printf("[HTTP OTA] GET failed, code=%d\n", httpCode);
    http.end();
    return false;
  }

  int contentLength = http.getSize();
#if CW_DEBUG_WEB_FREEZE
  Serial.printf("[HTTP OTA] content-length: %d\n", contentLength);
#endif
  if (!Update.begin(contentLength > 0 ? contentLength : UPDATE_SIZE_UNKNOWN))
  {
    Serial.println("[HTTP OTA] Update.begin failed");
    http.end();
    return false;
  }

  WiFiClient *stream = http.getStreamPtr();
  uint8_t buffer[1024];
  size_t totalWritten = 0;
  unsigned long lastActivityMillis = millis();
  unsigned long lastProgressLogMillis = millis();

  while (http.connected() && (contentLength > 0 || contentLength == -1))
  {
    size_t availableBytes = stream->available();
    if (availableBytes > 0)
    {
      size_t chunkSize = min(availableBytes, sizeof(buffer));
      int bytesRead = stream->readBytes(buffer, chunkSize);
      if (bytesRead <= 0)
      {
        Serial.println("[HTTP OTA] Stream read failed");
        Update.abort();
        http.end();
        return false;
      }

      size_t bytesWritten = Update.write(buffer, bytesRead);
      if (bytesWritten != static_cast<size_t>(bytesRead))
      {
        Serial.println("[HTTP OTA] Update.write failed");
        Update.abort();
        http.end();
        return false;
      }

      totalWritten += bytesWritten;
      lastActivityMillis = millis();

      if (contentLength > 0)
      {
        contentLength -= bytesRead;
        otaProgress = static_cast<uint8_t>((totalWritten * 100U) / (totalWritten + contentLength));
      }

      renderOtaProgressIfNeeded();

#if CW_DEBUG_WEB_FREEZE
      if (millis() - lastProgressLogMillis >= 500)
      {
        Serial.printf("[HTTP OTA] written=%u progress=%u%%\n",
          static_cast<unsigned int>(totalWritten),
          static_cast<unsigned int>(otaProgress));
        lastProgressLogMillis = millis();
      }
#endif
    }
    else
    {
      if (millis() - lastActivityMillis > 5000)
      {
        Serial.println("[HTTP OTA] Download timeout");
        Update.abort();
        http.end();
        return false;
      }
      delay(1);
    }
  }

  bool success = Update.end() && Update.isFinished();
  if (!success)
  {
    Serial.printf("[HTTP OTA] Update.end failed. error=%u\n", Update.getError());
  }
#if CW_DEBUG_WEB_FREEZE
  else
  {
    Serial.printf("[HTTP OTA] finished successfully in %lu ms, bytes=%u\n",
      millis() - downloadStart,
      static_cast<unsigned int>(totalWritten));
  }
#endif

  http.end();
  return success;
}

static bool performHttpOta(const String &url)
{
  otaInProgress = true;
  otaProgress = 0;
  otaLastRenderMillis = 0;
  otaLastRenderedProgress = 255;
  prepareForOta();
  StatusController::getInstance()->resetOtaUi();
  renderOtaProgressIfNeeded(true);
  Serial.printf("[HTTP OTA] Starting from URL: %s\n", url.c_str());

  bool success = false;
  if (url.startsWith("https://"))
  {
    WiFiClientSecure secureClient;
    secureClient.setInsecure();
    success = downloadAndApplyHttpOta(secureClient, url);
  }
  else
  {
    WiFiClient client;
    success = downloadAndApplyHttpOta(client, url);
  }

  otaProgress = success ? 100 : otaProgress;
  Serial.printf("[HTTP OTA] Result: %s\n", success ? "success" : "failure");
  ESP.restart();
  return success;
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

  if (!dmaDisplayConstructed)
  {
    dma_display = new (dmaDisplayStorage) MatrixPanel_I2S_DMA(mxconfig);
    dmaDisplayConstructed = true;
  }
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
  esp_bt_controller_disable();
  esp_bt_controller_deinit();
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
  if (!clockfaceConstructed)
  {
    clockface = new (clockfaceStorage) Clockface(dma_display);
    clockfaceConstructed = true;
  }


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
  String pendingHttpOtaUrl = ClockwiseWebServer::getInstance()->consumePendingHttpOtaUrl();
  if (pendingHttpOtaUrl.length() > 0 && wifi.isConnected() && !otaInProgress)
  {
#if CW_DEBUG_WEB_FREEZE
    Serial.printf("[HTTP OTA] queued URL consumed: %s\n", pendingHttpOtaUrl.c_str());
#endif
    performHttpOta(pendingHttpOtaUrl);
    return;
  }

  if (wifi.isConnected())
  {
    ArduinoOTA.handle();
  }

  if (otaInProgress)
  {
    renderOtaProgressIfNeeded();
#if CW_DEBUG_OTA
    unsigned long now = millis();
    if (now - otaLastProgressMillis >= 3000 && now - otaLastStallLogMillis >= 3000)
    {
      otaLastStallLogMillis = now;
      Serial.printf("[OTA] waiting... | progress=%u%% | idle=%lu ms | elapsed=%lu ms | freeHeap=%u | rssi=%d\n",
        static_cast<unsigned int>(otaProgress),
        now - otaLastProgressMillis,
        now - otaStartMillis,
        static_cast<unsigned int>(ESP.getFreeHeap()),
        WiFi.RSSI());
    }
#endif
    return;
  }

  wifi.handleImprovWiFi();

  if (wifi.isConnected())
  {
    unsigned long webLoopStart = millis();
    unsigned long httpStart = millis();
    ClockwiseWebServer::getInstance()->handleHttpRequest();
    unsigned long httpDuration = millis() - httpStart;

    unsigned long eztDuration = 0;
    unsigned long now = millis();
    if (now - lastEztEventsMillis >= EZT_EVENTS_INTERVAL_MS)
    {
      lastEztEventsMillis = now;
      unsigned long eztStart = millis();
      ezt::events();
      eztDuration = millis() - eztStart;
    }
#if CW_DEBUG_WEB_FREEZE
    unsigned long webDuration = millis() - webLoopStart;
    if (webDuration >= 100)
    {
      Serial.printf("[PERF] web loop took %lu ms | http=%lu ms | ezt=%lu ms\n",
        webDuration,
        httpDuration,
        eztDuration);
    }
#endif
  }

  if (wifi.connectionSucessfulOnce && clockface != nullptr)
  {
    unsigned long now = millis();
    if (now - lastClockfaceUpdateMillis >= CLOCKFACE_UPDATE_INTERVAL_MS)
    {
      lastClockfaceUpdateMillis = now;
      unsigned long clockfaceStart = millis();
      clockface->update();
#if CW_DEBUG_WEB_FREEZE
      unsigned long clockfaceDuration = millis() - clockfaceStart;
      if (clockfaceDuration >= 40)
      {
        Serial.printf("[PERF] clockface->update() took %lu ms\n", clockfaceDuration);
      }
#endif
    }
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
