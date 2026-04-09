## ClockwiseNG Web API Documentation

Bu dokuman, firmware'deki mevcut API davranisinin kod ile birebir uyumlu referansidir.

## 1) Genel Bakis

Cihaz, HTTP uzerinden JSON tabanli bir API saglar.

Aktif endpoint'ler:

- `GET /api/schema`
- `GET /api/state`
- `POST /api/settings`
- `POST /api/actions`
- `POST /api/upload-ota`

UI varlik endpoint'leri (data API disi):

- `GET /`
- `GET /clockwise-settings.js`
- `GET /favicon.ico`

Notlar:

- JSON endpoint yanitlarinda `Cache-Control: no-store` header'i gonderilir.
- Bulunamayan route'lar `404` ve `{"ok":false,"error":"not found"}` doner.

## 2) Hata Formati

Yapilandirilmis API hatalari su sekildedir:

```json
{
  "ok": false,
  "error": "error message"
}
```

Genel hata nedenleri:

- `missing request body`
- `invalid json`
- `unknown setting key`
- `unknown action`
- `missing url`
- `url must start with http:// or https://`
- `url too long`
- `missing pin`

## 3) `GET /api/schema`

Ayarlarin makinece okunabilir semasini doner. Frontend acilisinda ilk cagri olarak kullanilir.

### Request

```http
GET /api/schema
```

### Success response (`200`)

```json
{
  "device": "clockwise",
  "schemaVersion": 1,
  "settings": {
    "animationEnabled": {
      "type": "boolean",
      "default": true,
      "writable": true,
      "inState": true,
      "apply": "hot"
    },
    "goombaEnabled": {
      "type": "boolean",
      "default": true,
      "writable": true,
      "inState": true,
      "apply": "hot"
    },
    "displayBright": {
      "type": "integer",
      "default": 16,
      "min": 0,
      "max": 255,
      "writable": true,
      "inState": true,
      "apply": "hot"
    },
    "wifiPwd": {
      "type": "string",
      "default": "1234",
      "maxLength": 63,
      "writable": true,
      "inState": false,
      "sensitive": true,
      "apply": "restart"
    }
  }
}
```

Alanlar:

- `schemaVersion`: Su an `1`.
- `settings.<key>.type`: `boolean`, `integer`, `string`.
- `settings.<key>.inState`: Bu alan `GET /api/state` icinde donuyor mu.
- `settings.<key>.apply`:
  - `hot`: Degisiklik yeniden baslatma olmadan uygulanir.
  - `restart`: Degisiklik kalici olur ancak etkisi icin restart gerekir.
- `settings.<key>.sensitive`: Hassas alan (ornek: `wifiPwd`).

## 4) `GET /api/state`

Anlik durum + mevcut ayarlar + sistem metrikleri.

### Request

```http
GET /api/state
```

### Success response (`200`)

```json
{
  "device": "clockwise",
  "fwName": "clockwiseNG",
  "fwVersion": "3.2.3",
  "clockface": "cw-cf-0x01",
  "schemaVersion": 1,
  "ip": "192.168.1.120",
  "port": 80,

  "use24hFormat": true,
  "animationEnabled": true,
  "walkingMario": true,
  "goombaEnabled": true,

  "displayBright": 16,
  "autoBrightMin": 0,
  "autoBrightMax": 4095,
  "ldrPin": 34,
  "displayRotation": 0,
  "screenMode": 0,
  "character": 0,
  "cloudSpeed": 10,
  "dynamicSkyMode": 2,

  "timeZone": "Asia/Istanbul",
  "wifiSsid": "MyWiFi",
  "ntpServer": "time.google.com",
  "manualPosix": "",

  "ldrValue": 1720,
  "ramFree": 182304,
  "ramTotal": 327680,
  "sketchUsed": 1342176,
  "sketchFree": 1048576,
  "flashSize": 4194304
}
```

### Dikkat

