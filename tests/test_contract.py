import pathlib
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "src/main.cpp").read_text()
CONFIG = (ROOT / "include/config.example.h").read_text()
DOC = (ROOT / "docs/INLINE_BUTTON_BOARD.md").read_text()

class PrototypeContract(unittest.TestCase):
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
