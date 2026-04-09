#include "AppRuntime.h"

#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#include "Clockface.h"
#include "DisplayRuntime.h"
#include "OtaRuntime.h"
#include <CWDateTime.h>
#include <CWPreferences.h>
#include <CWWebServer.h>
#include <StatusController.h>
#include <WiFiController.h>

#define ESP32_LED_BUILTIN 2

namespace
{
enum class LoopStage : uint8_t
{
  Startup,
  OtaQueued,
  OtaInProgress,
  Running
};

struct AppState
{
  MatrixPanel_I2S_DMA *display = nullptr;
  Clockface *clockface = nullptr;
  WiFiController wifi;
  CWDateTime dateTime;
  bool startupSequencePending = false;
  bool runtimeInitialized = false;
  unsigned long startupSequenceDeadlineMillis = 0;
  unsigned long lastClockfaceUpdateMillis = 0;
  unsigned long lastEztEventsMillis = 0;
};

constexpr unsigned long CLOCKFACE_UPDATE_INTERVAL_MS = 33UL;
constexpr unsigned long EZT_EVENTS_INTERVAL_MS = 100UL;
AppState app;

void processQueuedOtaRestart()
{
  if (!OtaRuntime::consumeRestartRequest())
  {
    return;
  }

  // Use a single restart path to keep behavior deterministic and non-blocking.
  StatusController::getInstance()->forceRestart();
}

void completeStartupSequence(AppState &state)
{
  StatusController::getInstance()->wifiConnecting();
  if (state.wifi.begin())
  {
    OtaRuntime::setupArduinoOta();
    StatusController::getInstance()->ntpConnecting();
    state.dateTime.begin(ClockwiseParams::getInstance()->timeZone,
      ClockwiseParams::getInstance()->use24hFormat,
      ClockwiseParams::getInstance()->ntpServer,
      ClockwiseParams::getInstance()->manualPosix);
    if (state.clockface != nullptr)
    {
      state.clockface->setup(&state.dateTime);
    }
  }
  state.runtimeInitialized = true;
  state.startupSequencePending = false;
}

bool processStartupSequence(AppState &state)
{
  if (!state.startupSequencePending)
  {
    return true;
  }

  if (millis() < state.startupSequenceDeadlineMillis)
  {
    return false;
  }

  completeStartupSequence(state);
  return state.runtimeInitialized;
}

bool consumePendingHttpOtaUrl(char *pendingHttpOtaUrl, size_t pendingHttpOtaUrlSize)
{
  return ClockwiseWebServer::getInstance()->consumePendingHttpOtaUrl(
    pendingHttpOtaUrl,
    pendingHttpOtaUrlSize);
}

bool processPendingHttpOta(bool wifiConnected, const char *pendingHttpOtaUrl, bool hasPendingHttpOtaUrl)
{
  if (!hasPendingHttpOtaUrl || !wifiConnected || OtaRuntime::isInProgress())
  {
    return false;
  }

#if CW_DEBUG_WEB_FREEZE
  Serial.printf("[HTTP OTA] queued URL consumed: %s\n", pendingHttpOtaUrl);
#endif
  const bool started = OtaRuntime::startHttpOta(pendingHttpOtaUrl);
  Serial.printf("[HTTP OTA] startHttpOta returned %s\n", started ? "true" : "false");
  return started;
}

bool processOtaLoop(bool wifiConnected)
{
  if (wifiConnected)
  {
    OtaRuntime::handleArduinoOta();
  }

  if (!OtaRuntime::isInProgress())
  {
    return false;
  }

  OtaRuntime::handleInProgressLoop();
  return true;
}

void handleScheduledEzTimeEvents(AppState &state, unsigned long now, unsigned long &eztDuration)
{
  if (now - state.lastEztEventsMillis < EZT_EVENTS_INTERVAL_MS)
  {
    return;
  }

  state.lastEztEventsMillis = now;
  unsigned long eztStart = millis();
  ezt::events();
  eztDuration = millis() - eztStart;
}

void processWebLoop(AppState &state, bool wifiConnected)
{
  if (!wifiConnected)
  {
    return;
  }

  unsigned long webLoopStart = millis();
  unsigned long httpStart = millis();
  ClockwiseWebServer::getInstance()->handleHttpRequest();
  unsigned long httpDuration = millis() - httpStart;

  unsigned long eztDuration = 0;
  handleScheduledEzTimeEvents(state, millis(), eztDuration);

#if CW_DEBUG_WEB_FREEZE
  unsigned long webDuration = millis() - webLoopStart;
  if (webDuration >= 100)
  {
    Serial.printf("[PERF] web loop took %lu ms | http=%lu ms | ezt=%lu ms\n",
      webDuration,
      httpDuration,
      eztDuration);
  }
#else
  (void)httpDuration;
#endif
}

void updateClockfaceIfNeeded(AppState &state)
{
  if (!state.wifi.connectionSucessfulOnce || state.clockface == nullptr)
  {
    return;
  }

  unsigned long now = millis();
  if (now - state.lastClockfaceUpdateMillis < CLOCKFACE_UPDATE_INTERVAL_MS)
  {
    return;
  }

  state.lastClockfaceUpdateMillis = now;
  StatusController::getInstance()->clearStatusAnimation();
  unsigned long clockfaceStart = millis();
  state.clockface->update();

#if CW_DEBUG_WEB_FREEZE
  unsigned long clockfaceDuration = millis() - clockfaceStart;
  if (clockfaceDuration >= 40)
  {
    Serial.printf("[PERF] clockface->update() took %lu ms\n", clockfaceDuration);
  }
#else
  (void)clockfaceStart;
#endif
}

void processDisplayLoop(AppState &state)
{
  DisplayRuntime::updateScreenMode(state.display);
  DisplayRuntime::handlePanelButton();
}

LoopStage determineLoopStage(bool wifiConnected, const char *pendingHttpOtaUrl, bool hasPendingHttpOtaUrl)
{
  if (processPendingHttpOta(wifiConnected, pendingHttpOtaUrl, hasPendingHttpOtaUrl))
  {
    return LoopStage::OtaQueued;
  }

  if (processOtaLoop(wifiConnected))
  {
    return LoopStage::OtaInProgress;
  }

  return LoopStage::Running;
}
} // namespace

