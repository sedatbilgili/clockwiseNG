#pragma once

#include "ImprovWiFiLibrary.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include "CWWebServer.h"
#include "StatusController.h"
#include <WiFiManager.h>

ImprovWiFi improvSerial(&Serial);

struct WiFiController
{
  long elapsedTimeOffline = 0;
  bool connectionSucessfulOnce;

  static void startMdnsService()
  {
    if (MDNS.begin("clockwise"))
    {
      MDNS.addService("http", "tcp", 80);
      Serial.printf("[mDNS] Service announced as http://clockwise.local:%d\n", 80);
    }
    else
    {
      Serial.println("[mDNS] Failed to start");
    }
  }

  static void onImprovWiFiErrorCb(ImprovTypes::Error err)
  {
    ClockwiseWebServer::getInstance()->stopWebServer();
    StatusController::getInstance()->blink_led(2000, 3);
  }

  static void onImprovWiFiConnectedCb(const char *ssid, const char *password)
  {
    ClockwiseParams::getInstance()->load();
    ClockwiseParams::getInstance()->wifiSsid = String(ssid);
    ClockwiseParams::getInstance()->wifiPwd = String(password);
    ClockwiseParams::getInstance()->save();

    ClockwiseWebServer::getInstance()->startWebServer();
    startMdnsService();
  }

  bool isConnected()
  {
    if (improvSerial.isConnected()) {
      elapsedTimeOffline = 0;
      return true;
    } else {
      if (elapsedTimeOffline == 0 && !connectionSucessfulOnce)
        elapsedTimeOffline = millis();
      
      if ((millis() - elapsedTimeOffline) > 1000 * 60 * 1)  // restart if clockface is not showed and is 5min offline 
        StatusController::getInstance()->forceRestart();

      return false;
    }
  }

  static void handleImprovWiFi()
  {
    improvSerial.handleSerial();
  }

  bool alternativeSetupMethod()
  {
    WiFiManager wifiManager;
    static const char portalHead[] PROGMEM = R"rawliteral(
<style>
  :root {
    --cw-bg: #d9d4cb;
    --cw-card: #ffffff;
    --cw-text: #2e3440;
    --cw-accent: #159957;
    --cw-accent-dark: #155799;
    --cw-muted: #6b7280;
    --cw-border: #d7d7d7;
  }

  body {
    background-color: var(--cw-bg) !important;
    background-image:
      radial-gradient(rgba(255,255,255,0.16) 0.8px, transparent 0.8px),
      radial-gradient(rgba(0,0,0,0.05) 0.7px, transparent 0.7px),
      linear-gradient(180deg, #dfdbd2 0%, #d4cec4 100%) !important;
    background-position: 0 0, 9px 11px, 0 0 !important;
    background-size: 18px 18px, 21px 21px, auto !important;
    color: var(--cw-text) !important;
  }

  .wrap, .msg, fieldset, form {
    border-radius: 14px !important;
  }

  .wrap {
    max-width: 760px !important;
    background: transparent !important;
    box-shadow: none !important;
  }

  fieldset, .msg, form {
    background: var(--cw-card) !important;
    border: 1px solid var(--cw-border) !important;
    box-shadow: 0 8px 24px rgba(0,0,0,0.08) !important;
  }

  button, .btn, input[type=submit] {
    background: linear-gradient(120deg, var(--cw-accent-dark), var(--cw-accent)) !important;
    border: 0 !important;
    border-radius: 10px !important;
  }

  input, select {
    border-radius: 10px !important;
    border: 1px solid var(--cw-border) !important;
  }

  a, .q {
    color: var(--cw-accent-dark) !important;
  }

  .c {
    color: var(--cw-muted) !important;
  }

  h1, h2, h3 {
    color: var(--cw-text) !important;
  }

  hr {
    border-color: var(--cw-border) !important;
  }
 </style>
)rawliteral";

    static const char portalHeader[] PROGMEM = R"rawliteral(
<div style="max-width:760px;margin:18px auto 10px auto;background:linear-gradient(120deg,#155799,#159957);color:#fff;border-radius:16px;padding:18px 22px;box-shadow:0 10px 24px rgba(0,0,0,0.12);">
  <div style="font-size:26px;font-weight:700;line-height:1.2;">Clockwise Wi-Fi Kurulumu</div>
  <div style="font-size:14px;opacity:0.92;margin-top:6px;">Cihaz mevcut ağa bağlanamadı. Aşağıdan yeni bir Wi-Fi seçip şifre girerek kurulumu tamamlayabilirsiniz.</div>
</div>
)rawliteral";

    static const char portalMenu[] PROGMEM = R"rawliteral(
<div style="margin-top:12px;padding:12px 14px;background:#ffffff;border:1px solid #d7d7d7;border-radius:12px;color:#2e3440;">
  <strong>İpucu:</strong> Telefon veya bilgisayarınız <strong>Clockwise-Wifi</strong> ağına bağlı kalmalı. Kaydettikten sonra cihaz yeni ağa bağlanmayı deneyecek.
</div>
)rawliteral";

    wifiManager.setTitle("Clockwise");
    wifiManager.setCustomHeadElement(portalHead);
    wifiManager.setCustomBodyHeader(portalHeader);
    wifiManager.setCustomMenuHTML(portalMenu);
    wifiManager.setClass("");
    wifiManager.setConfigPortalTimeout(180); // Wait 3min to configure wifi via AP

    bool success = wifiManager.startConfigPortal("Clockwise-Wifi");

    if (success)
    {
      onImprovWiFiConnectedCb(WiFi.SSID().c_str(), WiFi.psk().c_str());
      Serial.printf("[WiFi] Connected via WiFiManager to %s, IP address %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
      connectionSucessfulOnce = success;
    }

    return success;
  }

  bool begin()
  {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    improvSerial.setDeviceInfo(ImprovTypes::ChipFamily::CF_ESP32, CW_FW_NAME, CW_FW_VERSION, "Clockwise");
    improvSerial.onImprovError(onImprovWiFiErrorCb);
    improvSerial.onImprovConnected(onImprovWiFiConnectedCb);

    ClockwiseParams::getInstance()->load();

    if (!ClockwiseParams::getInstance()->wifiSsid.isEmpty())
    {
      if (improvSerial.tryConnectToWifi(ClockwiseParams::getInstance()->wifiSsid.c_str(), ClockwiseParams::getInstance()->wifiPwd.c_str()))
      {
        connectionSucessfulOnce = true;
        ClockwiseWebServer::getInstance()->startWebServer();
        startMdnsService();
        Serial.printf("[WiFi] Connected to %s, IP address %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
        return true;
      }

      StatusController::getInstance()->wifiConnectionFailed("WiFi Ayarlayin");
      alternativeSetupMethod();
    }

    StatusController::getInstance()->wifiConnectionFailed("WiFi Yok!");
    return false;
  }
};
