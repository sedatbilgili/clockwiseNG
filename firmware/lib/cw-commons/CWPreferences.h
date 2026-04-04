#pragma once

#include <Preferences.h>

#ifndef CW_PREF_DB_NAME
    #define CW_PREF_DB_NAME "clockwise"
#endif


struct ClockwiseParams
{
    Preferences preferences;

    const char* const PREF_USE_24H_FORMAT = "use24hFormat";
    const char* const PREF_DISPLAY_BRIGHT = "displayBright";
    const char* const PREF_DISPLAY_ABC_MIN = "autoBrightMin";
    const char* const PREF_DISPLAY_ABC_MAX = "autoBrightMax";
    const char* const PREF_LDR_PIN = "ldrPin";
    const char* const PREF_TIME_ZONE = "timeZone";
    const char* const PREF_WIFI_SSID = "wifiSsid";
    const char* const PREF_WIFI_PASSWORD = "wifiPwd";
    const char* const PREF_NTP_SERVER = "ntpServer";
    const char* const PREF_CANVAS_FILE = "canvasFile";
    const char* const PREF_CANVAS_SERVER = "canvasServer";
    const char* const PREF_MANUAL_POSIX = "manualPosix";
    const char* const PREF_DISPLAY_ROTATION = "displayRotation";
    const char* const PREF_ANIMATION_ENABLED = "animEnabled";
    const char* const PREF_SCREEN_MODE = "screenMode";
    const char* const PREF_CHARACTER = "charSel";
    const char* const PREF_CLOUD_SPEED = "cloudSpeed";
    const char* const PREF_DYNAMIC_SKY = "dynSky";

    bool use24hFormat;
    uint8_t displayBright;
    uint16_t autoBrightMin;
    uint16_t autoBrightMax;
    uint8_t ldrPin;
    String timeZone;
    String wifiSsid;
    String wifiPwd;
    String ntpServer;
    String canvasFile;
    String canvasServer;
    String manualPosix;
    uint8_t displayRotation;
    bool animationEnabled;
    uint8_t screenMode;
    uint8_t characterSelection;
    uint8_t cloudSpeed;
    uint8_t dynamicSkyMode;


    ClockwiseParams() {
        preferences.begin("clockwise", false); 
        //preferences.clear();
    }

    static ClockwiseParams* getInstance() {
        static ClockwiseParams base;
        return &base;
    }

   
    void save()
    {
        preferences.putBool(PREF_USE_24H_FORMAT, use24hFormat);
        preferences.putUInt(PREF_DISPLAY_BRIGHT, displayBright);
        preferences.putUInt(PREF_DISPLAY_ABC_MIN, autoBrightMin);
        preferences.putUInt(PREF_DISPLAY_ABC_MAX, autoBrightMax);
        preferences.putUInt(PREF_LDR_PIN, ldrPin);        
        preferences.putString(PREF_TIME_ZONE, timeZone);
        preferences.putString(PREF_WIFI_SSID, wifiSsid);
        preferences.putString(PREF_WIFI_PASSWORD, wifiPwd);
        preferences.putString(PREF_NTP_SERVER, ntpServer);
        preferences.putString(PREF_CANVAS_FILE, canvasFile);
        preferences.putString(PREF_CANVAS_SERVER, canvasServer);
        preferences.putString(PREF_MANUAL_POSIX, manualPosix);
        preferences.putUInt(PREF_DISPLAY_ROTATION, displayRotation);
        preferences.putBool(PREF_ANIMATION_ENABLED, animationEnabled);
        preferences.putUInt(PREF_SCREEN_MODE, screenMode);
        preferences.putUInt(PREF_CHARACTER, characterSelection);
        preferences.putUInt(PREF_CLOUD_SPEED, cloudSpeed);
        preferences.putUInt(PREF_DYNAMIC_SKY, dynamicSkyMode);
    }

    void load()
    {
        use24hFormat = preferences.getBool(PREF_USE_24H_FORMAT, true);
        displayBright = preferences.getUInt(PREF_DISPLAY_BRIGHT, 16);
        autoBrightMin = preferences.getUInt(PREF_DISPLAY_ABC_MIN, 0);
        autoBrightMax = preferences.getUInt(PREF_DISPLAY_ABC_MAX, 4095);
        ldrPin = preferences.getUInt(PREF_LDR_PIN, 34);        
        timeZone = preferences.getString(PREF_TIME_ZONE, "Asia/Istanbul");
        wifiSsid = preferences.getString(PREF_WIFI_SSID, "wifi");
        wifiPwd = preferences.getString(PREF_WIFI_PASSWORD, "1234");
        ntpServer = preferences.getString(PREF_NTP_SERVER, "time.google.com");
        canvasFile = preferences.getString(PREF_CANVAS_FILE, "");
        canvasServer = preferences.getString(PREF_CANVAS_SERVER, "raw.githubusercontent.com");
        manualPosix = preferences.getString(PREF_MANUAL_POSIX, "");
        displayRotation = preferences.getUInt(PREF_DISPLAY_ROTATION, 0);
        animationEnabled = preferences.getBool(PREF_ANIMATION_ENABLED, true);
        screenMode = preferences.getUInt(PREF_SCREEN_MODE, 0);
        characterSelection = preferences.getUInt(PREF_CHARACTER, 0);
        cloudSpeed = preferences.getUInt(PREF_CLOUD_SPEED, 10);
        dynamicSkyMode = preferences.getUInt(PREF_DYNAMIC_SKY, 0);
    }

};
