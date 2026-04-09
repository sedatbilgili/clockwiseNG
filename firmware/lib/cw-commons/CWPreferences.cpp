#include "CWPreferences.h"

#include <Arduino.h>
#include <cstring>

namespace
{
struct BoolSettingSchema
{
    const char *preferenceKey;
    const char *apiKey;
    bool defaultValue;
    bool (*getValue)(const ClockwiseParams &params);
    void (*setValue)(ClockwiseParams &params, bool value);
};

struct UIntSettingSchema
{
    const char *preferenceKey;
    const char *apiKey;
    uint32_t defaultValue;
    uint32_t minValue;
    uint32_t maxValue;
    uint32_t (*getValue)(const ClockwiseParams &params);
    void (*setValue)(ClockwiseParams &params, uint32_t value);
};

struct StringSettingSchema
{
    const char *preferenceKey;
    const char *apiKey;
    const char *defaultValue;
    size_t capacity;
    bool includeInState;
    const char *(*getValue)(const ClockwiseParams &params);
    void (*setValue)(ClockwiseParams &params, const char *value);
};

bool getUse24hFormat(const ClockwiseParams &params) { return params.use24hFormat; }
void setUse24hFormat(ClockwiseParams &params, bool value) { params.use24hFormat = value; }
bool getAnimationEnabled(const ClockwiseParams &params) { return params.animationEnabled; }
void setAnimationEnabled(ClockwiseParams &params, bool value) { params.animationEnabled = value; }
bool getWalkingMario(const ClockwiseParams &params) { return params.walkingMario; }
void setWalkingMario(ClockwiseParams &params, bool value) { params.walkingMario = value; }
bool getGoombaEnabled(const ClockwiseParams &params) { return params.goombaEnabled; }
void setGoombaEnabled(ClockwiseParams &params, bool value) { params.goombaEnabled = value; }

uint32_t getDisplayBright(const ClockwiseParams &params) { return params.displayBright; }
void setDisplayBright(ClockwiseParams &params, uint32_t value) { params.displayBright = static_cast<uint8_t>(value); }
uint32_t getAutoBrightMin(const ClockwiseParams &params) { return params.autoBrightMin; }
void setAutoBrightMin(ClockwiseParams &params, uint32_t value) { params.autoBrightMin = static_cast<uint16_t>(value); }
uint32_t getAutoBrightMax(const ClockwiseParams &params) { return params.autoBrightMax; }
void setAutoBrightMax(ClockwiseParams &params, uint32_t value) { params.autoBrightMax = static_cast<uint16_t>(value); }
uint32_t getLdrPin(const ClockwiseParams &params) { return params.ldrPin; }
void setLdrPin(ClockwiseParams &params, uint32_t value) { params.ldrPin = static_cast<uint8_t>(value); }
uint32_t getDisplayRotation(const ClockwiseParams &params) { return params.displayRotation; }
void setDisplayRotation(ClockwiseParams &params, uint32_t value) { params.displayRotation = static_cast<uint8_t>(value); }
uint32_t getScreenMode(const ClockwiseParams &params) { return params.screenMode; }
void setScreenMode(ClockwiseParams &params, uint32_t value) { params.screenMode = static_cast<uint8_t>(value); }
uint32_t getCharacterSelection(const ClockwiseParams &params) { return params.characterSelection; }
void setCharacterSelection(ClockwiseParams &params, uint32_t value) { params.characterSelection = static_cast<uint8_t>(value); }
uint32_t getCloudSpeed(const ClockwiseParams &params) { return params.cloudSpeed; }
void setCloudSpeed(ClockwiseParams &params, uint32_t value) { params.cloudSpeed = static_cast<uint8_t>(value); }
uint32_t getDynamicSkyMode(const ClockwiseParams &params) { return params.dynamicSkyMode; }
void setDynamicSkyMode(ClockwiseParams &params, uint32_t value) { params.dynamicSkyMode = static_cast<uint8_t>(value); }

const char *getTimeZone(const ClockwiseParams &params) { return params.timeZone; }
const char *getWifiSsid(const ClockwiseParams &params) { return params.wifiSsid; }
const char *getWifiPwd(const ClockwiseParams &params) { return params.wifiPwd; }
const char *getNtpServer(const ClockwiseParams &params) { return params.ntpServer; }
const char *getManualPosix(const ClockwiseParams &params) { return params.manualPosix; }
void setTimeZoneValue(ClockwiseParams &params, const char *value) { params.setTimeZone(value); }
void setWifiSsidValue(ClockwiseParams &params, const char *value) { params.setWifiSsid(value); }
void setWifiPwdValue(ClockwiseParams &params, const char *value) { params.setWifiPwd(value); }
void setNtpServerValue(ClockwiseParams &params, const char *value) { params.setNtpServer(value); }
void setManualPosixValue(ClockwiseParams &params, const char *value) { params.setManualPosix(value); }

const BoolSettingSchema BOOL_SETTINGS[] = {
    {"use24hFormat", "use24hFormat", ClockwiseParams::DEFAULT_USE_24H_FORMAT, getUse24hFormat, setUse24hFormat},
    {"animEnabled", "animationEnabled", ClockwiseParams::DEFAULT_ANIMATION_ENABLED, getAnimationEnabled, setAnimationEnabled},
    {"walkingMario", "walkingMario", ClockwiseParams::DEFAULT_WALKING_MARIO, getWalkingMario, setWalkingMario},
    {"goombaEnabled", "goombaEnabled", ClockwiseParams::DEFAULT_GOOMBA_ENABLED, getGoombaEnabled, setGoombaEnabled},
};

const UIntSettingSchema UINT_SETTINGS[] = {
    {"displayBright", "displayBright", ClockwiseParams::DEFAULT_DISPLAY_BRIGHT, 0, 255, getDisplayBright, setDisplayBright},
    {"autoBrightMin", "autoBrightMin", ClockwiseParams::DEFAULT_AUTO_BRIGHT_MIN, 0, 4095, getAutoBrightMin, setAutoBrightMin},
    {"autoBrightMax", "autoBrightMax", ClockwiseParams::DEFAULT_AUTO_BRIGHT_MAX, 0, 4095, getAutoBrightMax, setAutoBrightMax},
    {"ldrPin", "ldrPin", ClockwiseParams::DEFAULT_LDR_PIN, 0, 39, getLdrPin, setLdrPin},
    {"displayRotation", "displayRotation", ClockwiseParams::DEFAULT_DISPLAY_ROTATION, 0, 3, getDisplayRotation, setDisplayRotation},
    {"screenMode", "screenMode", ClockwiseParams::DEFAULT_SCREEN_MODE, 0, 2, getScreenMode, setScreenMode},
    {"charSel", "character", ClockwiseParams::DEFAULT_CHARACTER, 0, 1, getCharacterSelection, setCharacterSelection},
    {"cloudSpeed", "cloudSpeed", ClockwiseParams::DEFAULT_CLOUD_SPEED, 1, 30, getCloudSpeed, setCloudSpeed},
    {"dynSky", "dynamicSkyMode", ClockwiseParams::DEFAULT_DYNAMIC_SKY, 0, 2, getDynamicSkyMode, setDynamicSkyMode},
};

const StringSettingSchema STRING_SETTINGS[] = {
    {"timeZone", "timeZone", ClockwiseParams::DEFAULT_TIME_ZONE, ClockwiseParams::TIME_ZONE_CAPACITY, true, getTimeZone, setTimeZoneValue},
    {"wifiSsid", "wifiSsid", ClockwiseParams::DEFAULT_WIFI_SSID, ClockwiseParams::WIFI_SSID_CAPACITY, true, getWifiSsid, setWifiSsidValue},
    {"wifiPwd", "wifiPwd", ClockwiseParams::DEFAULT_WIFI_PASSWORD, ClockwiseParams::WIFI_PASSWORD_CAPACITY, false, getWifiPwd, setWifiPwdValue},
    {"ntpServer", "ntpServer", ClockwiseParams::DEFAULT_NTP_SERVER, ClockwiseParams::NTP_SERVER_CAPACITY, true, getNtpServer, setNtpServerValue},
    {"manualPosix", "manualPosix", ClockwiseParams::DEFAULT_MANUAL_POSIX, ClockwiseParams::MANUAL_POSIX_CAPACITY, true, getManualPosix, setManualPosixValue},
};

const char *const RESTART_REQUIRED_API_KEYS[] = {
    "use24hFormat",
    "ldrPin",
    "displayRotation",
    "timeZone",
    "wifiSsid",
    "wifiPwd",
    "ntpServer",
    "manualPosix",
};

template <typename TSetting, size_t N>
const TSetting *findSettingByApiKey(const TSetting (&settings)[N], const char *key)
{
    if (key == nullptr) {
        return nullptr;
    }

    for (size_t i = 0; i < N; ++i) {
        if (strcmp(settings[i].apiKey, key) == 0) {
            return &settings[i];
        }
    }

    return nullptr;
}

template <typename TSetting, size_t N>
bool hasSettingByApiKey(const TSetting (&settings)[N], const char *key)
{
    return findSettingByApiKey(settings, key) != nullptr;
}

template <size_t N>
bool isKeyInList(const char *const (&values)[N], const char *key)
{
    if (key == nullptr) {
        return false;
    }

    for (size_t i = 0; i < N; ++i) {
        if (strcmp(values[i], key) == 0) {
            return true;
        }
    }

    return false;
}

} // namespace

ClockwiseParams::ClockwiseParams()
{
    preferences.begin(CW_PREF_DB_NAME, false);
}

ClockwiseParams* ClockwiseParams::getInstance()
{
    static ClockwiseParams base;
    return &base;
}

void ClockwiseParams::copyText(char *dest, size_t destSize, const char *src)
{
    if (dest == nullptr || destSize == 0) {
        return;
    }

    if (src == nullptr) {
        dest[0] = '\0';
        return;
    }

    strncpy(dest, src, destSize - 1);
    dest[destSize - 1] = '\0';
}

void ClockwiseParams::setTimeZone(const char *value)
{
    copyText(timeZone, sizeof(timeZone), value);
}

void ClockwiseParams::setWifiSsid(const char *value)
{
    copyText(wifiSsid, sizeof(wifiSsid), value);
}

void ClockwiseParams::setWifiPwd(const char *value)
{
    copyText(wifiPwd, sizeof(wifiPwd), value);
}

void ClockwiseParams::setNtpServer(const char *value)
{
    copyText(ntpServer, sizeof(ntpServer), value);
}

void ClockwiseParams::setManualPosix(const char *value)
{
    copyText(manualPosix, sizeof(manualPosix), value);
}

void ClockwiseParams::appendSettingsJson(JsonDocument &doc) const
{
    for (const BoolSettingSchema &setting : BOOL_SETTINGS) {
        doc[setting.apiKey] = setting.getValue(*this);
    }

    for (const UIntSettingSchema &setting : UINT_SETTINGS) {
        doc[setting.apiKey] = setting.getValue(*this);
    }

    for (const StringSettingSchema &setting : STRING_SETTINGS) {
        if (!setting.includeInState) {
            continue;
        }
        doc[setting.apiKey] = setting.getValue(*this);
    }
}

