/**
 * Project-local WiFiManager string overrides.
 * Edit this file to customize the captive portal texts.
 */

#ifndef _WM_STRINGS_TR_H_
#define _WM_STRINGS_TR_H_

#include "wm_consts_en.h"

const char WM_LANGUAGE[] PROGMEM = "tr-TR";

const char HTTP_HEAD_START[]       PROGMEM = "<!DOCTYPE html>"
"<html lang='tr'><head>"
"<meta name='format-detection' content='telephone=no'>"
"<meta charset='UTF-8'>"
"<meta  name='viewport' content='width=device-width,initial-scale=1,user-scalable=no'/>"
"<title>{v}</title>";

const char HTTP_SCRIPT[]           PROGMEM = "<script>function c(l){"
"document.getElementById('s').value=l.getAttribute('data-ssid')||l.innerText||l.textContent;"
"p = l.nextElementSibling.classList.contains('l');"
"document.getElementById('p').disabled = !p;"
"if(p)document.getElementById('p').focus();};"
"function f() {var x = document.getElementById('p');x.type==='password'?x.type='text':x.type='password';}"
"</script>";

const char HTTP_HEAD_END[]         PROGMEM = "</head><body class='{c}'><div class='wrap'>";
const char HTTP_ROOT_MAIN[]        PROGMEM = "<h1>{t}</h1><h3>{v}</h3>";

const char * const HTTP_PORTAL_MENU[] PROGMEM = {
"<form action='/wifi'    method='get'><button>Wi-Fi Ayarla</button></form><br/>\n",
"<form action='/0wifi'   method='get'><button>Wi-Fi Ayarla (Tarama Yok)</button></form><br/>\n",
"<form action='/info'    method='get'><button>Bilgi</button></form><br/>\n",
"<form action='/param'   method='get'><button>Ayarlar</button></form><br/>\n",
"<form action='/close'   method='get'><button>Kapat</button></form><br/>\n",
"<form action='/restart' method='get'><button>Yeniden Başlat</button></form><br/>\n",
"<form action='/exit'    method='get'><button>Çıkış</button></form><br/>\n",
"<form action='/erase'   method='get'><button class='D'>Wi-Fi Ayarlarini Sil</button></form><br/>\n",
"<form action='/update'  method='get'><button>Güncelle</button></form><br/>\n",
"<hr><br/>"
};

const char HTTP_PORTAL_OPTIONS[]   PROGMEM = "";
const char HTTP_ITEM_QI[]          PROGMEM = "<div role='img' aria-label='{r}%' title='{r}%' class='q q-{q} {i} {h}'></div>";
const char HTTP_ITEM_QP[]          PROGMEM = "<div class='q {h}'>{r}%</div>";
const char HTTP_ITEM[]             PROGMEM = "<div><a href='#p' onclick='c(this)' data-ssid='{V}'>{v}</a>{qi}{qp}</div>";

const char HTTP_FORM_START[]       PROGMEM = "<form method='POST' action='{v}'>";
const char HTTP_FORM_WIFI[]        PROGMEM = "<label for='s'>SSID</label><input id='s' name='s' maxlength='32' autocorrect='off' autocapitalize='none' placeholder='{v}'><br/><label for='p'>Şifre</label><input id='p' name='p' maxlength='64' type='password' placeholder='{p}'><input type='checkbox' id='showpass' onclick='f()'> <label for='showpass'>Şifreyi Göster</label><br/>";
const char HTTP_FORM_WIFI_END[]    PROGMEM = "";
const char HTTP_FORM_STATIC_HEAD[] PROGMEM = "<hr><br/>";
const char HTTP_FORM_END[]         PROGMEM = "<br/><br/><button type='submit'>Kaydet</button></form>";
const char HTTP_FORM_LABEL[]       PROGMEM = "<label for='{i}'>{t}</label>";
const char HTTP_FORM_PARAM_HEAD[]  PROGMEM = "<hr><br/>";
const char HTTP_FORM_PARAM[]       PROGMEM = "<br/><input id='{i}' name='{n}' maxlength='{l}' value='{v}' {c}>\n";

