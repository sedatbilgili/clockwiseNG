#include "WiFiController.h"

ImprovWiFi improvSerial(&Serial);
WiFiController *WiFiController::activeInstance = nullptr;

namespace
{
constexpr unsigned long CONNECT_ATTEMPT_TIMEOUT_MS = 11000UL;
constexpr unsigned long CONNECT_ATTEMPT_POLL_DELAY_MS = 50UL;

bool tryConnectToWifiWithUi(const char *ssid, const char *password)
{
  if (ssid == nullptr || ssid[0] == '\0')
  {
    return false;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    WiFi.disconnect();
    delay(100);
  }

  WiFi.begin(ssid, password);
  const unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED)
  {
    // Keep status animations and restart scheduler alive while connection waits.
    StatusController::getInstance()->process();
    delay(CONNECT_ATTEMPT_POLL_DELAY_MS);
    if ((millis() - start) >= CONNECT_ATTEMPT_TIMEOUT_MS)
    {
      WiFi.disconnect();
      return false;
    }
  }

  return true;
}
}

WiFiController::WiFiController()
{
  activeInstance = this;
}

void WiFiController::startNetworkServices()
{
  if (networkServicesStarted)
  {
    return;
  }

  ClockwiseWebServer::getInstance()->startWebServer();
  startMdnsService();
  networkServicesStarted = true;
}

void WiFiController::stopNetworkServices()
{
  if (!networkServicesStarted)
  {
    return;
  }

  ClockwiseWebServer::getInstance()->stopWebServer();
  MDNS.end();
  networkServicesStarted = false;
}

void WiFiController::markConnected()
{
  offlineSinceMillis = 0;
  lastReconnectAttemptMillis = 0;
  lastOfflineRetryAttemptMillis = 0;
  offlineRetryFailedAttempts = 0;
  state = State::Connected;
  connectionSucessfulOnce = true;
  startNetworkServices();
}

void WiFiController::markDisconnected(unsigned long now)
{
  if (offlineSinceMillis == 0)
  {
    offlineSinceMillis = now;
    if (!connectionSucessfulOnce)
    {
      lastOfflineRetryAttemptMillis = now;
      offlineRetryFailedAttempts = 0;
    }
  }

  state = connectionSucessfulOnce ? State::Recovering : State::Offline;
  stopNetworkServices();
}

bool WiFiController::shouldRestartAfterOffline(unsigned long now) const
{
  if (offlineSinceMillis == 0)
  {
    return false;
  }

  const unsigned long timeout = connectionSucessfulOnce
    ? RECOVERY_TIMEOUT_MS
    : INITIAL_CONNECT_TIMEOUT_MS;
  return (now - offlineSinceMillis) >= timeout;
}

bool WiFiController::tryReconnect(unsigned long now)
{
  ClockwiseParams *params = ClockwiseParams::getInstance();
  if (params->wifiSsid[0] == '\0')
  {
    return false;
  }

  if (lastReconnectAttemptMillis != 0 && (now - lastReconnectAttemptMillis) < RECONNECT_INTERVAL_MS)
  {
    return false;
  }

  lastReconnectAttemptMillis = now;
  state = State::Connecting;
  Serial.printf("[WiFi] Reconnect attempt to %s\n", params->wifiSsid);
  if (tryConnectToWifiWithUi(params->wifiSsid, params->wifiPwd))
  {
    markConnected();
    Serial.printf("[WiFi] Reconnected to %s, IP address %s\n",
      WiFi.SSID().c_str(),
      WiFi.localIP().toString().c_str());
    return true;
  }

  state = State::Recovering;
  return false;
}

bool WiFiController::tryOfflineRetry(unsigned long now)
{
  ClockwiseParams *params = ClockwiseParams::getInstance();
  if (params->wifiSsid[0] == '\0')
  {
    return false;
  }

  if (lastOfflineRetryAttemptMillis != 0
    && (now - lastOfflineRetryAttemptMillis) < OFFLINE_RETRY_INTERVAL_MS)
  {
    return false;
  }

  lastOfflineRetryAttemptMillis = now;
  state = State::Connecting;
  Serial.printf("[WiFi] Offline retry %u/%u to %s\n",
    static_cast<unsigned int>(offlineRetryFailedAttempts + 1),
    static_cast<unsigned int>(OFFLINE_RETRY_MAX_ATTEMPTS),
    params->wifiSsid);

  if (tryConnectToWifiWithUi(params->wifiSsid, params->wifiPwd))
  {
    markConnected();
    Serial.printf("[WiFi] Connected on offline retry to %s, IP address %s\n",
      WiFi.SSID().c_str(),
      WiFi.localIP().toString().c_str());
    return true;
  }

  if (offlineRetryFailedAttempts < OFFLINE_RETRY_MAX_ATTEMPTS)
  {
    offlineRetryFailedAttempts++;
  }
  state = State::Offline;
  StatusController::getInstance()->wifiConnectionFailed("WiFi Yok!");
  return false;
}