void ClockwiseParams::appendSettingsSchemaJson(JsonDocument &doc) const
{
    doc["schemaVersion"] = 1;
    JsonObject settings = doc.createNestedObject("settings");

    for (const BoolSettingSchema &setting : BOOL_SETTINGS) {
        JsonObject field = settings.createNestedObject(setting.apiKey);
        field["type"] = "boolean";
        field["default"] = setting.defaultValue;
        field["writable"] = true;
        field["inState"] = true;
        field["apply"] = isRestartRequiredSettingKey(setting.apiKey) ? "restart" : "hot";
    }

    for (const UIntSettingSchema &setting : UINT_SETTINGS) {
        JsonObject field = settings.createNestedObject(setting.apiKey);
        field["type"] = "integer";
        field["default"] = setting.defaultValue;
        field["min"] = setting.minValue;
        field["max"] = setting.maxValue;
        field["writable"] = true;
        field["inState"] = true;
        field["apply"] = isRestartRequiredSettingKey(setting.apiKey) ? "restart" : "hot";
    }

    for (const StringSettingSchema &setting : STRING_SETTINGS) {
        JsonObject field = settings.createNestedObject(setting.apiKey);
        field["type"] = "string";
        field["default"] = setting.defaultValue;
        field["maxLength"] = setting.capacity > 0 ? setting.capacity - 1 : 0;
        field["writable"] = true;
        field["inState"] = setting.includeInState;
        field["sensitive"] = !setting.includeInState;
        field["apply"] = isRestartRequiredSettingKey(setting.apiKey) ? "restart" : "hot";
    }
}

