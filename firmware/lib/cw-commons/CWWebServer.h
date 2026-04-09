#pragma once

#include <ArduinoJson.h>
#include <Esp.h>
#include <WebServer.h>
#include <WiFi.h>
#include <cstring>
#include <CWPreferences.h>
#include "StatusController.h"
#include "SettingsWebPage.h"
#include "SettingsWebPageScript.h"

#ifndef CW_DEBUG_WEB_FREEZE
  #define CW_DEBUG_WEB_FREEZE 0
#endif

#ifndef CW_WEB_SLOW_REQUEST_MS
  #define CW_WEB_SLOW_REQUEST_MS 25
#endif

#ifndef CLOCKFACE_NAME
  #define CLOCKFACE_NAME "UNKNOWN"
#endif

extern WebServer server;

struct ClockwiseWebServer
{
  static constexpr size_t HTTP_OTA_URL_CAPACITY = 512;
  static constexpr size_t JSON_RESPONSE_CAPACITY = 4096;
  static constexpr unsigned long LDR_CACHE_INTERVAL_MS = 250UL;
  char pendingHttpOtaUrl[HTTP_OTA_URL_CAPACITY] = {0};
  bool pendingHttpOtaQueued = false;
  bool force_restart = false;
  bool routesConfigured = false;
  unsigned long lastLdrReadMillis = 0;
  uint16_t lastLdrValue = 0;
  uint32_t cachedSketchUsed = 0;
  uint32_t cachedSketchFree = 0;
  uint32_t cachedFlashSize = 0;
  bool firmwareUploadInProgress = false;
  bool firmwareUploadSucceeded = false;
  uint8_t firmwareUploadProgress = 0;
  size_t firmwareUploadBytesReceived = 0;
  size_t firmwareUploadExpectedBytes = 0;
  char firmwareUploadError[96] = {0};

  static ClockwiseWebServer *getInstance();
  void startWebServer();
  void stopWebServer();
  void handleHttpRequest();
  bool consumePendingHttpOtaUrl(char *dest, size_t destSize);
  void logSlowRequest(const char *route, unsigned long durationMillis);

private:
  static void formatIpAddress(const IPAddress &ip, char *buffer, size_t bufferSize);
  void configureRoutes();
  void handleRoot();
  void handleFavicon();
  void handleSettingsScript();
  void handleNotFound();
  void restartDevice();
  void queueHttpOta();
  void handleOtaUploadRequest();
  void handleOtaUploadData();
  void sendProgmemHtml(const char *page);
  void sendJsonResponse(JsonDocument &doc, int statusCode = 200, const char *statusText = "OK");
  void sendErrorJson(const char *message, int statusCode = 400);
  void getApiSchema();
  void getApiState();
  bool applySettingsObject(JsonObjectConst data, const char *&error, ClockwiseParams::ApplySettingsReport &report);
  void handleApiSettings();
  void handleApiActions();
  uint16_t getCachedLdrReading();
};
