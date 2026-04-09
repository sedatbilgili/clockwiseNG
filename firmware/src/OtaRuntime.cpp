#include "OtaRuntime.h"

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <Update.h>
#include <WiFiClientSecure.h>
#include <cstring>
#include <limits>

#include <CWWebServer.h>
#include <StatusController.h>

#ifndef CW_DEBUG_OTA
#define CW_DEBUG_OTA 1
#endif

namespace
{
volatile bool otaInProgress = false;
volatile uint8_t otaProgress = 0;
volatile bool otaRestartRequested = false;
OtaRuntime::Source otaSource = OtaRuntime::Source::Arduino;
unsigned long otaLastProgressMillis = 0;
unsigned long otaLastProgressLogMillis = 0;
uint8_t otaLastRenderedProgress = 255;
unsigned long otaLastRenderMillis = 0;
unsigned long otaStartMillis = 0;
unsigned long otaLastStallLogMillis = 0;
uint8_t otaLastLoggedProgress = 255;
constexpr unsigned long OTA_RENDER_INTERVAL_MS = 80UL;
constexpr uint8_t OTA_RENDER_BUCKETS = 50U;
constexpr uint8_t OTA_RENDER_STEP_PERCENT = 2U;
constexpr unsigned long HTTP_OTA_DOWNLOAD_TIMEOUT_MS = 5000UL;
constexpr unsigned long HTTP_OTA_SLICE_BUDGET_MS = 12UL;
constexpr size_t HTTP_OTA_MAX_CHUNKS_PER_SLICE = 4;
constexpr size_t HTTP_OTA_BUFFER_SIZE = 256;

enum class HttpOtaPhase : uint8_t
{
  Idle,
  BeginRequest,
  Downloading,
  Finalizing
};

struct HttpOtaState
{
  HttpOtaPhase phase = HttpOtaPhase::Idle;
  HTTPClient http;
  WiFiClient client;
  WiFiClientSecure secureClient;
  WiFiClient *stream = nullptr;
  bool useSecure = false;
  int contentLength = 0;
  int initialContentLength = 0;
  size_t totalWritten = 0;
  unsigned long lastActivityMillis = 0;
  unsigned long lastProgressLogMillis = 0;
  char url[ClockwiseWebServer::HTTP_OTA_URL_CAPACITY] = {0};
  uint8_t buffer[HTTP_OTA_BUFFER_SIZE] = {0};
};

HttpOtaState httpOta;
bool firmwareStreamActive = false;
size_t firmwareStreamBytesExpected = 0;
size_t firmwareStreamBytesReceived = 0;

void prepareForOta()
{
  ClockwiseWebServer::getInstance()->stopWebServer();
  MDNS.end();
}

void resetOtaTracking()
{
  otaProgress = 0;
  otaStartMillis = millis();
  otaLastProgressMillis = millis();
  otaLastProgressLogMillis = millis();
  otaLastRenderMillis = 0;
  otaLastStallLogMillis = millis();
  otaLastRenderedProgress = 255;
  otaLastLoggedProgress = 255;
}

void renderOtaProgressIfNeeded(bool force = false)
{
  if (!otaInProgress)
  {
    return;
  }

  const uint8_t bucket = min<uint8_t>(OTA_RENDER_BUCKETS, otaProgress / OTA_RENDER_STEP_PERCENT);
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

void renderOtaProgressBucketIfChanged()
{
  if (!otaInProgress)
  {
    return;
  }

  const uint8_t bucket = min<uint8_t>(OTA_RENDER_BUCKETS, otaProgress / OTA_RENDER_STEP_PERCENT);
  if (bucket == otaLastRenderedProgress)
  {
    return;
  }

  StatusController::getInstance()->otaUpdating(otaProgress);
#if CW_DEBUG_OTA
  Serial.printf("[HTTP OTA] screen bucket=%u progress=%u%%\n",
    static_cast<unsigned int>(bucket),
    static_cast<unsigned int>(otaProgress));
#endif
  otaLastRenderedProgress = bucket;
  otaLastRenderMillis = millis();
}

bool hasHttpScheme(const char *url)
{
  if (url == nullptr)
  {
    return false;
  }

  return strncmp(url, "http://", 7) == 0 || strncmp(url, "https://", 8) == 0;
}

void resetHttpOtaState()
{
  httpOta.http.end();
  httpOta.phase = HttpOtaPhase::Idle;
  httpOta.stream = nullptr;
  httpOta.useSecure = false;
  httpOta.contentLength = 0;
  httpOta.initialContentLength = 0;
  httpOta.totalWritten = 0;
  httpOta.lastActivityMillis = 0;
  httpOta.lastProgressLogMillis = 0;
  httpOta.url[0] = '\0';
}

void resetFirmwareStreamState()
{
  firmwareStreamActive = false;
  firmwareStreamBytesExpected = 0;
  firmwareStreamBytesReceived = 0;
}

void failHttpOtaSession(const char *reason)
{
  if (reason != nullptr)
  {
    Serial.printf("[HTTP OTA] %s\n", reason);
  }
  OtaRuntime::abortFirmwareStream();
  resetHttpOtaState();
  OtaRuntime::failSession();
}

bool beginHttpOtaRequest()
{
  httpOta.http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);

#if CW_DEBUG_WEB_FREEZE
  Serial.printf("[HTTP OTA] begin: %s\n", httpOta.url);
#endif

  httpOta.useSecure = strncmp(httpOta.url, "https://", 8) == 0;
  if (httpOta.useSecure)
  {
    httpOta.secureClient.setInsecure();
  }
  WiFiClient *netClient = httpOta.useSecure
    ? static_cast<WiFiClient *>(&httpOta.secureClient)
    : &httpOta.client;

  if (!httpOta.http.begin(*netClient, httpOta.url))
  {
    Serial.println("[HTTP OTA] Failed to begin request");
    return false;
  }

  int httpCode = httpOta.http.GET();
#if CW_DEBUG_WEB_FREEZE
  Serial.printf("[HTTP OTA] GET code: %d\n", httpCode);
#endif
  if (httpCode != HTTP_CODE_OK)
  {
    Serial.printf("[HTTP OTA] GET failed, code=%d\n", httpCode);
    return false;
  }

  httpOta.contentLength = httpOta.http.getSize();
  httpOta.initialContentLength = httpOta.contentLength;
#if CW_DEBUG_WEB_FREEZE
  Serial.printf("[HTTP OTA] content-length: %d\n", httpOta.contentLength);
#endif
  if (!OtaRuntime::beginFirmwareStream(httpOta.contentLength > 0 ? static_cast<size_t>(httpOta.contentLength) : 0))
  {
    Serial.println("[HTTP OTA] Update.begin failed");
    return false;
  }

  httpOta.totalWritten = 0;
  httpOta.lastActivityMillis = millis();
  httpOta.lastProgressLogMillis = millis();
  httpOta.stream = httpOta.http.getStreamPtr();
  httpOta.phase = HttpOtaPhase::Downloading;
  return true;
}

void handleHttpOtaDownloadSlice()
{
  if (httpOta.phase != HttpOtaPhase::Downloading || httpOta.stream == nullptr)
  {
    return;
  }

  const unsigned long sliceStart = millis();
  size_t chunksProcessed = 0;
  while (chunksProcessed < HTTP_OTA_MAX_CHUNKS_PER_SLICE
    && (millis() - sliceStart) < HTTP_OTA_SLICE_BUDGET_MS)
  {
    size_t availableBytes = httpOta.stream->available();
    if (availableBytes > 0)
    {
      size_t chunkSize = min(availableBytes, sizeof(httpOta.buffer));
      int bytesRead = httpOta.stream->readBytes(httpOta.buffer, chunkSize);
      if (bytesRead <= 0)
      {
        failHttpOtaSession("Stream read failed");
        return;
      }

      if (!OtaRuntime::writeFirmwareStreamChunk(httpOta.buffer, static_cast<size_t>(bytesRead)))
      {
        failHttpOtaSession("Update.write failed");
        return;
      }

      httpOta.totalWritten += static_cast<size_t>(bytesRead);
      httpOta.lastActivityMillis = millis();
      if (httpOta.contentLength > 0)
      {
        httpOta.contentLength -= bytesRead;
        if (httpOta.contentLength < 0)
        {
          httpOta.contentLength = 0;
        }
      }
      otaLastProgressMillis = millis();

#if CW_DEBUG_WEB_FREEZE
      if (millis() - httpOta.lastProgressLogMillis >= 500)
      {
        Serial.printf("[HTTP OTA] written=%u progress=%u%%\n",
          static_cast<unsigned int>(httpOta.totalWritten),
          static_cast<unsigned int>(otaProgress));
        httpOta.lastProgressLogMillis = millis();
      }
#endif
      chunksProcessed++;
    }
    else
    {
      break;
    }
  }

  const bool downloadCompleted = (httpOta.initialContentLength > 0 && httpOta.contentLength == 0)
    || (httpOta.initialContentLength <= 0 && !httpOta.http.connected());
  if (downloadCompleted)
  {
    httpOta.phase = HttpOtaPhase::Finalizing;
    return;
  }

  if (millis() - httpOta.lastActivityMillis > HTTP_OTA_DOWNLOAD_TIMEOUT_MS)
  {
    failHttpOtaSession("Download timeout");
  }
}

void finalizeHttpOtaSession()
{
  if (httpOta.phase != HttpOtaPhase::Finalizing)
  {
    return;
  }

  bool success = OtaRuntime::finalizeFirmwareStream();
#if CW_DEBUG_OTA
  Serial.printf("[HTTP OTA] finalize success=%u finished=%u error=%u\n",
    static_cast<unsigned int>(success ? 1 : 0),
    static_cast<unsigned int>(success ? 1 : 0),
    static_cast<unsigned int>(Update.getError()));
#endif
  if (!success)
  {
    Serial.printf("[HTTP OTA] Update.end failed. error=%u\n", Update.getError());
    failHttpOtaSession("Finalize failed");
  }
  else
  {
    otaProgress = 100;
    renderOtaProgressIfNeeded(true);
#if CW_DEBUG_WEB_FREEZE
    Serial.printf("[HTTP OTA] finished successfully in %lu ms, bytes=%u\n",
      millis() - otaStartMillis,
      static_cast<unsigned int>(httpOta.totalWritten));
#endif
    resetHttpOtaState();
    OtaRuntime::completeSession(true);
  }
}
} // namespace

