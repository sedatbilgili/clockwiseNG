#pragma once

#include <Arduino.h>

const char SETTINGS_PAGE[] PROGMEM = R""""(
<!DOCTYPE html>
<html>
<head>
<title>Clockwise Settings</title>
<meta name="viewport" content="width=device-width, initial-scale=1" charset="UTF-8">
<link rel="icon" href="data:,">
<style>
  * { box-sizing: border-box; }
  body {
    margin: 0;
    padding: 16px;
    font-family: sans-serif;
    background: #f3f4f6;
    color: #111827;
  }
  .shell {
    max-width: 920px;
    margin: 0 auto;
  }
  h1 {
    margin: 0 0 8px 0;
    font-size: 24px;
  }
  .meta {
    margin-bottom: 16px;
    font-size: 14px;
  }
  #status {
    display: none;
    margin-bottom: 16px;
    padding: 10px 12px;
    border: 1px solid #cbd5e1;
    background: #ecfdf5;
  }
  #status.error {
    background: #fef2f2;
    border-color: #fca5a5;
  }
  .section {
    margin-bottom: 12px;
    padding: 14px;
    background: #ffffff;
    border: 1px solid #d1d5db;
  }
  .section h2 {
    margin: 0 0 12px 0;
    font-size: 17px;
  }
  .row {
    margin-bottom: 10px;
  }
  .row:last-child {
    margin-bottom: 0;
  }
  label {
    display: block;
    margin-bottom: 4px;
    font-size: 14px;
  }
  .hint {
    margin: 4px 0 0 0;
    font-size: 12px;
    color: #4b5563;
  }
  .checkbox {
    display: flex;
    align-items: center;
    gap: 8px;
  }
  .checkbox label {
    margin: 0;
  }
  input, select, button {
    width: 100%;
    padding: 8px 10px;
    font: inherit;
  }
  input[type="checkbox"] {
    width: auto;
  }
  input[type="range"] {
    padding: 0;
  }
  .inline {
    display: flex;
    gap: 8px;
  }
  .inline > * {
    flex: 1;
  }
  .actions {
    display: flex;
    gap: 8px;
    flex-wrap: wrap;
  }
  button {
    background: #111827;
    color: #fff;
    border: 0;
    cursor: pointer;
  }
  button:disabled {
    opacity: 0.5;
    cursor: not-allowed;
  }
  .muted {
    font-size: 12px;
    color: #4b5563;
  }
  @media (max-width: 640px) {
    body { padding: 12px; }
    .inline, .actions { display: block; }
    .inline > *, .actions > * { margin-bottom: 8px; }
    .inline > *:last-child, .actions > *:last-child { margin-bottom: 0; }
  }
</style>
</head>
<body>
  <div class="shell">
    <h1>Clockwise Settings</h1>
    <div id="fw-version" class="meta"></div>
    <div id="ssid" class="meta"></div>
    <div id="status"></div>

    <div class="section">
      <h2>Device</h2>
      <div class="actions">
        <button type="button" onclick="restartDevice();">Yeniden Baslat</button>
      </div>
    </div>

    <div class="section">
      <h2>Display</h2>
      <div class="row">
        <label for="bright">Ekran Parlakligi: <strong><output id="rangevalue">0</output></strong></label>
        <input id="bright" type="range" min="0" max="255" oninput="rangevalue.value=value">
        <button type="button" onclick="updatePreference('displayBright', bright.value)">Kaydet</button>
      </div>
      <div class="row">
        <label for="rotation">Ekran Yonu</label>
        <select id="rotation">
          <option value="0">0</option>
          <option value="1">90</option>
          <option value="2">180</option>
          <option value="3">270</option>
        </select>
        <button type="button" onclick="updatePreference('displayRotation', rotation.value)">Kaydet</button>
      </div>
      <div class="row">
        <label for="screenMode">Ekran Modu</label>
        <select id="screenMode">
          <option value="0">Otomatik</option>
          <option value="1">Kapali</option>
          <option value="2">Acik</option>
        </select>
        <button type="button" onclick="updatePreference('screenMode', screenMode.value)">Kaydet</button>
      </div>
      <div class="row">
        <label for="characterSelection">Karakter</label>
        <select id="characterSelection">
          <option value="0">Mario</option>
          <option value="1">Luigi</option>
        </select>
        <button type="button" onclick="updatePreference('charSel', characterSelection.value)">Kaydet</button>
      </div>
    </div>

    <div class="section">
      <h2>Animation</h2>
      <div class="row">
        <div class="checkbox">
          <input id="animationEnabled" type="checkbox">
          <label for="animationEnabled">Genel animasyon acik</label>
        </div>
        <button type="button" onclick="saveAnimationEnabled()">Kaydet</button>
      </div>
      <div class="row">
        <div class="checkbox">
          <input id="walkingMario" type="checkbox">
          <label for="walkingMario">Yurume animasyonu acik</label>
        </div>
        <button id="walkingMarioButton" type="button" onclick="saveWalkingMario()">Kaydet</button>
      </div>
      <div class="row">
        <label for="dynamicSkyMode">Dinamik Gokyuzu</label>
        <select id="dynamicSkyMode">
          <option value="0">Gunduz</option>
          <option value="1">Gece</option>
          <option value="2">Otomatik</option>
        </select>
        <button type="button" onclick="updatePreference('dynSky', dynamicSkyMode.value)">Kaydet</button>
      </div>
      <div class="row">
        <label for="cloudSpeed">Animasyon Hizi: <strong><output id="cloudSpeedValue">1,0 sn</output></strong></label>
        <input id="cloudSpeed" type="range" min="1" max="30" step="1" oninput="cloudSpeedValue.value=(value/10).toFixed(1).replace('.', ',') + ' sn'">
        <div class="hint">0,1 sn ile 3,0 sn arasi</div>
        <button type="button" onclick="updatePreference('cloudSpeed', cloudSpeed.value)">Kaydet</button>
      </div>
    </div>

    <div class="section">
      <h2>Time</h2>
      <div class="row">
        <div class="checkbox">
          <input id="use24h" type="checkbox">
          <label for="use24h">24 saat formatini kullan</label>
        </div>
        <button type="button" onclick="updatePreference('use24hFormat', Number(use24h.checked))">Kaydet</button>
      </div>
      <div class="row">
        <label for="tz">Zaman Dilimi</label>
        <input id="tz" type="text" placeholder="Europe/Istanbul">
        <button type="button" onclick="updatePreference('timeZone', tz.value)">Kaydet</button>
      </div>
      <div class="row">
        <label for="ntp">NTP Sunucu</label>
        <input id="ntp" type="text" placeholder="pool.ntp.org">
        <button type="button" onclick="updatePreference('ntpServer', ntp.value)">Kaydet</button>
      </div>
      <div class="row">
        <label for="posixString">Manual Posix</label>
        <input id="posixString" type="text" placeholder="Opsiyonel">
        <button type="button" onclick="updatePreference('manualPosix', posixString.value)">Kaydet</button>
      </div>
    </div>

    <div class="section">
      <h2>Wi-Fi</h2>
      <div class="row">
        <label for="wifiSsid">SSID</label>
        <input id="wifiSsid" type="text">
      </div>
      <div class="row">
        <label for="wifiPassword">Sifre</label>
        <input id="wifiPassword" type="password" placeholder="Yeni sifre">
      </div>
      <button type="button" onclick="saveWifiCredentials()">Kaydet ve Yeniden Baslat</button>
    </div>

    <div class="section">
      <h2>Auto Brightness</h2>
      <div class="row inline">
        <div>
          <label for="autoBrightMin">Minimum</label>
          <input id="autoBrightMin" type="number" min="0" max="4095">
        </div>
        <div>
          <label for="autoBrightMax">Maksimum</label>
          <input id="autoBrightMax" type="number" min="0" max="4095">
        </div>
      </div>
      <button type="button" onclick="saveAutoBrightness()">Kaydet</button>
    </div>

    <div class="section">
      <h2>LDR</h2>
      <div class="row">
        <label for="ldrPin">LDR Pini</label>
        <input id="ldrPin" type="number" min="0" max="39">
      </div>
      <div class="actions">
        <button type="button" onclick="updatePreference('ldrPin', ldrPin.value)">Kaydet</button>
        <button type="button" onclick="readPin(ldrPin.value)">Pin Degerini Oku</button>
      </div>
      <div class="hint">Okunan deger: <strong id="ldrPinRead">0</strong></div>
    </div>

    <div class="section">
      <h2>HTTP OTA</h2>
      <div class="row">
        <label for="httpOtaUrl">Firmware URL</label>
        <input id="httpOtaUrl" type="url" placeholder="https://example.com/firmware.bin">
      </div>
      <button type="button" onclick="startHttpOta()">Indir ve Kur</button>
    </div>

  </div>
  <script src="/clockwise-settings.js"></script>
</body>
</html>
)"""";