const char HTTP_SCAN_LINK[]        PROGMEM = "<br/><form action='/wifi?refresh=1' method='POST'><button name='refresh' value='1'>Yenile</button></form>";
const char HTTP_SAVED[]            PROGMEM = "<div class='msg'>Bilgiler kaydediliyor.<br/>Cihaz seçilen ağa bağlanmayı deneyecek.<br />Baglanamazsa AP moduna geri donup tekrar deneyin.</div>";
const char HTTP_PARAMSAVED[]       PROGMEM = "<div class='msg S'>Ayarlar kaydedildi<br/></div>";
const char HTTP_END[]              PROGMEM = "</div></body></html>";
const char HTTP_ERASEBTN[]         PROGMEM = "<br/><form action='/erase' method='get'><button class='D'>Wi-Fi Ayarlarını Sil</button></form>";
const char HTTP_UPDATEBTN[]        PROGMEM = "<br/><form action='/update' method='get'><button>Güncelle</button></form>";
const char HTTP_BACKBTN[]          PROGMEM = "<hr><br/><form action='/' method='get'><button>Geri</button></form>";

const char HTTP_STATUS_ON[]        PROGMEM = "<div class='msg S'><strong>Bağlı</strong> : {v}<br/><em><small>IP {i}</small></em></div>";
const char HTTP_STATUS_OFF[]       PROGMEM = "<div class='msg {c}'><strong>Bağlı degil</strong> : {v}{r}</div>";
const char HTTP_STATUS_OFFPW[]     PROGMEM = "<br/>Kimlik doğrulama hatası";
const char HTTP_STATUS_OFFNOAP[]   PROGMEM = "<br/>Ağ bulunamadı";
const char HTTP_STATUS_OFFFAIL[]   PROGMEM = "<br/>Baglantı kurulamadı";
const char HTTP_STATUS_NONE[]      PROGMEM = "<div class='msg'>Kayıtlı ağ yok</div>";
const char HTTP_BR[]               PROGMEM = "<br/>";

const char HTTP_STYLE[]            PROGMEM = "<style>"
".c,body{text-align:center;font-family:verdana}div,input,select{padding:5px;font-size:1em;margin:5px 0;box-sizing:border-box}"
"input,button,select,.msg{border-radius:.3rem;width: 100%}input[type=radio],input[type=checkbox]{width:auto}"
"button,input[type='button'],input[type='submit']{cursor:pointer;border:0;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%}"
"input[type='file']{border:1px solid #1fa3ec}"
".wrap {text-align:left;display:inline-block;min-width:260px;max-width:500px}"
"a{color:#000;font-weight:700;text-decoration:none}a:hover{color:#1fa3ec;text-decoration:underline}"
".q{height:16px;margin:0;padding:0 5px;text-align:right;min-width:38px;float:right}.q.q-0:after{background-position-x:0}.q.q-1:after{background-position-x:-16px}.q.q-2:after{background-position-x:-32px}.q.q-3:after{background-position-x:-48px}.q.q-4:after{background-position-x:-64px}.q.l:before{background-position-x:-80px;padding-right:5px}.ql .q{float:left}.q:after,.q:before{content:'';width:16px;height:16px;display:inline-block;background-repeat:no-repeat;background-position: 16px 0;"
"background-image:url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAGAAAAAQCAMAAADeZIrLAAAAJFBMVEX///8AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADHJj5lAAAAC3RSTlMAIjN3iJmqu8zd7vF8pzcAAABsSURBVHja7Y1BCsAwCASNSVo3/v+/BUEiXnIoXkoX5jAQMxTHzK9cVSnvDxwD8bFx8PhZ9q8FmghXBhqA1faxk92PsxvRc2CCCFdhQCbRkLoAQ3q/wWUBqG35ZxtVzW4Ed6LngPyBU2CobdIDQ5oPWI5nCUwAAAAASUVORK5CYII=');}"
"@media (-webkit-min-device-pixel-ratio: 2),(min-resolution: 192dpi){.q:before,.q:after {"
"background-image:url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAALwAAAAgCAMAAACfM+KhAAAALVBMVEX///8AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADAOrOgAAAADnRSTlMAESIzRGZ3iJmqu8zd7gKjCLQAAACmSURBVHgB7dDBCoMwEEXRmKlVY3L//3NLhyzqIqSUggy8uxnhCR5Mo8xLt+14aZ7wwgsvvPA/ofv9+44334UXXngvb6XsFhO/VoC2RsSv9J7x8BnYLW+AjT56ud/uePMdb7IP8Bsc/e7h8Cfk912ghsNXWPpDC4hvN+D1560A1QPORyh84VKLjjdvfPFm++i9EWq0348XXnjhhT+4dIbCW+WjZim9AKk4UZMnnCEuAAAAAElFTkSuQmCC');"
"background-size: 95px 16px;}}"
".msg{padding:20px;margin:20px 0;border:1px solid #eee;border-left-width:5px;border-left-color:#777}.msg h4{margin-top:0;margin-bottom:5px}.msg.P{border-left-color:#1fa3ec}.msg.P h4{color:#1fa3ec}.msg.D{border-left-color:#dc3630}.msg.D h4{color:#dc3630}.msg.S{border-left-color: #5cb85c}.msg.S h4{color: #5cb85c}"
"dt{font-weight:bold}dd{margin:0;padding:0 0 0.5em 0;min-height:12px}"
"td{vertical-align: top;}"
".h{display:none}"
"button{transition: 0s opacity;transition-delay: 3s;transition-duration: 0s;cursor: pointer}"
"button.D{background-color:#dc3630}"
"button:active{opacity:50% !important;cursor:wait;transition-delay: 0s}"
"body.invert{background-color:#060606;}"
"body.invert,body.invert a,body.invert h1 {color:#fff;}"
"body.invert .msg{color:#fff;background-color:#282828;border-top:1px solid #555;border-right:1px solid #555;border-bottom:1px solid #555;}"
"body.invert .q[role=img]{-webkit-filter:invert(1);filter:invert(1);}"
":disabled {opacity: 0.5;}"
"</style>";

