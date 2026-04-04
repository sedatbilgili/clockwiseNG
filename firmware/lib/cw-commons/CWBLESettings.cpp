#include "CWBLESettings.h"

#include <ArduinoJson.h>
#include <BLEDevice.h>
#include <CWPreferences.h>

namespace {
  static const char *BLE_DEVICE_NAME = "Clockwise";
  static const char *SERVICE_UUID = "b7f7f26e-c971-4f7c-8e85-47e0fcd4c001";
  static const char *SETTINGS_UUID = "b7f7f26e-c971-4f7c-8e85-47e0fcd4c002";
  static const char *STATUS_UUID = "b7f7f26e-c971-4f7c-8e85-47e0fcd4c003";

  BLEServer *g_server = nullptr;
  BLEService *g_service = nullptr;
  BLECharacteristic *g_settingsCharacteristic = nullptr;
  BLECharacteristic *g_statusCharacteristic = nullptr;

  String buildResultJson(bool ok, const String &message = "")
  {
    DynamicJsonDocument doc(192);
    doc["type"] = "result";
    doc["ok"] = ok;
    if (message.length() > 0) {
      doc["message"] = message;
    }

    String json;
    serializeJson(doc, json);
    return json;
  }

  String buildSettingsJson()
  {
    ClockwiseParams *params = ClockwiseParams::getInstance();
    DynamicJsonDocument doc(1024);
    JsonObject data = doc.createNestedObject("data");

    doc["type"] = "settings";
    data["bright"] = params->displayBright;
    data["use24h"] = params->use24hFormat;
    data["tz"] = params->timeZone;
    data["ntp"] = params->ntpServer;
    data["abMin"] = params->autoBrightMin;
    data["abMax"] = params->autoBrightMax;
    data["anim"] = params->animationEnabled;
    data["skyMode"] = params->dynamicSkyMode;
    data["screenMode"] = params->screenMode;
    data["character"] = params->characterSelection;
    data["cloudSpeed"] = params->cloudSpeed;
    data["ldrPin"] = params->ldrPin;
    data["posix"] = params->manualPosix;
    data["rotation"] = params->displayRotation;
    data["wifiSsid"] = params->wifiSsid;
    data["wifiPwd"] = params->wifiPwd;

    String json;
    serializeJson(doc, json);
    return json;
  }

  bool applySettingsObject(JsonObjectConst data, String &error)
  {
    ClockwiseParams *params = ClockwiseParams::getInstance();

    if (data.containsKey("bright")) params->displayBright = constrain((int)data["bright"], 0, 255);
    if (data.containsKey("use24h")) params->use24hFormat = data["use24h"].as<bool>();
    if (data.containsKey("tz")) params->timeZone = data["tz"].as<const char *>();
    if (data.containsKey("ntp")) params->ntpServer = data["ntp"].as<const char *>();
    if (data.containsKey("abMin")) params->autoBrightMin = constrain((int)data["abMin"], 0, 4095);
    if (data.containsKey("abMax")) params->autoBrightMax = constrain((int)data["abMax"], 0, 4095);
    if (data.containsKey("anim")) params->animationEnabled = data["anim"].as<bool>();
    if (data.containsKey("skyMode")) params->dynamicSkyMode = constrain((int)data["skyMode"], 0, 2);
    if (data.containsKey("screenMode")) params->screenMode = constrain((int)data["screenMode"], 0, 2);
    if (data.containsKey("character")) params->characterSelection = constrain((int)data["character"], 0, 1);
    if (data.containsKey("cloudSpeed")) params->cloudSpeed = constrain((int)data["cloudSpeed"], 1, 30);
    if (data.containsKey("ldrPin")) params->ldrPin = constrain((int)data["ldrPin"], 0, 39);
    if (data.containsKey("posix")) params->manualPosix = data["posix"].as<const char *>();
    if (data.containsKey("rotation")) params->displayRotation = constrain((int)data["rotation"], 0, 3);
    if (data.containsKey("wifiSsid")) params->wifiSsid = data["wifiSsid"].as<const char *>();
    if (data.containsKey("wifiPwd")) params->wifiPwd = data["wifiPwd"].as<const char *>();

    if (params->autoBrightMin >= params->autoBrightMax) {
      error = "abMin must be smaller than abMax";
      return false;
    }

    params->save();
    return true;
  }

  String applySettingsJson(const String &payload)
  {
    DynamicJsonDocument doc(1024);
    DeserializationError err = deserializeJson(doc, payload);
    if (err) {
      return buildResultJson(false, "invalid json");
    }

    JsonObjectConst data = doc["data"].is<JsonObjectConst>() ? doc["data"].as<JsonObjectConst>() : doc.as<JsonObjectConst>();
    if (data.isNull()) {
      return buildResultJson(false, "missing data object");
    }

    String error;
    if (!applySettingsObject(data, error)) {
      return buildResultJson(false, error);
    }

    return buildResultJson(true);
  }

  class SettingsCallbacks : public BLECharacteristicCallbacks
  {
    void onRead(BLECharacteristic *characteristic) override
    {
      ClockwiseParams::getInstance()->load();
      String json = buildSettingsJson();
      characteristic->setValue(json.c_str());
    }

    void onWrite(BLECharacteristic *characteristic) override
    {
      std::string rawValue = characteristic->getValue();
      String response = applySettingsJson(String(rawValue.c_str()));

      ClockwiseParams::getInstance()->load();
      String settingsJson = buildSettingsJson();
      characteristic->setValue(settingsJson.c_str());

      if (g_statusCharacteristic != nullptr) {
        g_statusCharacteristic->setValue(response.c_str());
        g_statusCharacteristic->notify();
      }
    }
  };
}

CWBLESettings *CWBLESettings::getInstance()
{
  static CWBLESettings base;
  return &base;
}

void CWBLESettings::begin()
{
  static bool started = false;
  if (started) {
    return;
  }
  started = true;

  BLEDevice::init(BLE_DEVICE_NAME);
  BLEDevice::setMTU(517);

  g_server = BLEDevice::createServer();
  g_service = g_server->createService(SERVICE_UUID);

  g_settingsCharacteristic = g_service->createCharacteristic(
    SETTINGS_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
  );
  g_settingsCharacteristic->setCallbacks(new SettingsCallbacks());
  g_settingsCharacteristic->setValue(buildSettingsJson().c_str());

  g_statusCharacteristic = g_service->createCharacteristic(
    STATUS_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  g_statusCharacteristic->setValue(buildResultJson(true, "ready").c_str());

  g_service->start();

  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->setScanResponse(true);
  BLEDevice::startAdvertising();

  Serial.println("[BLE] Settings service ready");
}
