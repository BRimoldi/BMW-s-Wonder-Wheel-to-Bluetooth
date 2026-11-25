# BMW Wonder Wheel to Bluetooth Keystroke Converter

**Enhanced Wonder Wheel edition — Wi-Fi OTA and improved long-push recognition**

Control the Scenic navigation app on an iPhone using the Wonder Wheel on a BMW motorcycle. This edition adds Wi-Fi firmware updates at startup and recognizes long left/right pushes as soon as their threshold is reached.

Developed by **Bixio Rimoldi**. Tested with an **Arduino Nano ESP32** on a **BMW M 1000 R**. Minor modifications may be needed for other BMW models, as the LIN messages corresponding to certain Wonder Wheel gestures can differ between models.

## Project overview

BMW's Multi-Function Controller, commonly called the **Wonder Wheel (WW)**, is used to operate the factory navigation interface. This project implements a LIN-bus slave that reads Wonder Wheel inputs and translates them into Bluetooth keyboard events for **Scenic** on an iPhone.

The same approach may be useful with other applications that accept Bluetooth keyboard arrow, Return/Enter, and Escape keys. Actual app behavior depends on its keyboard shortcuts.

A commercial product with similar functionality is [WunderLINQ X by Black Box Embedded](https://www.blackboxembedded.com/). This project was started before WunderLINQ X was available.

## Safety and liability

**This project is provided “as is”, without any guarantee of functionality, safety, or compatibility.** The author accepts no responsibility for accidents, injuries, damage, or legal violations arising from its use. Test all features safely while stationary, avoid distraction while riding, and comply with applicable laws. Installation involves the motorcycle's electrical system; verify wiring carefully.

## Hardware and project files

The original build uses:

- Arduino Nano ESP32
- NXP TJA1021 LIN transceiver
- One diode
- Three capacitors of approximately **220 pF**, **68 nF**, and **47–100 µF**
- BMW 3-pin connector/harness, part **83 30 0 413 585**

The circuit was implemented on a compact PCB approximately **7 × 3 cm**. Refer to the repository's **circuit schematic** for the actual wiring and component placement. The project also includes a **3D-printable enclosure design**.

On the tested BMW M 1000 R, the motorcycle's 3-pin navigation connector is located under the left-side tank cover and provides **+12 V, ground, and LIN**. This single connector is used both to connect the Arduino Nano ESP32 to the motorcycle's LIN bus and to supply power to the Nano and the Quad Lock Wireless Charging Head.

## Motorcycle prerequisites and NAV mode

The motorcycle must be **navigation-ready**. This is standard on some models equipped with a factory navigator cradle; other bikes may require software configuration by a BMW dealer.

The motorcycle checks its navigation LIN bus for a navigator. The firmware responds to the relevant LIN messages so the motorcycle makes navigation mode available.

To use the Wonder Wheel with Scenic:

1. Start the motorcycle and allow the Arduino to finish its startup OTA window (approximately one minute; see below).
2. On the left handlebar, **press and hold the MENU rocker upward** to select **NAV** mode. The TFT display indication changes from **TFT** to **NAV**.
3. In NAV mode, Wonder Wheel movements are forwarded via the navigation LIN bus and converted to Bluetooth keyboard inputs.
4. **Press the MENU rocker downward** to return to TFT control.

## Wonder Wheel controls

| Wonder Wheel action | Bluetooth keyboard event | Typical Scenic action |
|---|---|---|
| Rotate up/down | Up/Down arrow | Zoom in/out on the navigation screen; move the selection up/down within menus |
| Short push right/left | Right/Left arrow | Move the selection right/left within menus; trigger configured quick actions on the navigation screen |
| Long push right | Return/Enter | Select the currently highlighted item |
| Long pull left | Escape | Can be assigned to an action, such as writing the current GPS coordinates to a file |

**Improved long pushes:** A long left or right action is emitted when the firmware recognizes approximately **10 successive LIN frames** indicating the push (roughly **0.5 seconds** at the observed ~50 ms frame interval). The rider does not need to release the wheel first. The long action is sent once per hold rather than repeatedly. Short left/right actions are sent on release.

**Navigating and selecting items in Scenic:** Menu items are arranged in a two-dimensional grid. All four arrow directions move the highlight between items. The currently highlighted item is identified by a visually distinct border (green in the author's configuration); a long right push selects it. On the normal navigation screen, this opens the menu when its icon is highlighted.

The actual action within Scenic depends on Scenic's keyboard and shortcut configuration.

## Installing the software

1. Install the [Arduino IDE](https://www.arduino.cc/en/software).
2. In Boards Manager, install **Arduino ESP32 Boards** and select **Arduino Nano ESP32**.
3. Install the LIN slave and BLE keyboard libraries used by the sketch. The original repository includes or references `LIN_slave_portable_Arduino` and `ESP32-BLE-Keyboard`; the sketch uses `<LIN_slave_HardwareSerial_ESP32.h>` and `<BleKeyboard.h>`.
4. Download the **Enhanced Wonder Wheel** `.ino` sketch, place it in an Arduino sketch folder with the same basename, and open it in the IDE.
5. **Before uploading, replace both placeholder passwords** as explained under **Password configuration** below. Check the `WIFI_SSID`, `OTA_USERNAME`, and other configuration values. Do not publish personal credentials.
6. Compile and upload to the Arduino Nano ESP32 over USB for the first installation.
7. Pair the iPhone with the Bluetooth keyboard advertised as **`BMW's WW`**, then test Scenic while the motorcycle is stationary and in NAV mode.

The sketch uses the Arduino ESP32 core's `WiFi`, `WebServer`, and `Update` facilities for OTA. Library and board package versions should be recorded when preparing a reproducible release.

## Password configuration

Before compiling and uploading the sketch, open `EnhancedWonderWheel/EnhancedWonderWheel.ino` and locate these settings near the beginning:

```cpp
const char* WIFI_SSID = "Motorcycle-OTA";
const char* WIFI_PASSWORD = "ChangeThisPassword";
const char* OTA_USERNAME = "admin";
const char* OTA_PASSWORD = "ChooseAnotherStrongPassword";
```

**Replace both example passwords with your own strong, different passwords.** `WIFI_PASSWORD` protects the Nano's Wi-Fi access point and must be at least 8 characters long; `OTA_PASSWORD` protects the firmware-upload web interface. You may also change the OTA username. Keep your personalized copy private, and **do not commit real passwords to GitHub**. The published sketch intentionally contains placeholders, not the author's credentials.

## Wi-Fi OTA firmware updates

The firmware automatically enables Wi-Fi OTA mode for **60 seconds** each time the Nano starts. During this period, a new firmware version can be uploaded wirelessly. Once the OTA window closes, Wi-Fi is disabled and normal Wonder Wheel operation begins.

1. Compile the intended sketch in the Arduino IDE and **export the compiled `.bin` file**.
2. Restart or power-cycle the Nano. During the first **60 seconds**, connect your computer or phone to the Wi-Fi network **`Motorcycle-OTA`** using the configured Wi-Fi password.
3. Open **http://192.168.4.1** in a browser. Authenticate using the configured OTA username and password.
4. Select the compiled `.bin` firmware file and submit the update. Keep the Nano powered throughout the process.
5. On a successful upload, the Nano restarts. The new firmware begins another startup cycle.

If no update is in progress, the OTA window expires after approximately **60 seconds**. Wi-Fi is switched off and the LIN interface enters **Riding mode**. An upload started during the 60-second window is allowed to finish, even if the window expires before the upload is complete. The firmware also contains protection for a stalled upload.

**Important:** To restart the Nano and access OTA mode, the motorcycle ignition must remain off long enough for power to the Nano to be interrupted. On the tested BMW M 1000 R, power continues to be supplied for approximately 30–60 seconds after the ignition is switched off.

## Troubleshooting

**Wonder Wheel does not control Scenic:** Verify that the Nano has completed its OTA window and entered LIN Riding mode, the motorcycle is navigation-ready and in **NAV** rather than TFT mode, and the iPhone is connected to `BMW's WW` over Bluetooth.

**The OTA Wi-Fi network does not appear:** It is available only during the startup update window. Make sure the Nano has actually restarted; switching the motorcycle ignition off briefly may not remove power.

**iPhone will not reconnect after switching firmware editions:** When changing between the Wonder Wheel-only firmware and the version with Insta360 support, the iPhone may retain incompatible Bluetooth pairing information. In **Settings → Bluetooth**, select the Nano's keyboard device, choose **Forget This Device**, and pair it again. This resolved the issue in the author's tests; it may not be necessary after every update.

**OTA upload fails:** Verify that you selected the correct compiled `.bin` file, that the Nano is reliably powered, and that your device is connected to the Nano's Wi-Fi access point, **`Motorcycle-OTA`**.

## Extended edition: Wonder Wheel + Insta360 GO 3S

A separate, more advanced edition retains the Wonder Wheel-to-Scenic features and also uses the **separate turn-signal cancellation button** to control an **Insta360 GO 3S**. A single short press takes a photograph or stops recording; a double short press starts video. The firmware can reconnect to and wake the camera when needed.

## Credits and acknowledgments

- **Author and hardware testing:** Bixio Rimoldi.
- **LIN slave implementation:** Based on [LIN Slave Portable Arduino](https://github.com/gicking/LIN_slave_portable_Arduino) by gicking.
- **Bluetooth keyboard implementation:** Based on [ESP32-BLE-Keyboard](https://github.com/T-vK/ESP32-BLE-Keyboard).
- **Development support:** ChatGPT (OpenAI) assisted with firmware development, debugging, and documentation. Hardware integration and motorcycle testing were performed by the author.

When redistributing incorporated third-party source code, retain its applicable license and copyright notices. Review the actual license files before publishing a packaged source release.

## Demo and feedback

- [Original demonstration video](https://www.youtube.com/watch?v=-cVcGIMZuJ0)
- Feedback: **WWtoBluetooth@icloud.com**

---

*Tested configuration: BMW M 1000 R, Arduino Nano ESP32, iPhone with Scenic.*