namespace AppRuntime
{
void setup()
{
  Serial.begin(115200);
  pinMode(ESP32_LED_BUILTIN, OUTPUT);

  StatusController::getInstance()->blink_led(5, 100);

  ClockwiseParams::getInstance()->load();
  DisplayRuntime::initialize(
    ClockwiseParams::getInstance()->screenMode,
    ClockwiseParams::getInstance()->ldrPin);
  app.display = DisplayRuntime::createDisplay(
    ClockwiseParams::getInstance()->displayBright,
    ClockwiseParams::getInstance()->displayRotation);
  app.clockface = DisplayRuntime::createClockface(app.display);

  StatusController::getInstance()->clockwiseLogo();
  app.startupSequencePending = true;
  app.runtimeInitialized = false;
  app.startupSequenceDeadlineMillis = millis() + 1000UL;
}

void loop()
{
  StatusController::getInstance()->process();
  processQueuedOtaRestart();

  LoopStage stage = LoopStage::Startup;
  bool wifiConnected = false;
  if (processStartupSequence(app) && app.runtimeInitialized)
  {
    char pendingHttpOtaUrl[ClockwiseWebServer::HTTP_OTA_URL_CAPACITY] = {0};
    bool hasPendingHttpOtaUrl = consumePendingHttpOtaUrl(
      pendingHttpOtaUrl,
      sizeof(pendingHttpOtaUrl));
    app.wifi.handle();
    wifiConnected = app.wifi.isConnected();
    stage = determineLoopStage(wifiConnected, pendingHttpOtaUrl, hasPendingHttpOtaUrl);
  }

  switch (stage)
  {
    case LoopStage::Startup:
      // Startup sequence is intentionally exclusive: no normal runtime tasks yet.
      break;

    case LoopStage::OtaQueued:
      // OTA transfer was started this iteration.
      break;

    case LoopStage::OtaInProgress:
      // OTA is active; skip web and clockface workload.
      break;

    case LoopStage::Running:
      processWebLoop(app, wifiConnected);
      updateClockfaceIfNeeded(app);
      processDisplayLoop(app);
      break;
  }
}
} // namespace AppRuntime
