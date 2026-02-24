# Ir_remote_adv436
Arduino software for a ir remote 


# ESP8266 IR Remote (NodeMCU)

Standalone Wi-Fi IR remote based on **ESP8266 / NodeMCU**.

## Features
- 📡 Wi-Fi **Access Point**
- 🌐 **DNS captive portal** (auto-opens browser)
- 📱 iOS-style web UI
- 🔁 NEC IR protocol
- 🖥 Serial debug @ 115200 baud

## Hardware
- NodeMCU / ESP8266
- IR LED + resistor
- NPN transistor (base via resistor)


### IR wiring (NPN)

# ESP8266 NEC IR Remote

Simple NEC infrared sender using an ESP8266 (NodeMCU) with a web interface.

## Features

- WiFi Access Point (`NEC_REMOTE`)
- DNS name: http://necremote
- Captive portal with redirect screen
- Set NEC address (default `01`)
- Set NEC command (00–FF, hex, upper/lowercase)
- Repeats every second until stopped
- Send activity indicator

## Hardware

- ESP8266 (NodeMCU)
- IR LED + 100–220Ω resistor
- (Optional) transistor for higher IR power

**IR Pin:** `D2`

## Libraries

Install via Arduino Library Manager:

- IRremote (v4.x)
- ESP8266WiFi
- ESP8266WebServer
- DNSServer

## Usage

1. Flash the sketch.
2. Connect to WiFi:
