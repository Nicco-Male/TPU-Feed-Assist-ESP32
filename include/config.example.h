#pragma once
// Copy to config.h. Never commit secrets.
#define DEVICE_ID "x1c-tpu-01"
#define WIFI_SSID "CHANGE_ME"
#define WIFI_PASSWORD "CHANGE_ME"
#define HTTP_USER "admin"
#define HTTP_PASSWORD "CHANGE_TO_A_LONG_RANDOM_PASSWORD"
#define HTTP_API_KEY "CHANGE_TO_A_DIFFERENT_LONG_RANDOM_TOKEN"
#define HTTP_PORT 80
#define MQTT_HOST "192.168.10.222"
#define MQTT_PORT 1883
#define MQTT_USER "tpu-feeder"
#define MQTT_PASSWORD "CHANGE_ME"
#define MQTT_TLS 0
#define MQTT_CA_CERT ""
#define NTP_SERVER "pool.ntp.org"
// Seeed XIAO ESP32-S3 STANDARD board (D0..D10).
// Outputs to isolated PhotoMOS DRIVER LEDs, never directly to Bambu switch lines.
#define GPIO_FEED 4       // XIAO D3 -> PhotoMOS FEED
#define GPIO_UNLOAD 5     // XIAO D4 -> PhotoMOS UNLOAD
#define GPIO_FORCE 6      // XIAO D5 -> PhotoMOS FORCE
#define GPIO_UNLOAD_ARM 7 // XIAO D8 -> physical active-low arming switch
// Optional sensing of the ORIGINAL physical buttons. OFF until a dedicated,
// electrically isolated, active-low sensor board has been measured/validated.
// NEVER connect Bambu switch terminals to ESP32 GPIOs directly.
#define ENABLE_LOCAL_BUTTON_MONITORING false
#define GPIO_LOCAL_FEED 1    // XIAO D0; isolated, active-low sense input
#define GPIO_LOCAL_UNLOAD 2  // XIAO D1; isolated, active-low sense input
#define GPIO_LOCAL_FORCE 8   // XIAO D9; isolated, active-low sense input
// XIAO D2/GPIO3: ESP32-S3 strapping input, intentionally unused.
// XIAO D6/GPIO43 and D7/GPIO44: UART0, intentionally unused.
// XIAO D10/GPIO9: reserved for future expansion.
// No autonomous unloading; physical key alone is NOT proof the toolhead is safe.
#define ENABLE_UNLOAD_BUTTON false
#define PULSE_MS 150U
#define COMMAND_COOLDOWN_MS 1200U
