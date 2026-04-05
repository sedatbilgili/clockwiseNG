#pragma once

#include <ArduinoJson.h>
#include <Esp.h>
#include <WebServer.h>
#include <WiFi.h>
#include <CWPreferences.h>
#include "StatusController.h"
#include "SettingsWebPage.h"
#include "SettingsWebPageScript.h"

#ifndef CW_DEBUG_WEB_FREEZE
  #define CW_DEBUG_WEB_FREEZE 0
#endif

#ifndef CLOCKFACE_NAME
  #define CLOCKFACE_NAME "UNKNOWN"
#endif

WebServer server(80);

struct ClockwiseWebServer
{
  String pendingHttpOtaUrl;
  bool force_restart = false;
  bool routesConfigured = false;

  ClockwiseWebServer()
  {
    pendingHttpOtaUrl.reserve(192);
  }

  static ClockwiseWebServer *getInstance()
  {
    static ClockwiseWebServer base;
    return &base;
  }

  void startWebServer()
  {
    configureRoutes();
    server.begin();
    StatusController::getInstance()->blink_led(100, 3);
  }

  void stopWebServer()
  {
    server.stop();
  }

  void handleHttpRequest()
  {
    if (force_restart)
    {
      StatusController::getInstance()->forceRestart();
      return;
    }

    server.handleClient();
  }

  String consumePendingHttpOtaUrl()
  {
    String url = pendingHttpOtaUrl;
    pendingHttpOtaUrl = "";
#if CW_DEBUG_WEB_FREEZE
    if (url.length() > 0)
    {
      Serial.println("[WEB] HTTP OTA URL dequeued");
    }
#endif
    return url;
  }

private:
  void configureRoutes()
  {
    if (routesConfigured)
    {
      return;
    }

    routesConfigured = true;
    ClockwiseWebServer *self = this;

    server.on("/", HTTP_GET, [self]() { self->handleRoot(); });
    server.on("/favicon.ico", HTTP_GET, [self]() { self->handleFavicon(); });
    server.on("/clockwise-settings.js", HTTP_GET, [self]() { self->handleSettingsScript(); });
    server.on("/info", HTTP_GET, [self]() { self->getDeviceInfo(); });
    server.on("/settings", HTTP_GET, [self]() { self->getSettingsJson(); });
    server.on("/ldr", HTTP_GET, [self]() { self->getLdrReading(); });
    server.on("/settings", HTTP_POST, [self]() { self->updateSettingsJson(); });
    server.on("/get", HTTP_GET, [self]() { self->getCurrentSettings(); });
    server.on("/read", HTTP_GET, [self]() { self->readPin(); });
    server.on("/restart", HTTP_POST, [self]() { self->restartDevice(); });
    server.on("/http-ota", HTTP_POST, [self]() { self->queueHttpOta(); });
    server.on("/set", HTTP_POST, [self]() { self->updatePreference(); });
    server.onNotFound([self]() { self->handleNotFound(); });
  }

  void handleRoot()
  {
    sendProgmemHtml(SETTINGS_PAGE);
  }

  void handleFavicon()
  {
    server.send(204, "text/plain", "");
  }

  void handleSettingsScript()
  {
    server.sendHeader("Cache-Control", "no-store");
    server.send_P(200, "application/javascript; charset=UTF-8", SETTINGS_PAGE_JS);
  }

  void handleNotFound()
  {
    server.send(404, "application/json", "{\"ok\":false,\"error\":\"not found\"}");
  }

  void restartDevice()
  {
    force_restart = true;
    server.send(204, "text/plain", "");
  }

  void queueHttpOta()
  {
    String url = server.arg("url");
    if (url.length() == 0)
    {
      sendErrorJson("missing url");
      return;
    }

    if (!url.startsWith("http://") && !url.startsWith("https://"))
    {
      sendErrorJson("url must start with http:// or https://");
      return;
    }

    pendingHttpOtaUrl = url;
#if CW_DEBUG_WEB_FREEZE
    Serial.printf("[WEB] queued HTTP OTA URL: %s\n", pendingHttpOtaUrl.c_str());
#endif
    server.send(202, "application/json", "{\"ok\":true}");
  }