#ifndef WM_NOHELP
const char HTTP_HELP[]             PROGMEM =
 "<br/><h3>Kullanılabilir Sayfalar</h3><hr>"
 "<table class='table'>"
 "<thead><tr><th>Sayfa</th><th>Açıklama</th></tr></thead><tbody>"
 "<tr><td><a href='/'>/</a></td>"
 "<td>Ana menu sayfası.</td></tr>"
 "<tr><td><a href='/wifi'>/wifi</a></td>"
 "<td>Wi-Fi taramasını ve bağlantı formunu gösterir.</td></tr>"
 "<tr><td><a href='/wifisave'>/wifisave</a></td>"
 "<td>Wi-Fi bilgilerini kaydeder ve cihazi ağda başlatmayı dener.</td></tr>"
 "<tr><td><a href='/param'>/param</a></td>"
 "<td>Ek parametreler sayfası.</td></tr>"
 "<tr><td><a href='/info'>/info</a></td>"
 "<td>Bilgi sayfası.</td></tr>"
 "<tr><td><a href='/u'>/u</a></td>"
 "<td>OTA güncelleme sayfasi.</td></tr>"
 "<tr><td><a href='/close'>/close</a></td>"
 "<td>Captive portal penceresini kapatır, portal çalışmaya devam eder.</td></tr>"
 "<tr><td>/exit</td>"
 "<td>Config portal modundan çıkar.</td></tr>"
 "<tr><td>/restart</td>"
 "<td>Cihazı yeniden başlatır.</td></tr>"
 "<tr><td>/erase</td>"
 "<td>Wi-Fi ayarlarını siler ve cihazı yeniden başlatır.</td></tr>"
 "</table>"
 "<p/>Github <a href='https://github.com/tzapu/WiFiManager'>https://github.com/tzapu/WiFiManager</a>.";
#else
const char HTTP_HELP[]             PROGMEM = "";
#endif

const char HTTP_UPDATE[] PROGMEM = "Yeni firmware yükleyin<br/><form method='POST' action='u' enctype='multipart/form-data' onchange=\"(function(el){document.getElementById('uploadbin').style.display = el.value=='' ? 'none' : 'initial';})(this)\"><input type='file' name='update' accept='.bin,application/octet-stream'><button id='uploadbin' type='submit' class='h D'>Guncelle</button></form><small><a href='http://192.168.4.1/update' target='_blank'>* Captive portal icinde calismayabilir; tarayicida http://192.168.4.1/update adresini acin</a></small>";
const char HTTP_UPDATE_FAIL[] PROGMEM = "<div class='msg D'><strong>Güncelleme başarısız!</strong><Br/>Cihazı yeniden başlatıp tekrar deneyin</div>";
const char HTTP_UPDATE_SUCCESS[] PROGMEM = "<div class='msg S'><strong>Güncelleme başarılı.</strong> <br/> Cihaz şimdi yeniden başlatılıyor...</div>";

