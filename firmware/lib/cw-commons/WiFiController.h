#pragma once

#include "ImprovWiFiLibrary.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include "CWWebServer.h"
#include "StatusController.h"
#include <WiFiManager.h>

extern ImprovWiFi improvSerial;

struct WiFiController
{
  enum class State : uint8_t
  {
    Offline,
    Connecting,
    Connected,
    Provisioning,
    Recovering
  };

  static constexpr unsigned long INITIAL_CONNECT_TIMEOUT_MS = 60UL * 1000UL;
  static constexpr unsigned long RECOVERY_TIMEOUT_MS = 5UL * 60UL * 1000UL;
  static constexpr unsigned long RECONNECT_INTERVAL_MS = 10UL * 1000UL;
  static constexpr unsigned long OFFLINE_RETRY_INTERVAL_MS = 60UL * 1000UL;
  static constexpr uint8_t OFFLINE_RETRY_MAX_ATTEMPTS = 5;

  static WiFiController *activeInstance;

  State state = State::Offline;
  unsigned long offlineSinceMillis = 0;
  unsigned long lastReconnectAttemptMillis = 0;
  unsigned long lastOfflineRetryAttemptMillis = 0;
  uint8_t offlineRetryFailedAttempts = 0;
  bool connectionSucessfulOnce = false;
  bool networkServicesStarted = false;

  WiFiController();
  void startNetworkServices();
  void stopNetworkServices();
  void markConnected();
  void markDisconnected(unsigned long now);
  bool shouldRestartAfterOffline(unsigned long now) const;
  bool tryReconnect(unsigned long now);
  bool tryOfflineRetry(unsigned long now);
  static void startMdnsService();
  static void onImprovWiFiErrorCb(ImprovTypes::Error err);
  static void onImprovWiFiConnectedCb(const char *ssid, const char *password);
  bool isConnected();
  void handle();
  bool alternativeSetupMethod();
  bool begin();
};