  void updatePreference()
  {
    ClockwiseParams *params = ClockwiseParams::getInstance();
    params->load();

    if (server.args() == 0)
    {
      sendErrorJson("missing key/value");
      return;
    }

    String key = "";
    String value = "";
    for (uint8_t i = 0; i < server.args(); i++)
    {
      String argName = server.argName(i);
      if (argName == "plain")
      {
        continue;
      }

      key = argName;
      value = server.arg(i);
      break;
    }

    if (key.length() == 0)
    {
      sendErrorJson("missing key/value");
      return;
    }

    if (key == params->PREF_DISPLAY_BRIGHT) {
      params->displayBright = value.toInt();
    } else if (key == params->PREF_WIFI_SSID) {
      params->wifiSsid = value;
    } else if (key == params->PREF_WIFI_PASSWORD) {
      params->wifiPwd = value;
    } else if (key == "autoBright") {
      params->autoBrightMin = value.substring(0, 4).toInt();
      params->autoBrightMax = value.substring(5, 9).toInt();
    } else if (key == params->PREF_USE_24H_FORMAT) {
      params->use24hFormat = (value == "1");
    } else if (key == params->PREF_LDR_PIN) {
      params->ldrPin = value.toInt();
    } else if (key == params->PREF_TIME_ZONE) {
      params->timeZone = value;
    } else if (key == params->PREF_NTP_SERVER) {
      params->ntpServer = value;
    } else if (key == params->PREF_MANUAL_POSIX) {
      params->manualPosix = value;
    } else if (key == params->PREF_DISPLAY_ROTATION) {
      params->displayRotation = value.toInt();
    } else if (key == params->PREF_ANIMATION_ENABLED) {
      params->animationEnabled = (value == "1");
      if (!params->animationEnabled) {
        params->walkingMario = false;
      }
    } else if (key == params->PREF_SCREEN_MODE) {
      params->screenMode = value.toInt();
    } else if (key == params->PREF_CHARACTER) {
      params->characterSelection = value.toInt();
    } else if (key == params->PREF_CLOUD_SPEED) {
      params->cloudSpeed = value.toInt();
    } else if (key == params->PREF_DYNAMIC_SKY) {
      params->dynamicSkyMode = value.toInt();
    } else if (key == params->PREF_WALKING_MARIO) {
      params->walkingMario = params->animationEnabled && (value == "1");
    } else {
      sendErrorJson("unknown key");
      return;
    }

    params->save();
    server.send(204, "text/plain", "");
  }

  void sendProgmemHtml(const char *page)
  {
    server.sendHeader("Cache-Control", "no-store");
    server.send_P(200, "text/html; charset=UTF-8", page);
  }

  void sendJsonResponse(DynamicJsonDocument &doc, int statusCode = 200, const char *statusText = "OK")
  {
    String body;
    body.reserve(measureJson(doc) + 1);
    serializeJson(doc, body);
    server.sendHeader("Cache-Control", "no-store");
    server.send(statusCode, "application/json", body);
    (void)statusText;
  }

  void sendErrorJson(const char *message, int statusCode = 400)
  {
    DynamicJsonDocument doc(192);
    doc["ok"] = false;
    doc["error"] = message;
    sendJsonResponse(doc, statusCode);
  }

  void readPin()
  {
    if (!server.hasArg("pin"))
    {
      sendErrorJson("missing pin");
      return;
    }

    server.sendHeader("Cache-Control", "no-store");
    server.sendHeader("X-pin", String(analogRead(server.arg("pin").toInt())));
    server.send(204, "text/plain", "");
  }

  void getCurrentSettings()
  {
    ClockwiseParams *params = ClockwiseParams::getInstance();
    params->load();

    uint32_t ramFree = ESP.getFreeHeap();
    uint32_t ramTotal = ESP.getHeapSize();
    uint32_t sketchUsed = ESP.getSketchSize();
    uint32_t sketchFree = ESP.getFreeSketchSpace();
    uint32_t flashSize = ESP.getFlashChipSize();

    DynamicJsonDocument doc(1536);
    doc["displayBright"] = params->displayBright;
    doc["autoBrightMin"] = params->autoBrightMin;
    doc["autoBrightMax"] = params->autoBrightMax;
    doc["use24hFormat"] = params->use24hFormat;
    doc["ldrPin"] = params->ldrPin;
    doc["timeZone"] = params->timeZone;
    doc["wifiSsid"] = params->wifiSsid;
    doc["ntpServer"] = params->ntpServer;
    doc["manualPosix"] = params->manualPosix;
    doc["displayRotation"] = params->displayRotation;
    doc["animEnabled"] = params->animationEnabled;
    doc["screenMode"] = params->screenMode;
    doc["charSel"] = params->characterSelection;
    doc["cloudSpeed"] = params->cloudSpeed;
    doc["dynSky"] = params->dynamicSkyMode;
    doc["walkingMario"] = params->walkingMario;
    doc["cw_fw_version"] = CW_FW_VERSION;
    doc["cw_fw_name"] = CW_FW_NAME;
    doc["clockface_name"] = CLOCKFACE_NAME;
    doc["ramFree"] = ramFree;
    doc["ramTotal"] = ramTotal;
    doc["sketchUsed"] = sketchUsed;
    doc["sketchFree"] = sketchFree;
    doc["flashSize"] = flashSize;

    sendJsonResponse(doc);
  }

  void getDeviceInfo()
  {
    DynamicJsonDocument doc(256);
    doc["device"] = "clockwise";
    doc["fwName"] = CW_FW_NAME;
    doc["fwVersion"] = CW_FW_VERSION;
    doc["clockface"] = CLOCKFACE_NAME;
    doc["ip"] = WiFi.localIP().toString();
    doc["port"] = 80;
    sendJsonResponse(doc);
  }

