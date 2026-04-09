#include "CWWebServer.h"

namespace
{
#if CW_DEBUG_WEB_FREEZE
void logSlowStateStep(const char *step, unsigned long durationMillis)
{
  if (durationMillis >= CW_WEB_SLOW_REQUEST_MS)
  {
    Serial.printf("[WEB] /api/state step %s took %lu ms\n", step, durationMillis);
  }
}
#else
void logSlowStateStep(const char *step, unsigned long durationMillis)
{
  (void)step;
  (void)durationMillis;
}
#endif
}

void ClockwiseWebServer::getApiSchema()
{
  static StaticJsonDocument<4096> doc;
  doc.clear();
  doc["device"] = "clockwise";
  ClockwiseParams::getInstance()->appendSettingsSchemaJson(doc);
  sendJsonResponse(doc);
}

void ClockwiseWebServer::getApiState()
{
  unsigned long stepStart = millis();
  ClockwiseParams *params = ClockwiseParams::getInstance();
  params->load();
  logSlowStateStep("params->load", millis() - stepStart);

  stepStart = millis();
  uint32_t ramFree = ESP.getFreeHeap();
  uint32_t ramTotal = ESP.getHeapSize();
  logSlowStateStep("esp-metrics", millis() - stepStart);

  stepStart = millis();
  char ipBuffer[16];
  formatIpAddress(WiFi.localIP(), ipBuffer, sizeof(ipBuffer));
  logSlowStateStep("WiFi.localIP", millis() - stepStart);

  stepStart = millis();
  StaticJsonDocument<1536> doc;
  doc["device"] = "clockwise";
  doc["fwName"] = CW_FW_NAME;
  doc["fwVersion"] = CW_FW_VERSION;
  doc["clockface"] = CLOCKFACE_NAME;
  doc["schemaVersion"] = 1;
  doc["ip"] = ipBuffer;
  doc["port"] = 80;
  params->appendSettingsJson(doc);
  doc["ldrValue"] = getCachedLdrReading();
  doc["ramFree"] = ramFree;
  doc["ramTotal"] = ramTotal;
  doc["sketchUsed"] = cachedSketchUsed;
  doc["sketchFree"] = cachedSketchFree;
  doc["flashSize"] = cachedFlashSize;
  logSlowStateStep("build-json", millis() - stepStart);

  stepStart = millis();
  sendJsonResponse(doc);
  logSlowStateStep("send-json", millis() - stepStart);
}

uint16_t ClockwiseWebServer::getCachedLdrReading()
{
  unsigned long now = millis();
  if (now - lastLdrReadMillis >= LDR_CACHE_INTERVAL_MS)
  {
    lastLdrReadMillis = now;
    lastLdrValue = analogRead(ClockwiseParams::getInstance()->ldrPin);
  }

  return lastLdrValue;
}
