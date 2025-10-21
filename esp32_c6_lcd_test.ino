/*
  ESP32‑C6 LCD Test Sketch

  This Arduino sketch exercises the key peripherals on the Waveshare ESP32‑C6‑LCD‑1.47 development board.

  It demonstrates the following features:
    • Initializes the 1.47 inch ST7789 TFT display (172 × 320 pixels) and
      draws some text and graphics on the screen.
    • Cycles the on‑board WS2812 RGB LED through a rainbow of colors using the
      Adafruit_NeoPixel library.
    • Scans for nearby Wi‑Fi networks and prints their SSIDs to the serial
      terminal.  The total network count is also displayed on the LCD.
    • Optionally mounts an SD card if present and reports the free space.

  To compile this sketch you will need to install the following Arduino
  libraries via the Library Manager:

    • Adafruit ST7735 and ST7789 Library (Adafruit_ST7789)
    • Adafruit GFX Library (Adafruit_GFX)
    • Adafruit NeoPixel (Adafruit_NeoPixel)

  Pin assignments are based on the official Waveshare documentation:

    * TFT display (SPI):
        MOSI  – GPIO 6
        SCLK  – GPIO 7
        CS    – GPIO 14
        DC    – GPIO 15
        RST   – GPIO 21
        BL    – GPIO 22 (backlight control – set HIGH for full brightness)
    * RGB LED (WS2812) – GPIO 8
    * SD‑Card (SPI):
        MOSI  – GPIO 6 (shared with TFT)
        MISO  – GPIO 5
        SCLK  – GPIO 7 (shared with TFT)
        CS    – GPIO 4

  Note: The ESP32‑C6 has multiple SPI buses.  This sketch uses a dedicated
  SPIClass instance to assign the correct pins to the TFT.  Make sure your
  board package is up to date (ESP32 core ≥ 2.0.13) to support the C6.

  Author: OpenAI ChatGPT
  Date: 21 Oct 2025
*/

#include <WiFi.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_NeoPixel.h>
#include <SD.h>

// -----------------------------------------------------------------------------
// Pin definitions
// -----------------------------------------------------------------------------
// TFT display pins
static constexpr int TFT_MOSI  = 6;
static constexpr int TFT_MISO  = 5;    // MISO is only used by the SD card
static constexpr int TFT_SCLK  = 7;
static constexpr int TFT_CS    = 14;
static constexpr int TFT_DC    = 15;
static constexpr int TFT_RST   = 21;
static constexpr int TFT_BL    = 22;   // backlight control (active HIGH)

// SD card chip select
static constexpr int SD_CS     = 4;

// RGB LED pin
static constexpr int RGB_PIN   = 8;
static constexpr int RGB_COUNT = 1;

// -----------------------------------------------------------------------------
// Global objects
// -----------------------------------------------------------------------------

// Use FSPI port (or HSPI/VSPI depending on your board package).  The pins
// will be configured manually in setup().
SPIClass tftSPI(FSPI);

// ST7789 display driver.  We pass the SPI class and the control pins.
Adafruit_ST7789 tft(&tftSPI, TFT_CS, TFT_DC, TFT_RST);

// WS2812 RGB LED
Adafruit_NeoPixel rgbLed(RGB_COUNT, RGB_PIN, NEO_GRB + NEO_KHZ800);

// Track hue for the LED rainbow cycle
uint8_t ledHue = 0;

// -----------------------------------------------------------------------------
// Helper functions
// -----------------------------------------------------------------------------

// Convert a hue (0–255) to an RGB color for the NeoPixel
uint32_t hueToColor(uint8_t hue) {
  // Simple HSV to RGB conversion.  See Adafruit NeoPixel library for reference.
  uint8_t r, g, b;
  if (hue < 85) {
    r = hue * 3;
    g = 255 - hue * 3;
    b = 0;
  } else if (hue < 170) {
    hue -= 85;
    r = 255 - hue * 3;
    g = 0;
    b = hue * 3;
  } else {
    hue -= 170;
    r = 0;
    g = hue * 3;
    b = 255 - hue * 3;
  }
  return rgbLed.Color(r, g, b);
}

// Draw a simple test screen on the TFT
void drawTestScreen(int wifiCount, bool sdMounted) {
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(F("ESP32‑C6 Test"));
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_CYAN);
  tft.println(F(""));
  tft.println(F("CPU: 160MHz RISC‑V"));
  tft.print(F("Flash: ")); tft.print(ESP.getFlashChipSize() / (1024 * 1024)); tft.println(F(" MB"));
  tft.println(F(""));
  tft.print(F("Wi‑Fi networks: ")); tft.println(wifiCount);
  tft.println(F(""));
  tft.print(F("SD Card: "));
  tft.println(sdMounted ? F("present") : F("not found"));
  tft.println(F(""));
  tft.setTextColor(ST77XX_GREEN);
  tft.println(F("RGB LED cycling…"));
}

// -----------------------------------------------------------------------------
// Arduino setup function
// -----------------------------------------------------------------------------

void setup() {
  // Initialize serial port for debug output
  Serial.begin(115200);
  delay(1000);
  Serial.println(F("\nESP32‑C6 LCD Test Sketch"));

  // Configure backlight and RGB LED pins
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);  // full brightness

  // Initialize the NeoPixel
  rgbLed.begin();
  rgbLed.show();  // turn off LED

  // Initialize custom SPI bus for TFT and SD
  tftSPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);

  // Initialize TFT
  Serial.println(F("Initializing display…"));
  tft.init(172, 320);            // 172×320 resolution
  tft.setRotation(1);            // rotate for portrait orientation (0–3)
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextWrap(false);

  // Mount SD card (if present)
  bool sdOK = false;
  Serial.print(F("Mounting SD card…"));
  if (SD.begin(SD_CS)) {
    Serial.println(F("success"));
    uint64_t cardSize = SD.cardSize() / (1024ULL * 1024ULL);
    uint64_t totalBytes = SD.totalBytes() / (1024ULL * 1024ULL);
    uint64_t usedBytes = SD.usedBytes() / (1024ULL * 1024ULL);
    Serial.printf("  Size: %llu MB, Used: %llu MB\n", cardSize, usedBytes);
    sdOK = true;
  } else {
    Serial.println(F("failed"));
  }

  // Initialize Wi‑Fi (scan only)
  Serial.println(F("Scanning for Wi‑Fi networks…"));
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int networkCount = WiFi.scanNetworks();
  Serial.printf("Found %d networks\n", networkCount);
  for (int i = 0; i < networkCount; i++) {
    Serial.printf("  %d: %s (%d dBm)\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
  }

  // Draw information on the LCD
  drawTestScreen(networkCount, sdOK);
}

// -----------------------------------------------------------------------------
// Arduino loop function
// -----------------------------------------------------------------------------

void loop() {
  // Cycle the RGB LED through a rainbow of colors
  rgbLed.setPixelColor(0, hueToColor(ledHue));
  rgbLed.show();
  ledHue++;
  delay(20);
}
