#pragma once

#include <Arduino.h>

const char SETTINGS_PAGE_JS[] PROGMEM = R"JS(
let pendingSaves = 0;
let restartQueued = false;
let pendingReadRequest = null;
let settingsSchema = null;

function formatKiB(bytes) {
  return (Number(bytes || 0) / 1024).toFixed(1) + " KB";
}

function buildSystemStatus(settings) {
  const ramFree = Number(settings.ramFree || 0);
  const ramTotal = Number(settings.ramTotal || 0);
  const sketchUsed = Number(settings.sketchUsed || 0);
  const sketchFree = Number(settings.sketchFree || 0);
  const flashSize = Number(settings.flashSize || 0);
  return "RAM: " + formatKiB(ramFree) + " bos / " + formatKiB(ramTotal) +
    " | FLASH: " + formatKiB(sketchUsed) + " kod / " + formatKiB(sketchFree) + " OTA bos" +
    " | Toplam Flash: " + formatKiB(flashSize);
}

function showStatus(message, isError) {
  const status = document.getElementById("status");
  status.textContent = message;
  status.className = isError ? "error" : "";
  status.style.display = "block";
  setTimeout(() => {
    status.style.display = "none";
  }, 2500);
}

function setSelectValue(id, value) {
  const element = document.getElementById(id);
  if (element) {
    element.value = String(value);
  }
}

function getSchemaDefault(key, fallbackValue) {
  if (!settingsSchema || !settingsSchema.settings || !settingsSchema.settings[key]) {
    return fallbackValue;
  }
  const field = settingsSchema.settings[key];
  if (!("default" in field)) {
    return fallbackValue;
  }
  return field.default;
}

function getFirstDefined(settings, keys, fallbackValue) {
  for (const key of keys) {
    if (Object.prototype.hasOwnProperty.call(settings, key)) {
      return settings[key];
    }
  }
  return fallbackValue;
}

function toBoolean(value, fallbackValue) {
  if (typeof value === "boolean") {
    return value;
  }
  if (typeof value === "number") {
    return value !== 0;
  }
  if (typeof value === "string") {
    const normalized = value.trim().toLowerCase();
    if (normalized === "1" || normalized === "true" || normalized === "on" || normalized === "yes") {
      return true;
    }
    if (normalized === "0" || normalized === "false" || normalized === "off" || normalized === "no" || normalized === "") {
      return false;
    }
  }
  return fallbackValue;
}

function fillSettings(settings) {
  if (!settingsSchema || !settingsSchema.settings) {
    showStatus("Ayar semasi yuklenemedi.", true);
    return;
  }
  const animationEnabledValue = getFirstDefined(settings, ["animationEnabled", "animEnabled", "animationenabled", "animenabled"], getSchemaDefault("animationEnabled", true));
  const walkingMarioValue = getFirstDefined(settings, ["walkingMario", "walkingmario"], getSchemaDefault("walkingMario", true));
  const goombaEnabledValue = getFirstDefined(settings, ["goombaEnabled", "goombaenabled"], getSchemaDefault("goombaEnabled", true));

  document.getElementById("bright").value = settings.displayBright || 0;
  document.getElementById("rangevalue").value = settings.displayBright || 0;
  document.getElementById("use24h").checked = Boolean(settings.use24hFormat);
  document.getElementById("tz").value = settings.timeZone || "";
  document.getElementById("ntp").value = settings.ntpServer || "";
  document.getElementById("wifiSsid").value = settings.wifiSsid || "";
  document.getElementById("autoBrightMin").value = settings.autoBrightMin || 0;
  document.getElementById("autoBrightMax").value = settings.autoBrightMax || 0;
  document.getElementById("animationEnabled").checked = toBoolean(animationEnabledValue, true);
  document.getElementById("walkingMario").checked = toBoolean(walkingMarioValue, true);
  document.getElementById("goombaEnabled").checked = toBoolean(goombaEnabledValue, true);
  document.getElementById("cloudSpeed").value = settings.cloudSpeed || 10;
  document.getElementById("cloudSpeedValue").value = (Number(settings.cloudSpeed || 10) / 10).toFixed(1).replace(".", ",") + " sn";
  document.getElementById("ldrPin").value = settings.ldrPin || 35;
  document.getElementById("posixString").value = settings.manualPosix || "";
  setSelectValue("rotation", settings.displayRotation || 0);
  setSelectValue("dynamicSkyMode", settings.dynamicSkyMode || 0);
  setSelectValue("screenMode", settings.screenMode || 0);
  setSelectValue("characterSelection", settings.character || 0);
  document.getElementById("ssid").textContent = "Wi-Fi: " + (settings.wifiSsid || "");
  document.getElementById("fw-version").innerHTML =
    "Firmware v" + (settings.fwVersion || "") + " - Sedat Bilgili" +
    "<br><span class=\"muted\">" + buildSystemStatus(settings) + "</span>";

  syncWalkingMarioState();
}

