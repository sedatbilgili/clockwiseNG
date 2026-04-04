#pragma once

#include <ArduinoJson.h>
#include <Esp.h>
#include <WiFi.h>
#include <CWPreferences.h>
#include "StatusController.h"
#include "SettingsWebPage.h"

#ifndef CLOCKFACE_NAME
  #define CLOCKFACE_NAME "UNKNOWN"
#endif

WiFiServer server(80);

struct ClockwiseWebServer
{
  String httpBuffer;
  bool force_restart;
  const char* HEADER_TEMPLATE_D = "X-%s: %d\r\n";
  const char* HEADER_TEMPLATE_S = "X-%s: %s\r\n";
 
  static ClockwiseWebServer *getInstance()
  {
    static ClockwiseWebServer base;
    return &base;
  }

  void startWebServer()
  {
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
      StatusController::getInstance()->forceRestart();


    WiFiClient client = server.available();
    if (client)
    {
      StatusController::getInstance()->blink_led(100, 1);

      while (client.connected())
      {
        if (client.available())
        {
          char c = client.read();
          httpBuffer.concat(c);

          if (c == '\n')
          {
            uint8_t method_pos = httpBuffer.indexOf(' ');
            uint8_t path_pos = httpBuffer.indexOf(' ', method_pos + 1);

            String method = httpBuffer.substring(0, method_pos);
            String path = httpBuffer.substring(method_pos + 1, path_pos);
            String key = "";
            String value = "";

            if (path.indexOf('?') > 0)
            {
              key = path.substring(path.indexOf('?') + 1, path.indexOf('='));
              value = path.substring(path.indexOf('=') + 1);
              path = path.substring(0, path.indexOf('?'));
            }

            processRequest(client, method, path, key, value);
            httpBuffer = "";
            break;
          }
        }
      }
      delay(1);
      client.stop();
    }
  }

  void processRequest(WiFiClient client, String method, String path, String key, String value)
  {
    if (method == "GET" && path == "/") {
      client.println("HTTP/1.0 200 OK");
      client.println("Content-Type: text/html");
      client.println();
      client.println(SETTINGS_PAGE);
    } else if (method == "GET" && path == "/info") {
      getDeviceInfo(client);
    } else if (method == "GET" && path == "/settings") {
      getSettingsJson(client);
    } else if (method == "GET" && path == "/ldr") {
      getLdrReading(client);
    } else if (method == "POST" && path == "/settings") {
      updateSettingsJson(client);
    } else if (method == "GET" && path == "/get") {
      getCurrentSettings(client);
    } else if (method == "GET" && path == "/read") {
      if (key == "pin") {
        readPin(client, key, value.toInt());
      }
    } else if (method == "POST" && path == "/restart") {
      client.println("HTTP/1.0 204 No Content");
      force_restart = true;
    } else if (method == "POST" && path == "/set") {
      ClockwiseParams::getInstance()->load();
      //a baby seal has died due this ifs
      if (key == ClockwiseParams::getInstance()->PREF_DISPLAY_BRIGHT) {
        ClockwiseParams::getInstance()->displayBright = value.toInt();
      } else if (key == ClockwiseParams::getInstance()->PREF_WIFI_SSID) {
        ClockwiseParams::getInstance()->wifiSsid = value;
      } else if (key == ClockwiseParams::getInstance()->PREF_WIFI_PASSWORD) {
        ClockwiseParams::getInstance()->wifiPwd = value;
      } else if (key == "autoBright") {   //autoBright=0010,0800
        ClockwiseParams::getInstance()->autoBrightMin = value.substring(0,4).toInt();
        ClockwiseParams::getInstance()->autoBrightMax = value.substring(5,9).toInt();
      } else if (key == ClockwiseParams::getInstance()->PREF_USE_24H_FORMAT) {
        ClockwiseParams::getInstance()->use24hFormat = (value == "1");
      } else if (key == ClockwiseParams::getInstance()->PREF_LDR_PIN) {
        ClockwiseParams::getInstance()->ldrPin = value.toInt();
      } else if (key == ClockwiseParams::getInstance()->PREF_TIME_ZONE) {
        ClockwiseParams::getInstance()->timeZone = value;
      } else if (key == ClockwiseParams::getInstance()->PREF_NTP_SERVER) {
        ClockwiseParams::getInstance()->ntpServer = value;
      } else if (key == ClockwiseParams::getInstance()->PREF_CANVAS_FILE) {
        ClockwiseParams::getInstance()->canvasFile = value;
      } else if (key == ClockwiseParams::getInstance()->PREF_CANVAS_SERVER) {
        ClockwiseParams::getInstance()->canvasServer = value;
      } else if (key == ClockwiseParams::getInstance()->PREF_MANUAL_POSIX) {
        ClockwiseParams::getInstance()->manualPosix = value;
      } else if (key == ClockwiseParams::getInstance()->PREF_DISPLAY_ROTATION) {
        ClockwiseParams::getInstance()->displayRotation = value.toInt();
      } else if (key == ClockwiseParams::getInstance()->PREF_ANIMATION_ENABLED) {
        ClockwiseParams::getInstance()->animationEnabled = (value == "1");
        Serial.printf("[Settings] animationEnabled set request: key=%s value=%s parsed=%d\n",
          key.c_str(), value.c_str(), ClockwiseParams::getInstance()->animationEnabled);
      } else if (key == ClockwiseParams::getInstance()->PREF_SCREEN_MODE) {
        ClockwiseParams::getInstance()->screenMode = value.toInt();
      } else if (key == ClockwiseParams::getInstance()->PREF_CHARACTER) {
        ClockwiseParams::getInstance()->characterSelection = value.toInt();
      } else if (key == ClockwiseParams::getInstance()->PREF_CLOUD_SPEED) {
        ClockwiseParams::getInstance()->cloudSpeed = value.toInt();
      } else if (key == ClockwiseParams::getInstance()->PREF_DYNAMIC_SKY) {
        ClockwiseParams::getInstance()->dynamicSkyMode = value.toInt();
      }
      ClockwiseParams::getInstance()->save();
      if (key == ClockwiseParams::getInstance()->PREF_ANIMATION_ENABLED) {
        ClockwiseParams::getInstance()->load();
        Serial.printf("[Settings] animationEnabled after save/load: %d\n",
          ClockwiseParams::getInstance()->animationEnabled);
      }
      client.println("HTTP/1.0 204 No Content");
    }
  }



