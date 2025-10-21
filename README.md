# ESP32‑C6 LCD Test Firmware

This repository contains an example Arduino sketch and the accompanying
resources required to provide a one‑click web flash experience for the
Waveshare **ESP32‑C6‑LCD‑1.47** development board.

## Contents

* `esp32_c6_lcd_test.ino` – Arduino sketch that tests the board’s LCD, built‑in
  WS2812 RGB LED and performs a Wi‑Fi network scan.  It also mounts a TF
  card if present.
* `manifest.json` – ESP Web Tools manifest describing the firmware.  It points
  to a single merged binary file for the ESP32‑C6.
* `index.html` – A simple web page that embeds the ESP Web Tools installer
  button.  When served over HTTPS (e.g. via GitHub Pages) it allows a user
  to flash the firmware to the board directly from their browser.

## How to build the firmware

1. Install the **ESP32** board support package for Arduino (version 2.0.13 or
   later) using the Arduino IDE or `arduino-cli`.  Ensure that the
   `esp32c6` board definitions are available.
2. Use the Arduino IDE (or `arduino-cli compile`) to compile the
   `esp32_c6_lcd_test.ino` sketch for the `ESP32C6 Dev Module` target.
3. The compilation process will produce several binary images.  To prepare a
   single file for web flashing you need to merge these images using
   `esptool`.  Run a command similar to:

   ```bash
   esptool.py --chip esp32c6 merge_bin \
       -o firmware/esp32_c6_lcd_test.bin \
       --flash_mode dio --flash_freq 80m --flash_size 4MB \
       0x0 bootloader.bin \
       0x8000 partitions.bin \
       0xe000 boot_app0.bin \
       0x10000 esp32_c6_lcd_test.ino.bin
   ```

   Adjust the flash mode/frequency if required.  The file
   `esp32_c6_lcd_test.ino.bin` is the application binary produced by the
   compiler.  The location of each file will be shown in the Arduino IDE
   build output.
4. Place the resulting merged binary in a `firmware` folder within this
   repository so that it matches the path referenced in `manifest.json`.

## Hosting on GitHub Pages

1. Commit and push the files in this folder (`esp32_c6_lcd_test.ino`,
   `manifest.json`, `index.html` and your `firmware/esp32_c6_lcd_test.bin`) to
   a GitHub repository.  Ensure the repository has the following structure:

   ```
   ├── firmware/
   │   └── esp32_c6_lcd_test.bin
   ├── index.html
   ├── manifest.json
   └── esp32_c6_lcd_test.ino
   ```

2. Enable **GitHub Pages** in the repository settings and set the source to
   the branch/folder where these files reside (for example the root of `main`).
3. After GitHub Pages is published you can visit `https://<username>.github.io/<repo>/`
   (replace `<username>` and `<repo>` with your GitHub username and repository
   name) to access the installer page.
4. Connect your ESP32‑C6‑LCD‑1.47 board via USB, open the page in Chrome or
   Edge on a desktop computer and click the **Install** button.  Follow the
   prompts to flash your firmware.  When complete the LCD will display
   information about the board and the RGB LED will animate.

## Notes

* Web serial is only available over HTTPS and is not supported on iOS.
* To customise the on‑screen content edit `drawTestScreen()` in
  `esp32_c6_lcd_test.ino`.
* The example firmware does not enable Improv Wi‑Fi support.  If you would
  like to configure Wi‑Fi credentials via the installer UI you can
  integrate the Improv serial library into your sketch and add an
  `improv` section to `manifest.json`.  See
  <https://improv-wifi.github.io/> for details.