bool ClockwiseParams::isKnownApiSettingKey(const char *key) const
{
    return hasSettingByApiKey(BOOL_SETTINGS, key)
        || hasSettingByApiKey(UINT_SETTINGS, key)
        || hasSettingByApiKey(STRING_SETTINGS, key);
}

bool ClockwiseParams::isRestartRequiredSettingKey(const char *key) const
{
    return isKeyInList(RESTART_REQUIRED_API_KEYS, key);
}

bool ClockwiseParams::applySettingsJson(JsonObjectConst data, const char *&error, ApplySettingsReport *report)
{
    load();
    ApplySettingsReport localReport;
    ApplySettingsReport &appliedReport = report != nullptr ? *report : localReport;
    appliedReport = ApplySettingsReport();

    for (JsonPairConst entry : data) {
        const char *key = entry.key().c_str();
        const bool restartRequiredForKey = isRestartRequiredSettingKey(key);

        if (const UIntSettingSchema *setting = findSettingByApiKey(UINT_SETTINGS, key)) {
            if (!entry.value().is<int>() && !entry.value().is<long>() && !entry.value().is<unsigned int>() && !entry.value().is<unsigned long>()) {
                error = "invalid integer setting value";
                return false;
            }
            uint32_t oldValue = setting->getValue(*this);
            uint32_t value = static_cast<uint32_t>(constrain(entry.value().as<int>(), static_cast<int>(setting->minValue), static_cast<int>(setting->maxValue)));
            setting->setValue(*this, value);
            if (oldValue != value) {
                appliedReport.anyChanged = true;
                if (restartRequiredForKey) {
                    appliedReport.restartRequired = true;
                } else {
                    appliedReport.hotApplied = true;
                }
            }
            continue;
        }

        if (const BoolSettingSchema *setting = findSettingByApiKey(BOOL_SETTINGS, key)) {
            if (!entry.value().is<bool>()) {
                error = "invalid boolean setting value";
                return false;
            }
            const bool value = entry.value().as<bool>();
            const bool oldValue = setting->getValue(*this);
            setting->setValue(*this, value);
            if (oldValue != value) {
                appliedReport.anyChanged = true;
                if (restartRequiredForKey) {
                    appliedReport.restartRequired = true;
                } else {
                    appliedReport.hotApplied = true;
                }
            }
            continue;
        }

        if (const StringSettingSchema *setting = findSettingByApiKey(STRING_SETTINGS, key)) {
            if (!entry.value().is<const char *>()) {
                error = "invalid string setting value";
                return false;
            }
            const char *value = entry.value().as<const char *>();
            char oldValue[MANUAL_POSIX_CAPACITY] = {0};
            copyText(oldValue, sizeof(oldValue), setting->getValue(*this));
            setting->setValue(*this, value);
            const char *safeNewValue = value != nullptr ? value : "";
            if (strcmp(oldValue, safeNewValue) != 0) {
                appliedReport.anyChanged = true;
                if (restartRequiredForKey) {
                    appliedReport.restartRequired = true;
                } else {
                    appliedReport.hotApplied = true;
                }
            }
            continue;
        }

        error = "unknown setting key";
        return false;
    }

    if (!isValidAutoBrightnessRange(autoBrightMin, autoBrightMax)) {
        error = "autoBrightMin must be smaller than autoBrightMax";
        return false;
    }

    if (!animationEnabled) {
        const bool wasWalkingMario = walkingMario;
        walkingMario = false;
        if (wasWalkingMario != walkingMario) {
            appliedReport.anyChanged = true;
            appliedReport.hotApplied = true;
        }
    }

    save();
    error = nullptr;
    return true;
}