function hasSchemaField(key) {
  return Boolean(settingsSchema && settingsSchema.settings && settingsSchema.settings[key]);
}

function validateOutgoingSettings(settings) {
  if (!settingsSchema || !settingsSchema.settings) {
    return "settings schema not loaded";
  }

  for (const key of Object.keys(settings)) {
    const field = settingsSchema.settings[key];
    if (!field) {
      return "unknown setting: " + key;
    }

    const value = settings[key];
    if (field.type === "boolean" && typeof value !== "boolean") {
      return "invalid boolean value for " + key;
    }
    if (field.type === "integer" && (!Number.isInteger(value))) {
      return "invalid integer value for " + key;
    }
    if (field.type === "string" && typeof value !== "string") {
      return "invalid string value for " + key;
    }
  }

  return "";
}

function validateState(settings) {
  if (!settingsSchema || !settingsSchema.settings) {
    return;
  }

  const missingKeys = [];
  for (const [key, field] of Object.entries(settingsSchema.settings)) {
    if (field.inState && !(key in settings)) {
      missingKeys.push(key);
    }
  }

  if (missingKeys.length > 0) {
    showStatus("Eksik state alanlari: " + missingKeys.join(", "), true);
  }
}

function saveSettings(settings) {
  const validationError = validateOutgoingSettings(settings);
  if (validationError) {
    showStatus(validationError, true);
    return;
  }

  pendingSaves++;
  const xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function () {
    if (this.readyState !== 4) {
      return;
    }
    pendingSaves = Math.max(0, pendingSaves - 1);
    if (this.status >= 200 && this.status < 300) {
      let payload = {};
      try {
        payload = JSON.parse(this.responseText || "{}");
      } catch (err) {
      }
      if (payload.restartRequired) {
        showStatus("Kaydedildi. Degisiklikler icin yeniden baslatin.", false);
      } else if (payload.changed) {
        showStatus("Kaydedildi ve aninda uygulandi.", false);
      } else {
        showStatus("Ayarlar zaten guncel.", false);
      }
    } else {
      let message = "Kaydedilemedi.";
      try {
        const payload = JSON.parse(this.responseText || "{}");
        if (payload.error) {
          message = payload.error;
        }
      } catch (err) {
      }
      showStatus(message, true);
    }
    if (restartQueued && pendingSaves === 0) {
      restartQueued = false;
      restartDeviceNow();
    }
  };
  xhr.open("POST", "/api/settings");
  xhr.setRequestHeader("Content-Type", "application/json");
  xhr.send(JSON.stringify({ settings }));
}

function updatePreference(key, value) {
  const settings = {};
  settings[key] = value;
  saveSettings(settings);
}

function saveAutoBrightness() {
  const minValue = Number(autoBrightMin.value);
  const maxValue = Number(autoBrightMax.value);
  if (minValue >= maxValue) {
    showStatus("Minimum deger maksimum degerden kucuk olmali.", true);
    return;
  }
  saveSettings({
    autoBrightMin: minValue,
    autoBrightMax: maxValue
  });
}

function saveAnimationEnabled() {
  const enabled = document.getElementById("animationEnabled").checked;
  if (!enabled) {
    document.getElementById("walkingMario").checked = false;
  }
  syncWalkingMarioState();
  updatePreference("animationEnabled", enabled);
}

function saveWalkingMario() {
  const animationEnabled = document.getElementById("animationEnabled").checked;
  const walkingMario = document.getElementById("walkingMario");
  if (!animationEnabled) {
    walkingMario.checked = false;
    syncWalkingMarioState();
    showStatus("Bu ayar icin once genel animasyonu acin.", true);
    return;
  }
  updatePreference("walkingMario", walkingMario.checked);
}

function saveGoombaEnabled() {
  const goombaEnabled = document.getElementById("goombaEnabled");
  updatePreference("goombaEnabled", goombaEnabled.checked);
}

function syncWalkingMarioState() {
  const animationEnabled = document.getElementById("animationEnabled");
  const walkingMario = document.getElementById("walkingMario");
  const walkingMarioButton = document.getElementById("walkingMarioButton");
  const enabled = animationEnabled.checked;
  walkingMario.disabled = !enabled;
  walkingMarioButton.disabled = !enabled;
  if (!enabled) {
    walkingMario.checked = false;
  }
}

function saveWifiCredentials() {
  const ssid = document.getElementById("wifiSsid").value.trim();
  const password = document.getElementById("wifiPassword").value;
  if (!ssid) {
    showStatus("SSID bos birakilamaz.", true);
    return;
  }
  saveSettings({
    wifiSsid: ssid,
    wifiPwd: password
  });
  restartQueued = true;
  showStatus("Wi-Fi bilgileri kaydediliyor, cihaz yeniden baslatilacak.", false);
}

