#include "CWWebServer.h"

namespace
{
template <typename Handler>
void runTimedRoute(ClockwiseWebServer *serverInstance, const char *route, Handler handler)
{
  unsigned long start = millis();
  handler();
  serverInstance->logSlowRequest(route, millis() - start);
}

}

WebServer server(80);

ClockwiseWebServer *ClockwiseWebServer::getInstance()
{
  static ClockwiseWebServer base;
  return &base;
}

void ClockwiseWebServer::startWebServer()
{
  configureRoutes();
  cachedSketchUsed = ESP.getSketchSize();
  cachedSketchFree = ESP.getFreeSketchSpace();
  cachedFlashSize = ESP.getFlashChipSize();
  server.begin();
  StatusController::getInstance()->blink_led(100, 3);
}

void ClockwiseWebServer::stopWebServer()
{
  server.stop();
}

void ClockwiseWebServer::handleHttpRequest()
{
  if (force_restart)
  {
    force_restart = false;
    StatusController::getInstance()->forceRestart();
    return;
  }

  unsigned long start = millis();
  server.handleClient();
#if CW_DEBUG_WEB_FREEZE
  unsigned long duration = millis() - start;
  if (duration >= CW_WEB_SLOW_REQUEST_MS)
  {
    Serial.printf("[WEB] handleClient slow: %lu ms\n", duration);
  }
#endif
}

bool ClockwiseWebServer::consumePendingHttpOtaUrl(char *dest, size_t destSize)
{
  if (!pendingHttpOtaQueued || dest == nullptr || destSize == 0)
  {
    return false;
  }

  strncpy(dest, pendingHttpOtaUrl, destSize - 1);
  dest[destSize - 1] = '\0';
  pendingHttpOtaUrl[0] = '\0';
  pendingHttpOtaQueued = false;
#if CW_DEBUG_WEB_FREEZE
  Serial.println("[WEB] HTTP OTA URL dequeued");
#endif
  return true;
}

void ClockwiseWebServer::formatIpAddress(const IPAddress &ip, char *buffer, size_t bufferSize)
{
  if (buffer == nullptr || bufferSize == 0)
  {
    return;
  }

  snprintf(buffer, bufferSize, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
}

void ClockwiseWebServer::configureRoutes()
{
  if (routesConfigured)
  {
    return;
  }

  routesConfigured = true;
  ClockwiseWebServer *self = this;

  server.on("/", HTTP_GET, [self]() { runTimedRoute(self, "/", [self]() { self->handleRoot(); }); });
  server.on("/favicon.ico", HTTP_GET, [self]() { runTimedRoute(self, "/favicon.ico", [self]() { self->handleFavicon(); }); });
  server.on("/clockwise-settings.js", HTTP_GET, [self]() { runTimedRoute(self, "/clockwise-settings.js", [self]() { self->handleSettingsScript(); }); });
  server.on("/api/schema", HTTP_GET, [self]() { runTimedRoute(self, "/api/schema", [self]() { self->getApiSchema(); }); });
  server.on("/api/state", HTTP_GET, [self]() { runTimedRoute(self, "/api/state", [self]() { self->getApiState(); }); });
  server.on("/api/settings", HTTP_POST, [self]() { runTimedRoute(self, "/api/settings", [self]() { self->handleApiSettings(); }); });
  server.on("/api/actions", HTTP_POST, [self]() { runTimedRoute(self, "/api/actions", [self]() { self->handleApiActions(); }); });
  server.on("/api/upload-ota",
    HTTP_POST,
    [self]() { runTimedRoute(self, "/api/upload-ota", [self]() { self->handleOtaUploadRequest(); }); },
    [self]() { self->handleOtaUploadData(); });
  server.onNotFound([self]() { runTimedRoute(self, "404", [self]() { self->handleNotFound(); }); });
}

void ClockwiseWebServer::logSlowRequest(const char *route, unsigned long durationMillis)
{
#if CW_DEBUG_WEB_FREEZE
  if (durationMillis >= CW_WEB_SLOW_REQUEST_MS)
  {
    Serial.printf("[WEB] slow route %s took %lu ms\n", route, durationMillis);
  }
#else
  (void)route;
  (void)durationMillis;
#endif
}

void ClockwiseWebServer::handleRoot()
{
  sendProgmemHtml(SETTINGS_PAGE);
}

void ClockwiseWebServer::handleFavicon()
{
  server.send(204, "text/plain", "");
}

void ClockwiseWebServer::handleSettingsScript()
{
  server.sendHeader("Cache-Control", "no-store");
  server.send_P(200, "application/javascript; charset=UTF-8", SETTINGS_PAGE_JS);
}

void ClockwiseWebServer::handleNotFound()
{
  server.send(404, "application/json", "{\"ok\":false,\"error\":\"not found\"}");
}

void ClockwiseWebServer::restartDevice()
{
  force_restart = true;
  server.send(204, "text/plain", "");
}

void ClockwiseWebServer::sendProgmemHtml(const char *page)
{
  server.sendHeader("Cache-Control", "no-store");
  server.send_P(200, "text/html; charset=UTF-8", page);
}

void ClockwiseWebServer::sendJsonResponse(JsonDocument &doc, int statusCode, const char *statusText)
{
  server.sendHeader("Cache-Control", "no-store");
  static char body[JSON_RESPONSE_CAPACITY];
  size_t length = serializeJson(doc, body, sizeof(body));
  if (length == 0 || length >= sizeof(body))
  {
    server.send(500, "application/json", "{\"ok\":false,\"error\":\"json response too large\"}");
    return;
  }

  server.send(statusCode, "application/json", body);
  (void)statusText;
}

void ClockwiseWebServer::sendErrorJson(const char *message, int statusCode)
{
  StaticJsonDocument<192> doc;
  doc["ok"] = false;
  doc["error"] = message;
  sendJsonResponse(doc, statusCode);
}