bool ClockwiseParams::isValidAutoBrightnessRange(uint16_t minValue, uint16_t maxValue)
{
    return minValue < maxValue;
}

bool ClockwiseParams::putBoolIfChanged(const char *key, bool value, bool defaultValue)
{
    bool currentValue = preferences.getBool(key, defaultValue);
    if (currentValue == value) {
        return false;
    }

    preferences.putBool(key, value);
    return true;
}

bool ClockwiseParams::putUIntIfChanged(const char *key, uint32_t value, uint32_t defaultValue)
{
    uint32_t currentValue = preferences.getUInt(key, defaultValue);
    if (currentValue == value) {
        return false;
    }

    preferences.putUInt(key, value);
    return true;
}

bool ClockwiseParams::putStringIfChanged(const char *key, const char *value, const char *defaultValue, size_t bufferSize)
{
    char currentValue[MANUAL_POSIX_CAPACITY] = {0};
    size_t readSize = bufferSize < sizeof(currentValue) ? bufferSize : sizeof(currentValue);
    preferences.getString(key, currentValue, readSize);
    if (currentValue[0] == '\0' && defaultValue != nullptr) {
        copyText(currentValue, sizeof(currentValue), defaultValue);
    }

    if (strcmp(currentValue, value != nullptr ? value : "") == 0) {
        return false;
    }

    preferences.putString(key, value != nullptr ? value : "");
    return true;
}

void ClockwiseParams::save()
{
    if (!loaded) {
        load();
    }

    for (const BoolSettingSchema &setting : BOOL_SETTINGS) {
        putBoolIfChanged(setting.preferenceKey, setting.getValue(*this), setting.defaultValue);
    }

    for (const UIntSettingSchema &setting : UINT_SETTINGS) {
        putUIntIfChanged(setting.preferenceKey, setting.getValue(*this), setting.defaultValue);
    }

    for (const StringSettingSchema &setting : STRING_SETTINGS) {
        putStringIfChanged(
            setting.preferenceKey,
            setting.getValue(*this),
            setting.defaultValue,
            setting.capacity);
    }
}

void ClockwiseParams::load(bool force)
{
    if (loaded && !force) {
        return;
    }

    for (const BoolSettingSchema &setting : BOOL_SETTINGS) {
        setting.setValue(*this, preferences.getBool(setting.preferenceKey, setting.defaultValue));
    }

    for (const UIntSettingSchema &setting : UINT_SETTINGS) {
        setting.setValue(*this, preferences.getUInt(setting.preferenceKey, setting.defaultValue));
    }

    for (const StringSettingSchema &setting : STRING_SETTINGS) {
        char buffer[MANUAL_POSIX_CAPACITY] = {0};
        copyText(buffer, sizeof(buffer), setting.defaultValue);
        preferences.getString(setting.preferenceKey, buffer, setting.capacity);
        setting.setValue(*this, buffer);
    }

    loaded = true;
}
