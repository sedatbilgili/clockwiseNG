#pragma once

#include <Arduino.h>

const char SETTINGS_PAGE_JS[] PROGMEM = R"JS(
let pendingSaves = 0;
let restartQueued = false;
let pendingReadRequest = null;

function formatKiB(bytes) {
  return (Number(bytes || 0) / 1024).toFixed(1) + " KB";
}

function buildSystemStatus(settings) {
  const ramFree = Number(settings.ramfree || 0);
  const ramTotal = Number(settings.ramtotal || 0);
  const sketchUsed = Number(settings.sketchused || 0);
  const sketchFree = Number(settings.sketchfree || 0);
  const flashSize = Number(settings.flashsize || 0);
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

function normalizeSettings(settings) {
  const normalized = {};
  Object.keys(settings || {}).forEach((key) => {
    normalized[String(key).toLowerCase()] = settings[key];
  });
  return normalized;
}

function setSelectValue(id, value) {
  const element = document.getElementById(id);
  if (element) {
    element.value = String(value);
  }
}

function fillSettings(settings) {
  document.getElementById("bright").value = settings.displaybright || 0;
  document.getElementById("rangevalue").value = settings.displaybright || 0;
  document.getElementById("use24h").checked = String(settings.use24hformat) === "1" || settings.use24hformat === true;
  document.getElementById("tz").value = settings.timezone || "";
  document.getElementById("ntp").value = settings.ntpserver || "";
  document.getElementById("wifiSsid").value = settings.wifissid || "";
  document.getElementById("autoBrightMin").value = settings.autobrightmin || 0;
  document.getElementById("autoBrightMax").value = settings.autobrightmax || 0;
  document.getElementById("animationEnabled").checked = String(settings.animenabled) === "1" || settings.animenabled === true;
  document.getElementById("walkingMario").checked = String(settings.walkingmario) === "1" || settings.walkingmario === true;
  document.getElementById("cloudSpeed").value = settings.cloudspeed || 10;
  document.getElementById("cloudSpeedValue").value = (Number(settings.cloudspeed || 10) / 10).toFixed(1).replace(".", ",") + " sn";
  document.getElementById("ldrPin").value = settings.ldrpin || 35;
  document.getElementById("posixString").value = settings.manualposix || "";
  setSelectValue("rotation", settings.displayrotation || 0);
  setSelectValue("dynamicSkyMode", settings.dynsky || 0);
  setSelectValue("screenMode", settings.screenmode || 0);
  setSelectValue("characterSelection", settings.charsel || 0);
  document.getElementById("ssid").textContent = "Wi-Fi: " + (settings.wifissid || "");
  document.getElementById("fw-version").innerHTML =
    "Firmware v" + (settings.cw_fw_version || "") + " - Sedat Bilgili" +
    "<br><span class=\"muted\">" + buildSystemStatus(settings) + "</span>";

  syncWalkingMarioState();
}

function updatePreference(key, value) {
  pendingSaves++;
  const xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function () {
    if (this.readyState !== 4) {
      return;
    }
    pendingSaves = Math.max(0, pendingSaves - 1);
    if (this.status >= 200 && this.status < 300) {
      showStatus("Kaydedildi! Cihazi yeniden baslatin!", false);
    } else {
      showStatus("Kaydedilemedi.", true);
    }
    if (restartQueued && pendingSaves === 0) {
      restartQueued = false;
      restartDeviceNow();
    }
  };
  xhr.open("POST", "/set?" + key + "=" + value);
  xhr.send();
}

function saveAutoBrightness() {
  const minValue = Number(autoBrightMin.value);
  const maxValue = Number(autoBrightMax.value);
  if (minValue >= maxValue) {
    showStatus("Minimum deger maksimum degerden kucuk olmali.", true);
    return;
  }
  updatePreference("autoBright", autoBrightMin.value.padStart(4, "0") + "," + autoBrightMax.value.padStart(4, "0"));
}

function saveAnimationEnabled() {
  const enabled = document.getElementById("animationEnabled").checked;
  if (!enabled) {
    document.getElementById("walkingMario").checked = false;
  }
  syncWalkingMarioState();
  updatePreference("animEnabled", Number(enabled));
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
  updatePreference("walkingMario", Number(walkingMario.checked));
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
  updatePreference("wifiSsid", ssid);
  updatePreference("wifiPwd", password);
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
      showStatus("HTTP OTA baslatilamadi.", true);
    }
  };
  xhr.open("POST", "/http-ota?url=" + encodeURIComponent(url));
  xhr.send();
}

function splitHeaders(request) {
  const raw = request.getAllResponseHeaders().trim();
  const headerMap = {};
  if (!raw) {
    return headerMap;
  }
  raw.split(/[\r\n]+/).forEach((line) => {
    const parts = line.split(": ");
    const header = parts.shift().toLowerCase();
    const value = parts.join(": ");
    headerMap[header] = value;
  });
  return headerMap;
}

function requestGet(path, cb) {
  if (pendingReadRequest && pendingReadRequest.readyState !== 4) {
    pendingReadRequest.abort();
  }

  const xhr = new XMLHttpRequest();
  pendingReadRequest = xhr;
  xhr.onreadystatechange = function () {
    if (this.readyState === 4 && this.status === 204) {
      cb(this);
      pendingReadRequest = null;
    } else if (this.readyState === 4) {
      pendingReadRequest = null;
    }
  };
  xhr.open("GET", path + (path.indexOf("?") >= 0 ? "&" : "?") + "_ts=" + Date.now(), true);
  xhr.setRequestHeader("Cache-Control", "no-cache");
  xhr.send();
}

function requestJson(path, cb) {
  const xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function () {
    if (this.readyState !== 4) {
      return;
    }
    if (this.status < 200 || this.status >= 300) {
      showStatus("Ayarlar alinamadi.", true);
      return;
    }
    try {
      cb(JSON.parse(this.responseText));
    } catch (err) {
      const preview = String(this.responseText || "").slice(0, 120);
      showStatus("Ayar yaniti okunamadi. " + preview, true);
    }
  };
  xhr.open("GET", path, true);
  xhr.send();
}

function readPin(pin) {
  requestGet("/read?pin=" + pin, (req) => {
    const headers = splitHeaders(req);
    document.getElementById("ldrPinRead").textContent = headers["x-pin"] || "0";
  });
}

function restartDeviceNow() {
  const xhr = new XMLHttpRequest();
  xhr.open("POST", "/restart");
  xhr.send();
}

function restartDevice() {
  if (pendingSaves > 0) {
    restartQueued = true;
    showStatus("Kayit islemi tamamlaniyor, sonra yeniden baslatilacak.", false);
    return;
  }
  restartDeviceNow();
}

requestJson("/get", (settings) => {
  fillSettings(normalizeSettings(settings));
});
)JS";
