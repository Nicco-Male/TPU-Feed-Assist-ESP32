import pathlib
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "src/main.cpp").read_text()
CONFIG = (ROOT / "include/config.example.h").read_text()
DOC = (ROOT / "docs/INLINE_BUTTON_BOARD.md").read_text()

PLATFORMIO = (ROOT / "platformio.ini").read_text()
WORKFLOW = (ROOT / ".github/workflows/firmware.yml").read_text()

class PrototypeContract(unittest.TestCase):
    def test_xiao_target(self):
        self.assertIn("board = seeed_xiao_esp32s3", PLATFORMIO)
        self.assertIn("[env:xiao_esp32s3]", PLATFORMIO)
        self.assertIn("pio run -e xiao_esp32s3", WORKFLOW)
        pins = {
            "GPIO_FEED": 4, "GPIO_UNLOAD": 5, "GPIO_FORCE": 6,
            "GPIO_UNLOAD_ARM": 7,
            "GPIO_LOCAL_FEED": 1, "GPIO_LOCAL_UNLOAD": 2,
            "GPIO_LOCAL_FORCE": 8,
        }
        for name, pin in pins.items():
            self.assertRegex(CONFIG, rf"#define\s+{name}\s+{pin}(?:\s|$)")
    def test_wifi_ota_is_opt_in_and_password_protected(self):
        self.assertIn("#define OTA_PORT 3232", CONFIG)
        self.assertIn("ArduinoOTA.setPassword(otaPassword.c_str())", SOURCE)
        self.assertIn("otaRequested && otaPassword.length() >= 16", SOURCE)
        self.assertIn("otaInProgress", SOURCE)
        self.assertIn('if (otaInProgress) return "ota_in_progress"', SOURCE)

    def test_public_firmware_has_no_network_secrets(self):
        self.assertNotIn("#define WIFI_SSID", CONFIG)
        self.assertNotIn("#define WIFI_PASSWORD", CONFIG)
        self.assertNotIn("#define MQTT_PASSWORD", CONFIG)
        self.assertIn("setupWifiPortal()", SOURCE)
        self.assertIn('wm.autoConnect(name,"TPUFeedSetup!")', SOURCE)
        self.assertIn('preferences.begin("tpufeeder",false)', SOURCE)
        self.assertIn("bambuddy_integration_active", SOURCE)

    def test_protected_unload(self):
        self.assertIn("#define ENABLE_UNLOAD_BUTTON false", CONFIG)
        self.assertIn('buttonName == "unload" && !isUnloadArmed()', SOURCE)

    def test_no_fake_sensing_state(self):
        self.assertIn('doc["local_button_sensing_available"] = false', SOURCE)
        self.assertIn("NOT a verified schematic", DOC)

    def test_remote_unique_commands(self):
        self.assertIn("hasSeenId(id)", SOURCE)
        self.assertIn("MQTT_MAX_CLOCK_SKEW_S", SOURCE)

    def test_isolated_gpio_outputs(self):
        for output in ["GPIO_FEED", "GPIO_UNLOAD", "GPIO_FORCE"]:
            self.assertIn(output, SOURCE)
        self.assertIn("esp_timer_start_once", SOURCE)

    def test_no_claimed_automatic_switching(self):
        self.assertIn('doc["multimaterial_switching_supported"] = false', SOURCE)

if __name__ == "__main__":
    unittest.main()
