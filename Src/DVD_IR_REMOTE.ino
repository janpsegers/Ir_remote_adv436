#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>


// ===== Wi-Fi AP =====
const char* AP_SSID = "DVD_IR_REMOTE";
const char* AP_PASS = "12345678"; // open AP
ESP8266WebServer server(80);

// ===== IR =====
#define IR_SEND_PIN D2   // D2
#define NEC_ADDRESS 0x01
//#define USE_ACTIVE_LOW_OUTPUT_FOR_SEND_PIN // Reverses the polarity at the send pin.
//#define USE_OPEN_DRAIN_OUTPUT_FOR_SEND_PIN  // Use or simulate open drain output mode at send pin. Attention, active state of open drain is LOW, so connect the send LED between positive supply and send pin!
//#define SEND_PWM_BY_TIMER  
#include <IRremote.hpp>


// ===== Matrix pins =====
const uint8_t rowPins[3] = { D7, D6, D5 };   // R1, R2, R3
const uint8_t colPins[3] = { D1, D3, D4 }; // C1, C2, C3
#define MODE_BTN D8                       // D8

// ===== Button debounce =====
const unsigned long DEBOUNCE_MS = 500;
unsigned long lastPress[3][3] = {0};

// ===== Button enum =====
enum Btn {
  BTN_MENU, BTN_UP, BTN_POWER,
  BTN_LEFT, BTN_OK, BTN_RIGHT,
  BTN_PREV, BTN_DOWN, BTN_NEXT
};

// ===== IR send helper =====
void sendIR(uint8_t cmd) {
  Serial.printf("IR Send: 0x%02X\n", cmd);
  IrSender.sendNEC(NEC_ADDRESS, cmd, 3);
}

// ===== Handlers =====
void handleButtonPrimary(Btn b) {
  switch(b) {
    case BTN_MENU:  sendIR(0x5F); break;
    case BTN_UP:    sendIR(0x5B); break;
    case BTN_DOWN:  sendIR(0x19); break;
    case BTN_LEFT:  sendIR(0x1C); break;
    case BTN_RIGHT: sendIR(0x14); break;
    case BTN_OK:    sendIR(0x18); break;
    case BTN_PREV:  sendIR(0x1D); break;
    case BTN_NEXT:  sendIR(0x15); break;
    case BTN_POWER: sendIR(0x16); break;
  }
}

void handleButtonSecondary(Btn b) {
  switch(b) {
    case BTN_MENU:  sendIR(0x57); break; // SETUP
    case BTN_UP:    sendIR(0x06); break; // ZOOM
    case BTN_DOWN:  sendIR(0x0A); break; // ANGLE
    case BTN_LEFT:  sendIR(0x4C); break; // REPEAT AB
    case BTN_RIGHT: sendIR(0x50); break; // REPEAT
    case BTN_OK:    sendIR(0x0F); break; // PLAY/PAUSE
    case BTN_PREV:  sendIR(0x48); break; // PREVIEW
    case BTN_NEXT:  sendIR(0x1A); break; // DISPLAY
    case BTN_POWER: sendIR(0x0E); break; // AUDIO
  }
}

// ===== Matrix initialization =====
void initMatrix() {
  for (int r = 0; r < 3; r++) {
    pinMode(rowPins[r], OUTPUT);
    Serial.println(rowPins[r]);
    Serial.println("Matrix row initialized");
    delay(1000);
    digitalWrite(rowPins[r], HIGH); // rij “inactief”
  }
  for (int c = 0; c < 3; c++) {
    pinMode(colPins[c], INPUT_PULLUP); // kolommen met pull-ups
  }
  //pinMode(MODE_BTN, INPUT_PULLUP);     // mode knop
  //pinMode(IR_SEND_PIN, OUTPUT);        // IR LED
  //digitalWrite(IR_SEND_PIN, LOW);      // LED uit
  Serial.println("Matrix initialized");
}

// ===== Scan matrix =====
void scanMatrix() {
  bool shiftMode = digitalRead(MODE_BTN); // High = pressed

  for (int r = 0; r < 3; r++) {
    digitalWrite(rowPins[r], LOW);
    delayMicroseconds(5);

    for (int c = 0; c < 3; c++) {
      if (digitalRead(colPins[c]) == LOW) {
        unsigned long now = millis();
        if (now - lastPress[r][c] > DEBOUNCE_MS) {
          lastPress[r][c] = now;
          Btn b = (Btn)(r * 3 + c);
          if (shiftMode) handleButtonSecondary(b);
          else handleButtonPrimary(b);
        }
      }
    }
    digitalWrite(rowPins[r], HIGH);
  }
}