- `wifiPwd` guvenlik nedeniyle `state` icinde DONMEZ.
- `ldrValue` kisa sureli cache'lenir (sensor okuma maliyeti icin).

## 5) `POST /api/settings`

Kalici ayarlari gunceller.

### Request

```http
POST /api/settings
Content-Type: application/json
```

```json
{
  "settings": {
    "displayBright": 24,
    "use24hFormat": true,
    "timeZone": "Europe/Istanbul",
    "animationEnabled": true,
    "walkingMario": true,
    "goombaEnabled": true
  }
}
```

Goomba cizimini kapatmak icin minimum ornek:

```json
{
  "settings": {
    "goombaEnabled": false
  }
}
```

### Success response (`200`)

```json
{
  "ok": true,
  "changed": true,
  "restartRequired": true,
  "applyMode": "restart"
}
```

Yanittaki alanlar:

- `changed`: En az bir ayar degeri degisti mi.
- `restartRequired`: Degisen ayarlar icinde restart gerektiren var mi.
- `applyMode`:
  - `hot`: Degisiklik(ler) hemen uygulandi.
  - `restart`: Uygulama icin restart gerekir.
  - `none`: Istek gecerli ama efektif degisiklik yok (no-op).

### Error response (ornek, `400`)

```json
{
  "ok": false,
  "error": "autoBrightMin must be smaller than autoBrightMax"
}
```

## 6) Ayar Referansi (Ne ise yarar?)

Asagidaki tablo `settings` objesi ile guncellenebilen alanlarin tamamini kapsar.

| Key | Type | Aralik / Uzunluk | Varsayilan | Apply | Durum / Etki |
|---|---|---|---|---|---|
| `displayBright` | integer | `0..255` | `16` | hot | Ekranin ust parlaklik limiti. Auto mod hesaplari bu tavani kullanir. |
| `autoBrightMin` | integer | `0..4095` | `0` | hot | LDR alt esigi. Bu degerin altinda ekran kararabilir/kapanabilir. |
| `autoBrightMax` | integer | `0..4095` | `4095` | hot | LDR ust esigi. Bu deger ve ustunde ust parlakliga yaklasilir. |
| `use24hFormat` | boolean | - | `true` | restart | Saat formati (24h / 12h). |
| `ldrPin` | integer | `0..39` | `34` | restart | LDR sensor okunacak ADC pin'i. |
| `timeZone` | string | max `63` | `Asia/Istanbul` | restart | IANA timezone (ornek `Europe/Istanbul`). |
| `wifiSsid` | string | max `63` | `wifi` | restart | Baglanilacak Wi-Fi SSID. |
| `wifiPwd` | string | max `63` | `1234` | restart | Wi-Fi sifresi. `state` yanitina dahil edilmez. |
| `ntpServer` | string | max `63` | `time.google.com` | restart | NTP sunucusu. |
| `manualPosix` | string | max `95` | `""` | restart | Opsiyonel manuel POSIX timezone ifadesi. |
| `displayRotation` | integer | `0..3` | `0` | restart | Ekran rotasyonu: `0,1,2,3` = `0,90,180,270` derece. |
| `animationEnabled` | boolean | - | `true` | hot | Global animasyon ac/kapat. `false` olursa `walkingMario` zorla `false` yapilir. |
| `screenMode` | integer | `0..2` | `0` | hot | Ekran modu: `0=Auto`, `1=Off`, `2=On`. |
| `character` | integer | `0..1` | `0` | hot | Karakter secimi: `0=Mario`, `1=Luigi`. |
| `cloudSpeed` | integer | `1..30` | `10` | hot | Animasyon hiz parametresi (UI tarafinda `0.1s..3.0s` gostergesi). |
| `dynamicSkyMode` | integer | `0..2` | `0` | hot | Gokyuzu modu: `0=Gunduz`, `1=Gece`, `2=Otomatik`. |
| `walkingMario` | boolean | - | `true` | hot | Yurumeye dayali karakter animasyonu ac/kapat. |
| `goombaEnabled` | boolean | - | `true` | hot | Goomba cizimini ac/kapat. `false` iken Goomba hic cizilmez. |