function startHttpOta() {
  const url = document.getElementById("httpOtaUrl").value.trim();
  if (!url) {
    showStatus("Firmware URL bos birakilamaz.", true);
    return;
  }
  if (!(url.startsWith("http://") || url.startsWith("https://"))) {
    showStatus("URL http:// veya https:// ile baslamali.", true);
    return;
  }
  const xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function () {
    if (this.readyState !== 4) {
      return;
    }
    if (this.status === 202) {
      showStatus("HTTP OTA baslatildi.", false);
    } else {
      let message = "HTTP OTA baslatilamadi.";
      try {
        const payload = JSON.parse(this.responseText || "{}");
        if (payload.error) {
          message = payload.error;
        }
      } catch (err) {
      }
      showStatus(message, true);
    }
  };
  xhr.open("POST", "/api/actions?url=" + encodeURIComponent(url));
  xhr.setRequestHeader("Content-Type", "application/json");
  xhr.send(JSON.stringify({ action: "httpOta", url }));
}

function uploadFirmwareFile() {
  const fileInput = document.getElementById("otaFile");
  const file = fileInput.files && fileInput.files[0];
  if (!file) {
    showStatus("Lutfen bir .bin dosyasi secin.", true);
    return;
  }

  const formData = new FormData();
  formData.append("firmware", file, file.name);

  const xhr = new XMLHttpRequest();
  let lastProgressBucket = -1;
  xhr.upload.onprogress = function (event) {
    if (!event.lengthComputable) {
      return;
    }
    const percent = Math.max(0, Math.min(100, Math.round((event.loaded * 100) / event.total)));
    const bucket = Math.floor(percent / 10);
    if (bucket <= lastProgressBucket) {
      return;
    }
    lastProgressBucket = bucket;
    const shownPercent = Math.min(100, bucket * 10);
    showStatus("Firmware yukleniyor: %" + shownPercent, false);
  };
  xhr.onreadystatechange = function () {
    if (this.readyState !== 4) {
      return;
    }

    if (this.status >= 200 && this.status < 300) {
      showStatus("Firmware yuklendi. Cihaz yeniden baslatiliyor.", false);
      fileInput.value = "";
    } else {
      let message = "Firmware yuklenemedi.";
      try {
        const payload = JSON.parse(this.responseText || "{}");
        if (payload.error) {
          message = payload.error;
        }
      } catch (err) {
      }
      showStatus(message, true);
    }
  };
  xhr.open("POST", "/api/upload-ota?size=" + encodeURIComponent(String(file.size)));
  xhr.send(formData);
}

function requestJson(path, cb, options) {
  const xhr = new XMLHttpRequest();
  const method = (options && options.method) || "GET";
  const body = options && options.body ? JSON.stringify(options.body) : null;

  if (method === "GET") {
    if (pendingReadRequest && pendingReadRequest.readyState !== 4) {
      pendingReadRequest.abort();
    }
    pendingReadRequest = xhr;
  }

  xhr.onreadystatechange = function () {
    if (this.readyState !== 4) {
      return;
    }

    if (method === "GET") {
      pendingReadRequest = null;
    }

    if (this.status < 200 || this.status >= 300) {
      let message = "Istek basarisiz oldu.";
      try {
        const payload = JSON.parse(this.responseText || "{}");
        if (payload.error) {
          message = payload.error;
        }
      } catch (err) {
      }
      showStatus(message, true);
      return;
    }

    if (!this.responseText) {
      cb({});
      return;
    }

    try {
      cb(JSON.parse(this.responseText));
    } catch (err) {
      const preview = String(this.responseText || "").slice(0, 120);
      showStatus("Ayar yaniti okunamadi. " + preview, true);
    }
  };
  const requestPath = method === "GET" ? path + (path.indexOf("?") >= 0 ? "&" : "?") + "_ts=" + Date.now() : path;
  xhr.open(method, requestPath, true);
  if (method === "GET") {
    xhr.setRequestHeader("Cache-Control", "no-cache");
  } else {
    xhr.setRequestHeader("Content-Type", "application/json");
  }
  xhr.send(body);
}

function readPin(pin) {
  requestJson("/api/actions", (payload) => {
    document.getElementById("ldrPinRead").textContent = String(payload.value || 0);
  }, {
    method: "POST",
    body: { action: "readPin", pin: Number(pin) }
  });
}

function restartDeviceNow() {
  requestJson("/api/actions", () => {}, {
    method: "POST",
    body: { action: "restart" }
  });
}

function restartDevice() {
  if (pendingSaves > 0) {
    restartQueued = true;
    showStatus("Kayit islemi tamamlaniyor, sonra yeniden baslatilacak.", false);
    return;
  }
  restartDeviceNow();
}

requestJson("/api/schema", (schema) => {
  settingsSchema = schema;
  requestJson("/api/state", (settings) => {
    validateState(settings);
    fillSettings(settings);
  });
});
)JS";
