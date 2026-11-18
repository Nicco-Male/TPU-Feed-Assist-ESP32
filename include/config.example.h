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
#define GPIO_FEED 4
#define GPIO_UNLOAD 5
#define GPIO_FORCE 6
#define GPIO_UNLOAD_ARM 7
// Optional sensing of the ORIGINAL physical buttons. OFF until a dedicated,
// electrically isolated, active-low sensor board has been measured/validated.
// NEVER connect Bambu switch terminals to ESP32 GPIOs directly.
#define ENABLE_LOCAL_BUTTON_MONITORING false
#define GPIO_LOCAL_FEED 8
#define GPIO_LOCAL_UNLOAD 9
#define GPIO_LOCAL_FORCE 10
// No autonomous unloading; physical key alone is NOT proof the toolhead is safe.
#define ENABLE_UNLOAD_BUTTON false
#define PULSE_MS 150U
#define COMMAND_COOLDOWN_MS 1200U