void WiFiController::startMdnsService()
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

void WiFiController::onImprovWiFiErrorCb(ImprovTypes::Error err)
{
  if (activeInstance != nullptr)
  {
    activeInstance->stopNetworkServices();
  }
  StatusController::getInstance()->blink_led(2000, 3);
  (void)err;
}

void WiFiController::onImprovWiFiConnectedCb(const char *ssid, const char *password)
{
  ClockwiseParams::getInstance()->load();
  ClockwiseParams::getInstance()->setWifiSsid(ssid);
  ClockwiseParams::getInstance()->setWifiPwd(password);
  ClockwiseParams::getInstance()->save();

  if (activeInstance != nullptr)
  {
    activeInstance->markConnected();
  }
}

bool WiFiController::isConnected()
{
  return state == State::Connected;
}

void WiFiController::handle()
{
  improvSerial.handleSerial();
  unsigned long now = millis();
  ClockwiseParams *params = ClockwiseParams::getInstance();

  if (improvSerial.isConnected())
  {
    if (state != State::Connected)
    {
      markConnected();
    }
    return;
  }

  markDisconnected(now);

  if (!connectionSucessfulOnce && params->wifiSsid[0] != '\0')
  {
    tryOfflineRetry(now);
    const bool exceededOfflineRetryLimit = offlineRetryFailedAttempts >= OFFLINE_RETRY_MAX_ATTEMPTS;
    const bool exceededInitialOfflineTimeout = offlineSinceMillis != 0
      && (now - offlineSinceMillis) >= RECOVERY_TIMEOUT_MS;
    if (exceededOfflineRetryLimit || exceededInitialOfflineTimeout)
    {
      StatusController::getInstance()->forceRestart();
    }
    return;
  }

  if (connectionSucessfulOnce)
  {
    tryReconnect(now);
  }

  if (shouldRestartAfterOffline(now))
  {
    StatusController::getInstance()->forceRestart();
  }
}

bool WiFiController::alternativeSetupMethod()
{
  WiFiManager wifiManager;
  constexpr unsigned long portalTimeoutMs = 180000UL;
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
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.setConfigPortalBlocking(false);

  wifiManager.startConfigPortal("Clockwise-Wifi");
  const unsigned long portalStart = millis();
  while (millis() - portalStart < portalTimeoutMs)
  {
    StatusController::getInstance()->process();

    if (wifiManager.process() || WiFi.status() == WL_CONNECTED)
    {
      onImprovWiFiConnectedCb(WiFi.SSID().c_str(), WiFi.psk().c_str());
      Serial.printf("[WiFi] Connected via WiFiManager to %s, IP address %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
      return true;
    }

    delay(10);
  }

  wifiManager.stopConfigPortal();
  return false;
}

bool WiFiController::begin()
{
  activeInstance = this;
  connectionSucessfulOnce = false;
  networkServicesStarted = false;
  offlineSinceMillis = 0;
  lastReconnectAttemptMillis = 0;
  lastOfflineRetryAttemptMillis = 0;
  offlineRetryFailedAttempts = 0;
  state = State::Connecting;

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  improvSerial.setDeviceInfo(ImprovTypes::ChipFamily::CF_ESP32, CW_FW_NAME, CW_FW_VERSION, "Clockwise");
  improvSerial.onImprovError(onImprovWiFiErrorCb);
  improvSerial.onImprovConnected(onImprovWiFiConnectedCb);

  ClockwiseParams::getInstance()->load();

  if (ClockwiseParams::getInstance()->wifiSsid[0] != '\0')
  {
    if (tryConnectToWifiWithUi(ClockwiseParams::getInstance()->wifiSsid, ClockwiseParams::getInstance()->wifiPwd))
    {
      markConnected();
      Serial.printf("[WiFi] Connected to %s, IP address %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
      return true;
    }

    state = State::Provisioning;
    StatusController::getInstance()->wifiConnectionFailed("WiFi Ayarlayin");
    if (alternativeSetupMethod())
    {
      return true;
    }
  }

  state = State::Offline;
  offlineSinceMillis = millis();
  lastOfflineRetryAttemptMillis = offlineSinceMillis;
  offlineRetryFailedAttempts = 0;
  StatusController::getInstance()->wifiConnectionFailed("WiFi Yok!");
  return false;
}