// ===== Web UI =====
String makePage() {
  String html =
    "<!DOCTYPE html><html lang='nl'>"
    "<head><meta charset='utf-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<title>DVD IR Remote</title>"
    "<style>"
    "body{font-family:system-ui; background:#f2f2f7; text-align:center; margin:0; padding:12px;}"
    "h2{margin:8px 0 16px;font-weight:600}"
    "button{width:92px;height:50px;font-size:15px;margin:6px;border-radius:10px;}"
    ".wide{width:160px}"
    ".power{background:#ff3b30;color:#fff;border:none}"
    ".power:active{background:#d70015}"
    ".dpad{display:grid;grid-template-columns:92px 92px 92px;grid-template-rows:50px 50px 50px;gap:10px;justify-content:center}"
    ".empty{visibility:hidden}"
    ".row{display:flex;justify-content:center;gap:10px;flex-wrap:wrap}"
    "</style></head><body>";

  html += "<h2>DVD IR Remote</h2>";

  // Power
  html += "<div class='row'><button class='power wide' onclick='send(0x16)'>POWER</button></div>";

  // D-pad
  html += "<div class='dpad'>"
          "<div class='empty'></div>"
          "<button onclick='send(0x5B)'>▲</button>"
          "<div class='empty'></div>"
          "<button onclick='send(0x1C)'>◀</button>"
          "<button onclick='send(0x18)'>OK</button>"
          "<button onclick='send(0x14)'>▶</button>"
          "<div class='empty'></div>"
          "<button onclick='send(0x19)'>▼</button>"
          "<div class='empty'></div>"
          "</div>";

  // Transport
  html += "<div class='row'>"
          "<button onclick='send(0x1D)'>⏮</button>"
          "<button onclick='send(0x0F)'>▶</button>"
          "<button onclick='send(0x13)'>■</button>"
          "<button onclick='send(0x15)'>⏭</button>"
          "</div>";

  // Menu
  html += "<div class='row'>"
          "<button onclick='send(0x5F)'>MENU</button>"
          "<button onclick='send(0x57)'>SETUP</button>"
          "<button onclick='send(0x1A)'>DISPLAY</button>"
          "</div>";

  // Extra
  html += "<div class='row'>"
          "<button onclick='send(0x06)'>ZOOM</button>"
          "<button onclick='send(0x0A)'>ANGLE</button>"
          "<button onclick='send(0x12)'>SUB</button>"
          "<button onclick='send(0x0E)'>AUDIO</button>"
          "</div>";

  // Repeat / Eject
  html += "<div class='row'>"
          "<button onclick='send(0x50)'>REPEAT</button>"
          "<button onclick='send(0x4C)'>A-B</button>"
          "<button onclick='send(0x1E)'>EJECT</button>"
          "</div>";

  // JS
  html += "<script>function send(cmd){fetch('/ir?c='+cmd.toString(16));}</script>";
  html += "</body></html>";
  return html;
}

// ===== Web handlers =====
void handleRoot() { server.send(200,"text/html",makePage()); }

void handleIR() {
  if (!server.hasArg("c")) { server.send(400,"text/plain","Missing command"); return; }
  uint8_t cmd = strtoul(server.arg("c").c_str(), nullptr, 16);
  sendIR(cmd);
  server.send(200,"text/plain","OK");
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);
  Serial.println(" ");
  Serial.println("Starting DVD IR Remote...");


  delay(5000);
  Serial.println("init matrix...");
  initMatrix();
  Serial.println("Starting matrix...");

  // Wi-Fi AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());

  // IR sender
  IrSender.begin(IR_SEND_PIN);

  // Web server
  server.on("/", handleRoot);
  server.on("/ir", handleIR);                                  
  server.begin();
  Serial.println("Web server started");
}

// ===== Loop =====
void loop() {
  scanMatrix();         // hardware buttons
  delay(100);
  server.handleClient(); // web UI
}
