#pragma once

#include <ArduinoJson.h>
#include <Preferences.h>

#ifndef CW_PREF_DB_NAME
    #define CW_PREF_DB_NAME "clockwise"
#endif

struct ClockwiseParams
{
    struct ApplySettingsReport
    {
        bool anyChanged = false;
        bool restartRequired = false;
        bool hotApplied = false;
    };

    Preferences preferences;
    bool loaded = false;

    static constexpr bool DEFAULT_USE_24H_FORMAT = true;
    static constexpr uint32_t DEFAULT_DISPLAY_BRIGHT = 16;
    static constexpr uint32_t DEFAULT_AUTO_BRIGHT_MIN = 0;
    static constexpr uint32_t DEFAULT_AUTO_BRIGHT_MAX = 4095;
    static constexpr uint32_t DEFAULT_LDR_PIN = 34;
    static constexpr const char *DEFAULT_TIME_ZONE = "Asia/Istanbul";
    static constexpr const char *DEFAULT_WIFI_SSID = "wifi";
    static constexpr const char *DEFAULT_WIFI_PASSWORD = "1234";
    static constexpr const char *DEFAULT_NTP_SERVER = "time.google.com";
    static constexpr const char *DEFAULT_MANUAL_POSIX = "";
    static constexpr uint32_t DEFAULT_DISPLAY_ROTATION = 0;
    static constexpr bool DEFAULT_ANIMATION_ENABLED = true;
    static constexpr uint32_t DEFAULT_SCREEN_MODE = 0;
    static constexpr uint32_t DEFAULT_CHARACTER = 0;
    static constexpr uint32_t DEFAULT_CLOUD_SPEED = 10;
    static constexpr uint32_t DEFAULT_DYNAMIC_SKY = 0;
    static constexpr bool DEFAULT_WALKING_MARIO = true;
    static constexpr bool DEFAULT_GOOMBA_ENABLED = true;
    static constexpr size_t TIME_ZONE_CAPACITY = 64;
    static constexpr size_t WIFI_SSID_CAPACITY = 64;
    static constexpr size_t WIFI_PASSWORD_CAPACITY = 64;
    static constexpr size_t NTP_SERVER_CAPACITY = 64;
    static constexpr size_t MANUAL_POSIX_CAPACITY = 96;

    const char* const PREF_USE_24H_FORMAT = "use24hFormat";
    const char* const PREF_DISPLAY_BRIGHT = "displayBright";
    const char* const PREF_DISPLAY_ABC_MIN = "autoBrightMin";
    const char* const PREF_DISPLAY_ABC_MAX = "autoBrightMax";
    const char* const PREF_LDR_PIN = "ldrPin";
    const char* const PREF_TIME_ZONE = "timeZone";
    const char* const PREF_WIFI_SSID = "wifiSsid";
    const char* const PREF_WIFI_PASSWORD = "wifiPwd";
    const char* const PREF_NTP_SERVER = "ntpServer";
    const char* const PREF_MANUAL_POSIX = "manualPosix";
    const char* const PREF_DISPLAY_ROTATION = "displayRotation";
    const char* const PREF_ANIMATION_ENABLED = "animEnabled";
    const char* const PREF_SCREEN_MODE = "screenMode";
    const char* const PREF_CHARACTER = "charSel";
    const char* const PREF_CLOUD_SPEED = "cloudSpeed";
    const char* const PREF_DYNAMIC_SKY = "dynSky";
    const char* const PREF_WALKING_MARIO = "walkingMario";
    const char* const PREF_GOOMBA_ENABLED = "goombaEnabled";

    bool use24hFormat;
    uint8_t displayBright;
    uint16_t autoBrightMin;
    uint16_t autoBrightMax;
    uint8_t ldrPin;
    char timeZone[TIME_ZONE_CAPACITY] = {0};
    char wifiSsid[WIFI_SSID_CAPACITY] = {0};
    char wifiPwd[WIFI_PASSWORD_CAPACITY] = {0};
    char ntpServer[NTP_SERVER_CAPACITY] = {0};
    char manualPosix[MANUAL_POSIX_CAPACITY] = {0};
    uint8_t displayRotation;
    bool animationEnabled;
    uint8_t screenMode;
    uint8_t characterSelection;
    uint8_t cloudSpeed;
    uint8_t dynamicSkyMode;
    bool walkingMario = true;
    bool goombaEnabled = true;


    ClockwiseParams();

    static ClockwiseParams* getInstance();

    static void copyText(char *dest, size_t destSize, const char *src);

    void setTimeZone(const char *value);

    void setWifiSsid(const char *value);

    void setWifiPwd(const char *value);

    void setNtpServer(const char *value);

    void setManualPosix(const char *value);

    void appendSettingsJson(JsonDocument &doc) const;
    void appendSettingsSchemaJson(JsonDocument &doc) const;

    bool isKnownApiSettingKey(const char *key) const;
    bool isRestartRequiredSettingKey(const char *key) const;

    bool applySettingsJson(JsonObjectConst data, const char *&error, ApplySettingsReport *report = nullptr);

    static bool isValidAutoBrightnessRange(uint16_t minValue, uint16_t maxValue);

    bool putBoolIfChanged(const char *key, bool value, bool defaultValue);

    bool putUIntIfChanged(const char *key, uint32_t value, uint32_t defaultValue);

    bool putStringIfChanged(const char *key, const char *value, const char *defaultValue, size_t bufferSize);

   
    void save();

    void load(bool force = false);

};
