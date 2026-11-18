#pragma once

// Public, non-secret firmware configuration for the standard Seeed XIAO ESP32-S3.
// Wi-Fi SSID/password, Bambuddy URL/API key, MQTT and administrator credentials
// are provisioned on first boot through the local captive portal (Preferences/NVS).
#define DEVICE_ID "x1c-tpu-01"
#define HTTP_PORT 80
#define NTP_SERVER "pool.ntp.org"

// D pins below refer to XIAO silkscreen. Numbers here are ESP32 native GPIOs.
#define GPIO_FEED 4          // D3 -> isolated PhotoMOS driver
#define GPIO_UNLOAD 5        // D4 -> isolated PhotoMOS driver
#define GPIO_FORCE 6         // D5 -> isolated PhotoMOS driver
#define GPIO_UNLOAD_ARM 7    // D8 -> local physical arming switch, active LOW

// Optional, unverified isolated sensing circuit. OFF until bench-tested.
#define ENABLE_LOCAL_BUTTON_MONITORING false
#define GPIO_LOCAL_FEED 1    // D0
#define GPIO_LOCAL_UNLOAD 2  // D1
#define GPIO_LOCAL_FORCE 8   // D9

// No automatic unload: still requires printer/toolhead filament safety work.
#define ENABLE_UNLOAD_BUTTON false
#define PULSE_MS 150U
#define COMMAND_COOLDOWN_MS 1200U

// OTA port; the secret password + enable flag are provisioned at runtime.
#define OTA_PORT 3232

// The current prototype supports optional plaintext MQTT on a trusted LAN.
// MQTT with CA-validated TLS requires a separate certificate provisioning step.
#define MQTT_TLS 0
#define MQTT_CA_CERT ""