  void readPin(WiFiClient client, String key, uint16_t pin) {
    ClockwiseParams::getInstance()->load();

    client.println("HTTP/1.0 204 No Content");
    client.printf(HEADER_TEMPLATE_D, key, analogRead(pin));
    
    client.println();
  }


  void getCurrentSettings(WiFiClient client) {
    ClockwiseParams::getInstance()->load();

    uint32_t ramFree = ESP.getFreeHeap();
    uint32_t ramTotal = ESP.getHeapSize();
    uint32_t sketchUsed = ESP.getSketchSize();
    uint32_t sketchFree = ESP.getFreeSketchSpace();
    uint32_t flashSize = ESP.getFlashChipSize();

    client.println("HTTP/1.0 204 No Content");

    client.printf(HEADER_TEMPLATE_D, ClockwiseParams::getInstance()->PREF_DISPLAY_BRIGHT, ClockwiseParams::getInstance()->displayBright);
    client.printf(HEADER_TEMPLATE_D, ClockwiseParams::getInstance()->PREF_DISPLAY_ABC_MIN, ClockwiseParams::getInstance()->autoBrightMin);
    client.printf(HEADER_TEMPLATE_D, ClockwiseParams::getInstance()->PREF_DISPLAY_ABC_MAX, ClockwiseParams::getInstance()->autoBrightMax);
    client.printf(HEADER_TEMPLATE_D, ClockwiseParams::getInstance()->PREF_USE_24H_FORMAT, ClockwiseParams::getInstance()->use24hFormat);
    client.printf(HEADER_TEMPLATE_D, ClockwiseParams::getInstance()->PREF_LDR_PIN, ClockwiseParams::getInstance()->ldrPin);    
    client.printf(HEADER_TEMPLATE_S, ClockwiseParams::getInstance()->PREF_TIME_ZONE, ClockwiseParams::getInstance()->timeZone.c_str());
    client.printf(HEADER_TEMPLATE_S, ClockwiseParams::getInstance()->PREF_WIFI_SSID, ClockwiseParams::getInstance()->wifiSsid.c_str());
    client.printf(HEADER_TEMPLATE_S, ClockwiseParams::getInstance()->PREF_NTP_SERVER, ClockwiseParams::getInstance()->ntpServer.c_str());
    client.printf(HEADER_TEMPLATE_S, ClockwiseParams::getInstance()->PREF_CANVAS_FILE, ClockwiseParams::getInstance()->canvasFile.c_str());
    client.printf(HEADER_TEMPLATE_S, ClockwiseParams::getInstance()->PREF_CANVAS_SERVER, ClockwiseParams::getInstance()->canvasServer.c_str());
    client.printf(HEADER_TEMPLATE_S, ClockwiseParams::getInstance()->PREF_MANUAL_POSIX, ClockwiseParams::getInstance()->manualPosix.c_str());
    client.printf(HEADER_TEMPLATE_D, ClockwiseParams::getInstance()->PREF_DISPLAY_ROTATION, ClockwiseParams::getInstance()->displayRotation);
    client.printf(HEADER_TEMPLATE_D, ClockwiseParams::getInstance()->PREF_ANIMATION_ENABLED, ClockwiseParams::getInstance()->animationEnabled);
    client.printf(HEADER_TEMPLATE_D, ClockwiseParams::getInstance()->PREF_SCREEN_MODE, ClockwiseParams::getInstance()->screenMode);
    client.printf(HEADER_TEMPLATE_D, ClockwiseParams::getInstance()->PREF_CHARACTER, ClockwiseParams::getInstance()->characterSelection);
    client.printf(HEADER_TEMPLATE_D, ClockwiseParams::getInstance()->PREF_CLOUD_SPEED, ClockwiseParams::getInstance()->cloudSpeed);
    client.printf(HEADER_TEMPLATE_D, ClockwiseParams::getInstance()->PREF_DYNAMIC_SKY, ClockwiseParams::getInstance()->dynamicSkyMode);
    Serial.printf("[Settings] animationEnabled served by /get: %d\n",
      ClockwiseParams::getInstance()->animationEnabled);

    client.printf(HEADER_TEMPLATE_S, "CW_FW_VERSION", CW_FW_VERSION);
    client.printf(HEADER_TEMPLATE_S, "CW_FW_NAME", CW_FW_NAME);
    client.printf(HEADER_TEMPLATE_S, "CLOCKFACE_NAME", CLOCKFACE_NAME);
    client.printf(HEADER_TEMPLATE_D, "RAMFREE", ramFree);
    client.printf(HEADER_TEMPLATE_D, "RAMTOTAL", ramTotal);
    client.printf(HEADER_TEMPLATE_D, "SKETCHUSED", sketchUsed);
    client.printf(HEADER_TEMPLATE_D, "SKETCHFREE", sketchFree);
    client.printf(HEADER_TEMPLATE_D, "FLASHSIZE", flashSize);
    client.println();
  }