#ifdef WM_JSTEST
const char HTTP_JS[] PROGMEM =
"<script>function postAjax(url, data, success) {"
"    var params = typeof data == 'string' ? data : Object.keys(data).map("
"            function(k){ return encodeURIComponent(k) + '=' + encodeURIComponent(data[k]) }"
"        ).join('&');"
"    var xhr = window.XMLHttpRequest ? new XMLHttpRequest() : new ActiveXObject(\"Microsoft.XMLHTTP\");"
"    xhr.open('POST', url);"
"    xhr.onreadystatechange = function() {"
"        if (xhr.readyState>3 && xhr.status==200) { success(xhr.responseText); }"
"    };"
"    xhr.setRequestHeader('X-Requested-With', 'XMLHttpRequest');"
"    xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');"
"    xhr.send(params);"
"    return xhr;}"
"postAjax('/status', 'p1=1&p2=Hello+World', function(data){ console.log(data); });"
"postAjax('/status', { p1: 1, p2: 'Hello World' }, function(data){ console.log(data); });"
"</script>";
#endif

#ifdef ESP32
const char HTTP_INFO_esphead[]    PROGMEM = "<h3>esp32</h3><hr><dl>";
const char HTTP_INFO_chiprev[]    PROGMEM = "<dt>Çip revizyonu</dt><dd>{1}</dd>";
const char HTTP_INFO_lastreset[]  PROGMEM = "<dt>Son reset nedeni</dt><dd>CPU0: {1}<br/>CPU1: {2}</dd>";
const char HTTP_INFO_aphost[]     PROGMEM = "<dt>Erişim noktası host adı</dt><dd>{1}</dd>";
const char HTTP_INFO_psrsize[]    PROGMEM = "<dt>PSRAM Boyutu</dt><dd>{1} byte</dd>";
const char HTTP_INFO_temp[]       PROGMEM = "<dt>Sıcaklık</dt><dd>{1} C&deg; / {2} F&deg;</dd>";
const char HTTP_INFO_hall[]       PROGMEM = "<dt>Hall</dt><dd>{1}</dd>";
#else
const char HTTP_INFO_esphead[]    PROGMEM = "<h3>esp8266</h3><hr><dl>";
const char HTTP_INFO_fchipid[]    PROGMEM = "<dt>Flash cip ID</dt><dd>{1}</dd>";
const char HTTP_INFO_corever[]    PROGMEM = "<dt>Core surumu</dt><dd>{1}</dd>";
const char HTTP_INFO_bootver[]    PROGMEM = "<dt>Boot surumu</dt><dd>{1}</dd>";
const char HTTP_INFO_lastreset[]  PROGMEM = "<dt>Son reset nedeni</dt><dd>{1}</dd>";
const char HTTP_INFO_flashsize[]  PROGMEM = "<dt>Gercek flash boyutu</dt><dd>{1} byte</dd>";
#endif

const char HTTP_INFO_memsmeter[]  PROGMEM = "<br/><progress value='{1}' max='{2}'></progress></dd>";
const char HTTP_INFO_memsketch[]  PROGMEM = "<dt>Bellek - Sketch boyutu</dt><dd>Kullanilan / Toplam byte<br/>{1} / {2}";
const char HTTP_INFO_freeheap[]   PROGMEM = "<dt>Bellek - Boş heap</dt><dd>{1} byte kullanilabilir</dd>";
const char HTTP_INFO_wifihead[]   PROGMEM = "<br/><h3>WiFi</h3><hr>";
const char HTTP_INFO_uptime[]     PROGMEM = "<dt>Çalışma süresi</dt><dd>{1} dk {2} sn</dd>";
const char HTTP_INFO_chipid[]     PROGMEM = "<dt>Çip ID</dt><dd>{1}</dd>";
const char HTTP_INFO_idesize[]    PROGMEM = "<dt>Flash boyutu</dt><dd>{1} byte</dd>";
const char HTTP_INFO_sdkver[]     PROGMEM = "<dt>SDK sürümü</dt><dd>{1}</dd>";
const char HTTP_INFO_cpufreq[]    PROGMEM = "<dt>CPU frekansı</dt><dd>{1}MHz</dd>";
const char HTTP_INFO_apip[]       PROGMEM = "<dt>Erişim noktası IP</dt><dd>{1}</dd>";
const char HTTP_INFO_apmac[]      PROGMEM = "<dt>Erişim noktası MAC</dt><dd>{1}</dd>";
const char HTTP_INFO_apssid[]     PROGMEM = "<dt>Erişim noktası SSID</dt><dd>{1}</dd>";
const char HTTP_INFO_apbssid[]    PROGMEM = "<dt>BSSID</dt><dd>{1}</dd>";
const char HTTP_INFO_stassid[]    PROGMEM = "<dt>İstasyon SSID</dt><dd>{1}</dd>";
const char HTTP_INFO_staip[]      PROGMEM = "<dt>İstasyon IP</dt><dd>{1}</dd>";
const char HTTP_INFO_stagw[]      PROGMEM = "<dt>İstasyon ağ geçidi</dt><dd>{1}</dd>";
const char HTTP_INFO_stasub[]     PROGMEM = "<dt>İstasyon alt ağ maskesi</dt><dd>{1}</dd>";
const char HTTP_INFO_dnss[]       PROGMEM = "<dt>DNS Sunucusu</dt><dd>{1}</dd>";
const char HTTP_INFO_host[]       PROGMEM = "<dt>Host adı</dt><dd>{1}</dd>";
const char HTTP_INFO_stamac[]     PROGMEM = "<dt>İstasyon MAC</dt><dd>{1}</dd>";
const char HTTP_INFO_conx[]       PROGMEM = "<dt>Bağlı</dt><dd>{1}</dd>";
const char HTTP_INFO_autoconx[]   PROGMEM = "<dt>Otomatik bağlantı</dt><dd>{1}</dd>";

