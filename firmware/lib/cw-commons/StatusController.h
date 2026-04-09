#pragma once

#include <Arduino.h>

struct StatusController
{
    enum class StatusScreen : uint8_t
    {
        None,
        WiFiConnecting,
        WiFiFailed,
        NtpConnecting
    };

    unsigned long _lastOtaRenderMillis = 0;
    uint8_t _lastOtaProgress = 255;
    bool _ledBlinkActive = false;
    bool _ledStateHigh = false;
    unsigned long _ledLastToggleMillis = 0;
    unsigned long _ledBlinkIntervalMillis = 0;
    int _ledRemainingTransitions = 0;
    bool _restartScheduled = false;
    unsigned long _restartAtMillis = 0;
    StatusScreen _activeStatusScreen = StatusScreen::None;
    unsigned long _statusPixelLastStepMillis = 0;
    int8_t _statusPixelX = -1;
    uint8_t _statusPixelColorIndex = 0;

    static const uint8_t OTA_ICON_SIZE = 16;
    static const uint8_t OTA_ICON_BOX_X = 24;
    static const uint8_t OTA_ICON_BOX_Y = 14;
    static const uint8_t OTA_ICON_BOX_SIZE = 16;
    static const uint8_t OTA_PROGRESS_BAR_X = 7;
    static const uint8_t OTA_PROGRESS_BAR_Y = 53;
    static const uint8_t OTA_PROGRESS_BAR_WIDTH = 50;
    static const uint8_t OTA_PROGRESS_BAR_HEIGHT = 5;
    static const uint8_t OTA_PROGRESS_BUCKETS = 50;
    static const uint8_t STATUS_PIXEL_Y = 63;
    static const unsigned long STATUS_PIXEL_STEP_INTERVAL_MS = 100UL;

    void drawOtaIcon(uint8_t frameIndex);
    void drawOtaProgressBar(uint8_t bucket);
    void resetStatusPixelAnimation();
    void animateStatusPixel(unsigned long now);

    static StatusController *getInstance();

    void clockwiseLogo();
    void wifiConnecting();
    void wifiConnectionFailed(const char *msg);
    void ntpConnecting();
    void otaUpdating(uint8_t progress);
    void resetOtaUi();
    void printCenter(const char *buf, int y);
    void blink_led(int d, int times);
    void forceRestart();
    void clearStatusAnimation();
    void process();
};