  void getDeviceInfo(WiFiClient client) {
    DynamicJsonDocument doc(256);

    doc["device"] = "clockwise";
    doc["fwName"] = CW_FW_NAME;
    doc["fwVersion"] = CW_FW_VERSION;
    doc["clockface"] = CLOCKFACE_NAME;
    doc["ip"] = WiFi.localIP().toString();
    doc["port"] = 80;

    client.println("HTTP/1.0 200 OK");
    client.println("Content-Type: application/json");
    client.println("Cache-Control: no-store");
    client.println();
    serializeJson(doc, client);
    client.println();
  }

  String readHttpBody(WiFiClient client) {
    int contentLength = -1;
    bool isChunked = false;
    unsigned long deadline = millis() + 3000;

    while (millis() < deadline) {
      if (!client.connected() && !client.available()) {
        break;
      }

      if (!client.available()) {
        delay(1);
        continue;
      }

      String line = client.readStringUntil('\n');
      line.trim();

      if (line.length() == 0) {
        break;
      }

      String lowerLine = line;
      lowerLine.toLowerCase();

      if (lowerLine.startsWith("content-length:")) {
        contentLength = line.substring(line.indexOf(':') + 1).toInt();
      } else if (lowerLine.startsWith("transfer-encoding:") && lowerLine.indexOf("chunked") > 0) {
        isChunked = true;
      }
    }

    String body = "";

    if (isChunked) {
      while (millis() < deadline) {
        while (!client.available() && millis() < deadline) {
          delay(1);
        }

        String chunkSizeLine = client.readStringUntil('\n');
        chunkSizeLine.trim();
        if (chunkSizeLine.length() == 0) {
          continue;
        }

        int chunkSize = (int)strtol(chunkSizeLine.c_str(), nullptr, 16);
        if (chunkSize <= 0) {
          while (client.available()) {
            client.read();
          }
          break;
        }

        int received = 0;
        while (received < chunkSize && millis() < deadline) {
          while (client.available() && received < chunkSize) {
            body.concat((char)client.read());
            received++;
          }

          if (received < chunkSize) {
            delay(1);
          }
        }

        if (client.available()) {
          client.read();
        }
        if (client.available()) {
          client.read();
        }
      }
    } else if (contentLength >= 0) {
      while ((int)body.length() < contentLength && millis() < deadline) {
        while (client.available() && (int)body.length() < contentLength) {
          body.concat((char)client.read());
        }

        if ((int)body.length() < contentLength) {
          delay(1);
        }
      }
    } else {
      unsigned long idleDeadline = millis() + 500;
      while (millis() < deadline && millis() < idleDeadline) {
        while (client.available()) {
          body.concat((char)client.read());
          idleDeadline = millis() + 100;
        }

        if (!client.available()) {
          delay(1);
        }
      }
    }

    return body;
  }