namespace OtaRuntime
{
void beginSession(Source source)
{
  otaSource = source;
  otaInProgress = true;
  otaRestartRequested = false;
  resetOtaTracking();
  prepareForOta();
  StatusController::getInstance()->resetOtaUi();
  StatusController::getInstance()->otaUpdating(0);
  otaLastRenderedProgress = 0;
  otaLastRenderMillis = millis();
}

void updateProgress(unsigned int progress, unsigned int total)
{
  if (!otaInProgress)
  {
    return;
  }

  if (total > 0)
  {
    otaProgress = static_cast<uint8_t>(min<unsigned int>(99U, (progress * 100U) / total));
  }
  else
  {
    otaProgress = min<uint8_t>(99, otaProgress + 1);
  }

  otaLastProgressMillis = millis();
  renderOtaProgressBucketIfChanged();
}

void failSession()
{
  abortFirmwareStream();
  if (otaSource == Source::HttpUrl)
  {
    resetHttpOtaState();
  }
  otaInProgress = false;
  otaRestartRequested = false;
}

void completeSession(bool requestRestart)
{
  resetFirmwareStreamState();
  if (otaSource == Source::HttpUrl)
  {
    resetHttpOtaState();
  }
  otaProgress = 100;
  renderOtaProgressIfNeeded(true);
  otaInProgress = false;
  otaRestartRequested = requestRestart;
}

void queueRestart()
{
  otaRestartRequested = true;
}

bool consumeRestartRequest()
{
  if (!otaRestartRequested)
  {
    return false;
  }

  otaRestartRequested = false;
  return true;
}

void setupArduinoOta()
{
  ArduinoOTA.setHostname("clockwise");

  ArduinoOTA.onStart([]() {
    beginSession(Source::Arduino);
    renderOtaProgressIfNeeded(true);
#if CW_DEBUG_OTA
    Serial.printf("[OTA] start | freeHeap=%u | rssi=%d\n",
      static_cast<unsigned int>(ESP.getFreeHeap()),
      WiFi.RSSI());
#endif
  });

  ArduinoOTA.onEnd([]() {
    completeSession(true);
#if CW_DEBUG_OTA
    Serial.printf("[OTA] end | duration=%lu ms | freeHeap=%u | rssi=%d\n",
      millis() - otaStartMillis,
      static_cast<unsigned int>(ESP.getFreeHeap()),
      WiFi.RSSI());
#endif
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    const unsigned long now = millis();
    const unsigned long delta = now - otaLastProgressMillis;
    updateProgress(progress, total);

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
    failSession();
#if CW_DEBUG_OTA
    Serial.printf("[OTA] error=%u | elapsed=%lu ms | freeHeap=%u | rssi=%d\n",
      static_cast<unsigned int>(error),
      millis() - otaStartMillis,
      static_cast<unsigned int>(ESP.getFreeHeap()),
      WiFi.RSSI());
#endif
    queueRestart();
  });

  ArduinoOTA.begin();
#if CW_DEBUG_OTA
  Serial.println("[OTA] ready");
#endif
}

void handleArduinoOta()
{
  ArduinoOTA.handle();
}

bool isInProgress()
{
  return otaInProgress;
}

void handleInProgressLoop()
{
  if (otaSource == Source::HttpUrl)
  {
    if (httpOta.phase == HttpOtaPhase::BeginRequest)
    {
      if (!beginHttpOtaRequest())
      {
        failHttpOtaSession("Failed to initialize HTTP OTA request");
        return;
      }
    }

    if (httpOta.phase == HttpOtaPhase::Downloading)
    {
      handleHttpOtaDownloadSlice();
    }

    if (httpOta.phase == HttpOtaPhase::Finalizing)
    {
      finalizeHttpOtaSession();
    }
  }

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
}

bool startHttpOta(const char *url)
{
  if (otaInProgress || url == nullptr || url[0] == '\0')
  {
    return false;
  }

  if (!hasHttpScheme(url))
  {
    return false;
  }

  const size_t urlLength = strnlen(url, sizeof(httpOta.url));
  if (urlLength == 0 || urlLength >= sizeof(httpOta.url))
  {
    return false;
  }

  resetHttpOtaState();
  beginSession(Source::HttpUrl);
  strncpy(httpOta.url, url, sizeof(httpOta.url) - 1);
  httpOta.url[sizeof(httpOta.url) - 1] = '\0';
  httpOta.phase = HttpOtaPhase::BeginRequest;
#if CW_DEBUG_OTA
  Serial.printf("[HTTP OTA] queued non-blocking transfer: %s\n", httpOta.url);
#endif
  return true;
}

bool beginFirmwareStream(size_t expectedSize)
{
  if (!otaInProgress || firmwareStreamActive)
  {
    return false;
  }

  if (!Update.begin(expectedSize > 0 ? expectedSize : UPDATE_SIZE_UNKNOWN))
  {
    return false;
  }

  firmwareStreamActive = true;
  firmwareStreamBytesExpected = expectedSize;
  firmwareStreamBytesReceived = 0;
  return true;
}

bool writeFirmwareStreamChunk(uint8_t *data, size_t size)
{
  if (!firmwareStreamActive || data == nullptr || size == 0)
  {
    return false;
  }

  size_t bytesWritten = Update.write(data, size);
  if (bytesWritten != size)
  {
    return false;
  }

  firmwareStreamBytesReceived += bytesWritten;
  const unsigned int progressValue = firmwareStreamBytesReceived > static_cast<size_t>(std::numeric_limits<unsigned int>::max())
    ? std::numeric_limits<unsigned int>::max()
    : static_cast<unsigned int>(firmwareStreamBytesReceived);
  const unsigned int totalValue = firmwareStreamBytesExpected > static_cast<size_t>(std::numeric_limits<unsigned int>::max())
    ? std::numeric_limits<unsigned int>::max()
    : static_cast<unsigned int>(firmwareStreamBytesExpected);

  updateProgress(progressValue, totalValue);
  return true;
}

bool finalizeFirmwareStream()
{
  if (!firmwareStreamActive)
  {
    return false;
  }

  const bool success = Update.end(true) && Update.isFinished();
  resetFirmwareStreamState();
  return success;
}

void abortFirmwareStream()
{
  if (!firmwareStreamActive)
  {
    return;
  }

  Update.abort();
  resetFirmwareStreamState();
}
} // namespace OtaRuntime