### Settings validasyon kurallari

- Integer alanlar guvenli araliga clamp edilir.
- `autoBrightMin < autoBrightMax` olmalidir.
- Type mismatch hata verir (`invalid integer/boolean/string setting value`).
- Bilinmeyen key hata verir (`unknown setting key`).

## 7) `POST /api/actions`

Ayar disi islemler icin kullanilir.

### Request

```http
POST /api/actions
Content-Type: application/json
```

## 7.1) `restart`

### Request body

```json
{
  "action": "restart"
}
```

### Response

- `204 No Content`
- Content-Type: `text/plain`
- Govde yok

Not: `restart` aksiyonu JSON donmez.

## 7.2) `httpOta`

Firmware URL'den OTA kurulumunu queue eder.

### Request body

```json
{
  "action": "httpOta",
  "url": "https://example.com/firmware.bin"
}
```

### Alternatif

`url` query param ile de gecilebilir. Ancak request body'de `action: "httpOta"` bulunmasi yine zorunludur:

```http
POST /api/actions?url=https%3A%2F%2Fexample.com%2Ffirmware.bin
```

### Success response (`202`)

```json
{
  "ok": true,
  "queued": true
}
```

### Error response (ornek)

```json
{
  "ok": false,
  "error": "missing url"
}
```

```json
{
  "ok": false,
  "error": "url must start with http:// or https://"
}
```

```json
{
  "ok": false,
  "error": "url too long"
}
```

Notlar:

- Bu endpoint indirmeyi sadece queue eder.
- Gercek transfer runtime loop'ta, Wi-Fi musaitken baslar.

## 7.3) `readPin`

ADC pin degeri okur (diagnostic).

### Request body

```json
{
  "action": "readPin",
  "pin": 34
}
```

### Success response (`200`)

```json
{
  "ok": true,
  "pin": 34,
  "value": 1728
}
```

### Error response

```json
{
  "ok": false,
  "error": "missing pin"
}
```

## 8) `POST /api/upload-ota`

Tarayicidan dogrudan `.bin` dosya yukleyerek OTA kurulumunu yapar.

### Request

```http
POST /api/upload-ota?size=<bytes>
Content-Type: multipart/form-data
```

Form alanlari:

- `firmware`: OTA `.bin` dosyasi

`size` query param'i opsiyoneldir ancak progress hesaplarinda yardimci olur.

### Success response (`200`)

```json
{
  "ok": true,
  "uploaded": true,
  "restarting": true
}
```

### Olasi hata yanitlari

```json
{
  "ok": false,
  "error": "upload still in progress"
}
```

```json
{
  "ok": false,
  "error": "no firmware uploaded"
}
```

```json
{
  "ok": false,
  "error": "Update.write failed (12)"
}
```

Notlar:

- Yazi islemi OTA partition'a stream edilerek yapilir.
- Basaridan sonra cihaz restart queue eder.

## 9) Beklenen istemci akislari

### Ilk yukleme

1. `GET /api/schema`
2. `GET /api/state`

### Ayar kaydetme

1. Degisen alanlari topla.
2. `POST /api/settings` ile gonder.
3. `applyMode` ve `restartRequired` alanlarina gore UI davranisini belirle.

### Aksiyonlar

- Restart: `POST /api/actions` (`{"action":"restart"}`)
- URL OTA: `POST /api/actions` (`{"action":"httpOta","url":"..."}`)
- Pin oku: `POST /api/actions` (`{"action":"readPin","pin":34}`)
- Dosya OTA: `POST /api/upload-ota` (multipart)

## 10) Eski endpointler

Asagidaki eski route'lar aktif JSON API tasariminin parcasi degildir:

- `/get`
- `/set`
- `/info`
- `/ldr`
- `/read`
- `/restart`
- `/http-ota`

Yeni istemciler bu endpoint'leri kullanmamalidir.