  bool applySettingsObject(JsonObjectConst data, String &error)
  {
    ClockwiseParams *params = ClockwiseParams::getInstance();
    params->load();

    if (data.containsKey("displayBright")) params->displayBright = constrain((int)data["displayBright"], 0, 255);
    if (data.containsKey("use24hFormat")) params->use24hFormat = data["use24hFormat"].as<bool>();
    if (data.containsKey("timeZone")) params->timeZone = data["timeZone"].as<const char *>();
    if (data.containsKey("ntpServer")) params->ntpServer = data["ntpServer"].as<const char *>();
    if (data.containsKey("autoBrightMin")) params->autoBrightMin = constrain((int)data["autoBrightMin"], 0, 4095);
    if (data.containsKey("autoBrightMax")) params->autoBrightMax = constrain((int)data["autoBrightMax"], 0, 4095);
    if (data.containsKey("animationEnabled")) params->animationEnabled = data["animationEnabled"].as<bool>();
    if (data.containsKey("dynamicSkyMode")) params->dynamicSkyMode = constrain((int)data["dynamicSkyMode"], 0, 2);
    if (data.containsKey("screenMode")) params->screenMode = constrain((int)data["screenMode"], 0, 2);
    if (data.containsKey("character")) params->characterSelection = constrain((int)data["character"], 0, 1);
    if (data.containsKey("cloudSpeed")) params->cloudSpeed = constrain((int)data["cloudSpeed"], 1, 30);
    if (data.containsKey("walkingMario")) params->walkingMario = data["walkingMario"].as<bool>();
    if (data.containsKey("ldrPin")) params->ldrPin = constrain((int)data["ldrPin"], 0, 39);
    if (data.containsKey("manualPosix")) params->manualPosix = data["manualPosix"].as<const char *>();
    if (data.containsKey("displayRotation")) params->displayRotation = constrain((int)data["displayRotation"], 0, 3);
    if (data.containsKey("wifiSsid")) params->wifiSsid = data["wifiSsid"].as<const char *>();
    if (data.containsKey("wifiPwd")) params->wifiPwd = data["wifiPwd"].as<const char *>();

    if (params->autoBrightMin >= params->autoBrightMax) {
      error = "autoBrightMin must be smaller than autoBrightMax";
      return false;
    }

    if (!params->animationEnabled) {
      params->walkingMario = false;
    }

    params->save();
    return true;
  }

  void updateSettingsJson()
  {
    String body = server.arg("plain");
    if (body.length() == 0)
    {
      sendErrorJson("missing request body");
      return;
    }

    DynamicJsonDocument doc(1536);
    DeserializationError err = deserializeJson(doc, body);
    if (err)
    {
      sendErrorJson("invalid json");
      return;
    }

    JsonObjectConst data = doc["data"].is<JsonObjectConst>() ? doc["data"].as<JsonObjectConst>() : doc.as<JsonObjectConst>();
    if (data.isNull())
    {
      sendErrorJson("missing data object");
      return;
    }

    String error;
    if (!applySettingsObject(data, error))
    {
      sendErrorJson(error.c_str());
      return;
    }

    getSettingsJson();
  }

  void getLdrReading()
  {
    ClockwiseParams *params = ClockwiseParams::getInstance();
    params->load();

    DynamicJsonDocument doc(192);
    doc["device"] = "clockwise";
    doc["sensor"] = "ldr";
    doc["pin"] = params->ldrPin;
    doc["value"] = analogRead(params->ldrPin);
    sendJsonResponse(doc);
  }

  void getSettingsJson()
  {
    ClockwiseParams *params = ClockwiseParams::getInstance();
    params->load();

    DynamicJsonDocument doc(1024);
    doc["device"] = "clockwise";
    doc["fwName"] = CW_FW_NAME;
    doc["fwVersion"] = CW_FW_VERSION;
    doc["clockface"] = CLOCKFACE_NAME;
    doc["ip"] = WiFi.localIP().toString();
    doc["port"] = 80;
    doc["displayBright"] = params->displayBright;
    doc["autoBrightMin"] = params->autoBrightMin;
    doc["autoBrightMax"] = params->autoBrightMax;
    doc["use24hFormat"] = params->use24hFormat;
    doc["ldrPin"] = params->ldrPin;
    doc["timeZone"] = params->timeZone;
    doc["wifiSsid"] = params->wifiSsid;
    doc["ntpServer"] = params->ntpServer;
    doc["manualPosix"] = params->manualPosix;
    doc["displayRotation"] = params->displayRotation;
    doc["animationEnabled"] = params->animationEnabled;
    doc["screenMode"] = params->screenMode;
    doc["character"] = params->characterSelection;
    doc["cloudSpeed"] = params->cloudSpeed;
    doc["dynamicSkyMode"] = params->dynamicSkyMode;
    doc["walkingMario"] = params->walkingMario;
    sendJsonResponse(doc);
  }
};