const char HTTP_INFO_aboutver[]     PROGMEM = "<dt>WiFiManager</dt><dd>{1}</dd>";
const char HTTP_INFO_aboutarduino[] PROGMEM = "<dt>Arduino</dt><dd>{1}</dd>";
const char HTTP_INFO_aboutsdk[]     PROGMEM = "<dt>ESP-SDK/IDF</dt><dd>{1}</dd>";
const char HTTP_INFO_aboutdate[]    PROGMEM = "<dt>Derleme tarihi</dt><dd>{1}</dd>";

const char S_brand[]              PROGMEM = "WiFiManager";
const char S_debugPrefix[]        PROGMEM = "*wm:";
const char S_y[]                  PROGMEM = "Evet";
const char S_n[]                  PROGMEM = "Hayır";
const char S_enable[]             PROGMEM = "Etkin";
const char S_disable[]            PROGMEM = "Devre dışı";
const char S_GET[]                PROGMEM = "GET";
const char S_POST[]               PROGMEM = "POST";
const char S_NA[]                 PROGMEM = "Bilinmiyor";
const char S_passph[]             PROGMEM = "********";
const char S_titlewifisaved[]     PROGMEM = "Bilgiler kaydedildi";
const char S_titlewifisettings[]  PROGMEM = "Ayarlar kaydedildi";
const char S_titlewifi[]          PROGMEM = "Wi-Fi Kurulumu";
const char S_titleinfo[]          PROGMEM = "Bilgi";
const char S_titleparam[]         PROGMEM = "Ayarlar";
const char S_titleparamsaved[]    PROGMEM = "Ayarlar kaydedildi";
const char S_titleexit[]          PROGMEM = "Çıkış";
const char S_titlereset[]         PROGMEM = "Yeniden Başlat";
const char S_titleerase[]         PROGMEM = "Sil";
const char S_titleclose[]         PROGMEM = "Kapat";
const char S_options[]            PROGMEM = "Seçenekler";
const char S_nonetworks[]         PROGMEM = "Ağ bulunamadı. Tekrar taramak için yenileyin.";
const char S_staticip[]           PROGMEM = "Statik IP";
const char S_staticgw[]           PROGMEM = "Statik ağ geçidi";
const char S_staticdns[]          PROGMEM = "Statik DNS";
const char S_subnet[]             PROGMEM = "Alt ağ";
const char S_exiting[]            PROGMEM = "Çıkılıyor";
const char S_resetting[]          PROGMEM = "Modül birkaç saniye içinde yeniden başlatılacak.";
const char S_closing[]            PROGMEM = "Sayfayı kapatabilirsiniz, portal calışmaya devam edecek";
const char S_error[]              PROGMEM = "Bir hata oluştu";
const char S_notfound[]           PROGMEM = "Dosya bulunamadı\n\n";
const char S_uri[]                PROGMEM = "URI: ";
const char S_method[]             PROGMEM = "\nMetod: ";
const char S_args[]               PROGMEM = "\nArgumanlar: ";
const char S_parampre[]           PROGMEM = "param_";

const char D_blank[]              PROGMEM = "";
const char D_linebr[]             PROGMEM = "<br/>";
const char D_HR[]                 PROGMEM = "--------------------";

#ifdef ESP8266
const char S_ssidpre[]            PROGMEM = "ESP";
#elif defined(ESP32)
const char S_ssidpre[]            PROGMEM = "ESP32";
#else
const char S_ssidpre[]            PROGMEM = "WM";
#endif

#endif
