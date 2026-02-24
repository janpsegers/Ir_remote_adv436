#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <IRremote.hpp>

// ================= WIFI =================
const char* AP_SSID = "NEC_REMOTE";
const char* AP_PASS = "12345678";

IPAddress apIP(192,168,4,1);
DNSServer dnsServer;
ESP8266WebServer server(80);

// ================= IR =================
#define IR_SEND_PIN D2

// ================= STATUS =================
uint8_t currentAddress = 0x01;   // default 01
uint8_t currentCommand = 0x00;
bool sendingActive = false;
bool sendPulse = false;
unsigned long lastSendTime = 0;

// =====================================================
// ================= CAPTIVE SCREEN ====================
// =====================================================
void handleCaptive() {

  String page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body { font-family: Arial; text-align:center; margin-top:50px; }
button { font-size:22px; padding:15px; }
</style>
</head>
<body>

<h2>NEC Remote</h2>
<p>WiFi verbonden.</p>
<p>Druk op DOORSTART om volledige browser te openen.</p>

<a href="http://necremote">
<button>DOORSTART</button>
</a>

</body>
</html>
)rawliteral";

  server.send(200, "text/html", page);
}

// =====================================================
// ================= MAIN PAGE =========================
// =====================================================
void handleRoot() {

  String page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body { font-family: Arial; text-align:center; }
input { font-size:20px; width:80px; text-align:center; }
button { font-size:20px; padding:10px; margin:10px; }
#led {
  width:20px;
  height:20px;
  border-radius:50%;
  background:#400;
  margin:20px auto;
}
.active { background:#0f0; }
</style>

<script>
function refreshStatus(){
 fetch('/status')
  .then(r=>r.text())
  .then(t=>{
    if(t=="1")
      document.getElementById("led").classList.add("active");
    else
      document.getElementById("led").classList.remove("active");
  });
}
setInterval(refreshStatus,500);
</script>

</head>
<body>

<h2>NEC IR Remote</h2>

<form action="/start">
Adres (hex):<br>
<input name="adr" value="%ADR%" maxlength="2"><br><br>
CMD (hex):<br>
<input name="cmd" maxlength="2"><br><br>
<button type="submit">START</button>
</form>

<form action="/stop">
<button type="submit">STOP</button>
</form>

<div id="led"></div>

</body>
</html>
)rawliteral";

  char adrBuf[3];
  sprintf(adrBuf, "%02X", currentAddress);
  page.replace("%ADR%", adrBuf);

  server.send(200, "text/html", page);
}

// =====================================================
// ================= START =============================
// =====================================================
void handleStart() {

  if (server.hasArg("adr")) {
    String adrStr = server.arg("adr");
    currentAddress = (uint8_t) strtol(adrStr.c_str(), NULL, 16);
  }

  if (server.hasArg("cmd")) {
    String cmdStr = server.arg("cmd");
    currentCommand = (uint8_t) strtol(cmdStr.c_str(), NULL, 16);
  }

  sendingActive = true;

  server.sendHeader("Location", "/");
  server.send(303);
}

// =====================================================
// ================= STOP ==============================
// =====================================================
void handleStop() {
  sendingActive = false;
  server.sendHeader("Location", "/");
  server.send(303);
}

// =====================================================
// ================= STATUS API ========================
// =====================================================
void handleStatus() {
  if (sendPulse) {
    server.send(200, "text/plain", "1");
    sendPulse = false;
  } else {
    server.send(200, "text/plain", "0");
  }
}

// =====================================================
// ================= SETUP =============================
// =====================================================
void setup() {

  Serial.begin(115200);

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255,255,255,0));
  WiFi.softAP(AP_SSID, AP_PASS);

  dnsServer.start(53, "*", apIP);

  IrSender.begin(IR_SEND_PIN);

  // Routes
  server.on("/", handleRoot);
  server.on("/start", handleStart);
  server.on("/stop", handleStop);
  server.on("/status", handleStatus);

  // Captive detect routes (iOS / Android)
  server.on("/generate_204", handleCaptive);
  server.on("/fwlink", handleCaptive);
  server.on("/hotspot-detect.html", handleCaptive);

  server.onNotFound(handleCaptive);

  server.begin();

  Serial.println("NEC Remote gestart");
  Serial.println(WiFi.softAPIP());
}

// =====================================================
// ================= LOOP ==============================
// =====================================================
void loop() {

  dnsServer.processNextRequest();
  server.handleClient();

  if (sendingActive && millis() - lastSendTime > 1000) {
    lastSendTime = millis();

    IrSender.sendNEC(currentAddress, currentCommand, 0);

    Serial.print("Send NEC adr 0x");
    Serial.print(currentAddress, HEX);
    Serial.print(" cmd 0x");
    Serial.println(currentCommand, HEX);

    sendPulse = true;
  }
}