  bool applySettingsObject(JsonObjectConst data, String &error) {
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
    if (data.containsKey("ldrPin")) params->ldrPin = constrain((int)data["ldrPin"], 0, 39);
    if (data.containsKey("manualPosix")) params->manualPosix = data["manualPosix"].as<const char *>();
    if (data.containsKey("displayRotation")) params->displayRotation = constrain((int)data["displayRotation"], 0, 3);
    if (data.containsKey("wifiSsid")) params->wifiSsid = data["wifiSsid"].as<const char *>();
    if (data.containsKey("wifiPwd")) params->wifiPwd = data["wifiPwd"].as<const char *>();
    if (data.containsKey("canvasFile")) params->canvasFile = data["canvasFile"].as<const char *>();
    if (data.containsKey("canvasServer")) params->canvasServer = data["canvasServer"].as<const char *>();

    if (params->autoBrightMin >= params->autoBrightMax) {
      error = "autoBrightMin must be smaller than autoBrightMax";
      return false;
    }

    params->save();
    return true;
  }

  void updateSettingsJson(WiFiClient client) {
    String body = readHttpBody(client);
    DynamicJsonDocument doc(1024);
    DeserializationError err = deserializeJson(doc, body);

    if (err) {
      client.println("HTTP/1.0 400 Bad Request");
      client.println("Content-Type: application/json");
      client.println();
      client.println("{\"ok\":false,\"error\":\"invalid json\"}");
      return;
    }

    JsonObjectConst data = doc["data"].is<JsonObjectConst>() ? doc["data"].as<JsonObjectConst>() : doc.as<JsonObjectConst>();
    if (data.isNull()) {
      client.println("HTTP/1.0 400 Bad Request");
      client.println("Content-Type: application/json");
      client.println();
      client.println("{\"ok\":false,\"error\":\"missing data object\"}");
      return;
    }

    String error;
    if (!applySettingsObject(data, error)) {
      client.println("HTTP/1.0 400 Bad Request");
      client.println("Content-Type: application/json");
      client.println();
      client.printf("{\"ok\":false,\"error\":\"%s\"}\n", error.c_str());
      return;
    }

    getSettingsJson(client);
  }

  void getLdrReading(WiFiClient client) {
    ClockwiseParams::getInstance()->load();

    const uint8_t pin = ClockwiseParams::getInstance()->ldrPin;
    const uint16_t value = analogRead(pin);

    DynamicJsonDocument doc(192);
    doc["device"] = "clockwise";
    doc["sensor"] = "ldr";
    doc["pin"] = pin;
    doc["value"] = value;

    client.println("HTTP/1.0 200 OK");
    client.println("Content-Type: application/json");
    client.println("Cache-Control: no-store");
    client.println();
    serializeJson(doc, client);
    client.println();
  }

  void getSettingsJson(WiFiClient client) {
    ClockwiseParams::getInstance()->load();

    DynamicJsonDocument doc(1024);

    doc["device"] = "clockwise";
    doc["fwName"] = CW_FW_NAME;
    doc["fwVersion"] = CW_FW_VERSION;
    doc["clockface"] = CLOCKFACE_NAME;
    doc["ip"] = WiFi.localIP().toString();
    doc["port"] = 80;

    doc["displayBright"] = ClockwiseParams::getInstance()->displayBright;
    doc["autoBrightMin"] = ClockwiseParams::getInstance()->autoBrightMin;
    doc["autoBrightMax"] = ClockwiseParams::getInstance()->autoBrightMax;
    doc["use24hFormat"] = ClockwiseParams::getInstance()->use24hFormat;
    doc["ldrPin"] = ClockwiseParams::getInstance()->ldrPin;
    doc["timeZone"] = ClockwiseParams::getInstance()->timeZone;
    doc["wifiSsid"] = ClockwiseParams::getInstance()->wifiSsid;
    doc["ntpServer"] = ClockwiseParams::getInstance()->ntpServer;
    doc["canvasFile"] = ClockwiseParams::getInstance()->canvasFile;
    doc["canvasServer"] = ClockwiseParams::getInstance()->canvasServer;
    doc["manualPosix"] = ClockwiseParams::getInstance()->manualPosix;
    doc["displayRotation"] = ClockwiseParams::getInstance()->displayRotation;
    doc["animationEnabled"] = ClockwiseParams::getInstance()->animationEnabled;
    doc["screenMode"] = ClockwiseParams::getInstance()->screenMode;
    doc["character"] = ClockwiseParams::getInstance()->characterSelection;
    doc["cloudSpeed"] = ClockwiseParams::getInstance()->cloudSpeed;
    doc["dynamicSkyMode"] = ClockwiseParams::getInstance()->dynamicSkyMode;

    client.println("HTTP/1.0 200 OK");
    client.println("Content-Type: application/json");
    client.println("Cache-Control: no-store");
    client.println();
    serializeJson(doc, client);
    client.println();
  }
  
};
