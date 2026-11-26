# Wonder Wheel + Insta360 Edition — Version 1.0

**BMW Wonder Wheel to Bluetooth keyboard and Insta360 GO 3S camera control**

Developed and hardware-tested by **Bixio Rimoldi**. Release: **November 2026**.

The Arduino Nano ESP32 reads the motorcycle's LIN bus to interpret inputs from both the Wonder Wheel and the turn-signal cancellation button. It uses these inputs to perform two functions:

1. It translates movements of the **Wonder Wheel** into Bluetooth keyboard commands to control **Scenic** on an iPhone.
2. It translates gestures of the **turn-signal cancellation button** into Bluetooth Low Energy (BLE) commands to control an **Insta360 GO 3S** camera.

The firmware extends the Wonder Wheel interface with camera control, camera-state tracking, BLE reconnection and gesture-triggered over-the-air firmware updates.

> **Safety:** This experimental project is provided “as is”, without any warranty of safety, compatibility or reliability. Test while stationary. Do not operate the controls in a distracting manner while riding. Verify wiring and comply with relevant laws.

## Which edition should I install?

| Edition | What it does | OTA activation |
| --- | --- | --- |
| **Wonder Wheel Edition — Version 2.0** | Scenic navigation control via Wonder Wheel | Automatically at startup |
| **Wonder Wheel + Insta360 Edition — Version 1.0** | Scenic control **plus** GO 3S camera control | Hold the turn-signal cancellation button for about 10 seconds |

These are **alternative firmware sketches** for the same Arduino, not two programs to upload together. The editions have independent version numbers.

## Tested hardware

- Arduino Nano ESP32
- NXP TJA1021 LIN transceiver and compatible interface circuitry
- BMW M 1000 R with navigation-ready Wonder Wheel/LIN connection
- iPhone running the Scenic navigation app
- Insta360 GO 3S camera

For wiring, the BMW navigation connector, circuit schematic, and enclosure details, consult the [repository's main README](../README.md) and its hardware files. The sketch has not established compatibility with other BMW motorcycles or camera models.

## Wonder Wheel control of Scenic

The motorcycle must be navigation-ready and in **NAV** mode. On the tested BMW, hold the MENU rocker **upward** to select NAV mode; press it **downward** to return to TFT control.

| Wonder Wheel movement | BLE keyboard key | Typical Scenic use |
| --- | --- | --- |
| Rotate up/down | Up/Down arrow | Zoom, or navigate menus |
| Short push right/left | Right/Left arrow | Move selection or trigger configured actions |
| Long push right | Return/Enter | Select highlighted item |
| Long pull left | Escape | App-dependent shortcut |

A lateral push becomes “long” after about **10 consecutive LIN frames** (approximately **0.5 seconds** at the observed frame rate). Long events are sent once per hold and do not wait for release; short events are sent on release. The actual effect depends on Scenic's keyboard configuration.

The BLE keyboard is advertised as **`BMW's WW`**.

## Insta360 GO 3S controls

Camera gestures use the **turn-signal cancellation button**, **not** the Wonder Wheel. The firmware distinguishes intentional gestures from an ordinary cancellation after activating a turn signal.

| Gesture | Effect |
| --- | --- |
| **One short press** (under 1 second) | Take a photo if not recording; **stop** video if recording |
| **Double short press** (second press within approximately 250 ms of the first release) | **Start** video if not recording; **stop** video if already recording |
| **Long press**, released after 1 second but before 10 seconds | Request camera wake-up / reconnection |
| **Hold for approximately 10 seconds** | Enter Wi-Fi firmware-update (OTA) mode |

**Important:** The double press does **not** restart recording if the camera is already recording: the firmware stops recording in that case. Camera commands may be delayed because the firmware first requests the camera's actual recording status. If the camera cannot be reached or the status cannot be confirmed, the gesture may be discarded rather than executing an uncertain action.

### Camera connection and reconnection

When the Arduino starts, it automatically searches for the Insta360 GO 3S and attempts to establish a Bluetooth connection. If the camera is not found or a connection cannot be established within approximately **10 seconds**, the Arduino stops trying and proceeds with normal Wonder Wheel operation.

If the camera subsequently becomes disconnected, the Arduino does not continuously attempt to reconnect. Instead, the next camera-control gesture triggers a reconnection attempt when needed.

An important distinction is that **a Bluetooth connection does not necessarily mean that the camera is awake and ready to respond**. The firmware therefore checks the camera's status and, when necessary, sends a wake-up command before executing a photo or video instruction.

If communication with the camera cannot be established reliably, the requested photo or video action may be discarded. This prevents the Arduino from issuing commands without knowing the camera's current recording state.

## Required software and libraries

The following versions were used to compile the author's working firmware:

| Software or library | Tested version | Notes |
| --- | --- | --- |
| Arduino ESP32 Boards by Arduino | `2.0.18-arduino.5` | Arduino Nano ESP32 board package |
| [NimBLE-Arduino by h2zero](https://github.com/h2zero/NimBLE-Arduino) | `2.5.2` | Provides `NimBLEDevice.h` |
| [ESP32 NIMBLE Keyboard](https://github.com/Berg0162/ESP32-NIMBLE-Keyboard) | `2.0.1` | Author T-vK; maintainer J. van den Berg; provides `NimBleKeyboard.h` |
| [LIN_slave_portable_Arduino](https://github.com/gicking/LIN_slave_portable_Arduino) | Project repository copy | Installed as legacy library; no numeric version supplied |
| WiFi, WebServer, Update, FS | `2.0.0` each | Bundled with the ESP32 board package, not separate installs |

## Installing with Arduino IDE

1. Install the [Arduino IDE](https://www.arduino.cc/en/software) and the **Arduino ESP32 Boards** package through Boards Manager. Select **Arduino Nano ESP32**.
2. Install the [LIN slave implementation](https://github.com/gicking/LIN_slave_portable_Arduino) supplied in this project, which provides **`LIN_slave_HardwareSerial_ESP32.h`**.
3. Install **[NimBLE-Arduino by h2zero](https://github.com/h2zero/NimBLE-Arduino)** version **2.5.2** and **[ESP32 NIMBLE Keyboard by J. van den Berg](https://github.com/Berg0162/ESP32-NIMBLE-Keyboard)** version **2.0.1**. The latter provides **`NimBleKeyboard.h`** and uses NimBLE-Arduino 2.x. This edition does not use the older `BleKeyboard.h` library.
4. Keep the file **`BMW_To_PhoneAndInsta360_Control.ino`** in a folder of the same name, then open it in Arduino IDE.
5. Review the configuration settings described below, especially **camera name** and **OTA passwords**.
6. Compile, then upload the sketch over USB for the initial installation.
7. Pair the iPhone to the advertised Bluetooth keyboard **`BMW's WW`**. Test Scenic and the camera gestures **while stationary**.

**Switching editions:** Your iPhone may need to forget the previously paired `BMW's WW` device and pair again after switching between firmware editions because their Bluetooth implementations differ.

## Configuration before compiling

The sketch contains settings similar to:

```cpp
constexpr bool CAMERA_BENCH_SIMULATOR = false;
constexpr bool VERBOSE_INSTA360_RESPONSES = false;

const char* WIFI_SSID = "Motorcycle-OTA";
const char* WIFI_PASSWORD = "ChangeThisPassword";
const char* OTA_USERNAME = "admin";
const char* OTA_PASSWORD = "ChooseAnotherStrongPassword";

// Example: replace with your own camera's advertised BLE name.
const char* CAMERA_NAME = "GO 3S 59JUXP";
```

- **`CAMERA_NAME`**: Must match the BLE advertised name of the camera you want to control. The example above is from the uploaded sketch; it is **not** a universal GO 3S name.
- **`CAMERA_BENCH_SIMULATOR`**: Leave `false` on the motorcycle. When `true`, the Serial Monitor can simulate some controls; the OTA gesture is detected but does not start Wi-Fi OTA mode.
- **`VERBOSE_INSTA360_RESPONSES`**: Set `true` for additional hexadecimal BLE diagnostic output.
- **OTA passwords**: Replace *both* example passwords with strong, distinct passwords before uploading. The Wi-Fi password must be at least eight characters. Never commit your actual credentials to a public repository.

## Wi-Fi OTA updates (camera edition)

**This edition does not enable OTA automatically during startup.** Enter OTA mode using the turn-signal cancellation button:

1. Compile the desired sketch in Arduino IDE and **export the compiled `.bin` firmware image**.
2. With the Arduino running in normal mode, **hold the turn-signal cancellation button for approximately 10 seconds**. The firmware switches from normal LIN Riding mode to OTA maintenance mode, stopping the LIN slave. **The rider can confirm activation because the NAV indication disappears from the motorcycle's TFT display.** This is the visible indication that the Arduino has stopped communicating with the motorcycle over the LIN bus.
3. Once OTA mode is activated, a **two-minute (120-second) update window** begins. During this period, connect your computer or phone to the **`Motorcycle-OTA`** Wi-Fi network using the configured password.
4. **Within the same two-minute window**, open **http://192.168.4.1** in a browser, authenticate with the configured OTA username and password, select the compiled `.bin` firmware file, and **start the upload**. The upload must begin before the two-minute window expires, but it does not need to finish within that time. Once started, the firmware allows the upload to complete, provided communication continues normally.
5. Keep the Arduino powered during the update. Following a successful upload, it restarts.

A firmware upload already in progress can complete after the two-minute Wi-Fi window. A stalled upload is subject to an approximately 30-second inactivity timeout. If no upload is active when the two-minute Wi-Fi window expires, Wi-Fi is disabled and the firmware returns to LIN Riding mode.

During maintenance mode, normal riding-input processing is suspended; the BLE stack remains active.

## Diagnostics and troubleshooting

### Using the Arduino Serial Monitor

The firmware provides extensive diagnostic information through the Arduino IDE's **Serial Monitor**. This is useful for troubleshooting camera connections, Bluetooth communication, camera commands and OTA firmware updates.

1. Connect the Arduino Nano ESP32 to your computer with a USB cable.
2. Open the sketch in Arduino IDE and select the correct board and serial port.
3. Open **Tools → Serial Monitor** and set the baud rate to **115200**.

Messages include camera discovery, BLE connection and reconnection, camera wake-up, requests to take photographs or start/stop video, camera recording-state responses, and OTA activity or failures. **A command transmission message does not necessarily confirm that the camera executed the command.**

For raw hexadecimal camera BLE responses, set `VERBOSE_INSTA360_RESPONSES` to `true`. The setting `CAMERA_BENCH_SIMULATOR` enables simulation of **turn-signal cancellation gestures via the Serial Monitor** without operating the physical motorcycle button. Leave bench simulator mode disabled (`false`) for normal motorcycle use; simulated OTA gestures do not actually start Wi-Fi OTA.

**Scenic does not respond:** Confirm NAV mode, BLE pairing with `BMW's WW`, correct LIN wiring, and that the Arduino is not in OTA maintenance mode.

**Camera does not respond:** Confirm the camera model and advertised BLE name, battery/power state, and BLE proximity. Try a deliberate wake-up press (1 to under 10 seconds), then allow time for reconnection. The firmware may discard a capture command if it cannot confirm camera status.

**Double press starts no video:** Try the two presses closer together: the second must occur inside the approximately **250 ms** recognition interval. If the camera is already recording, the same gesture **stops** it.

**OTA network does not appear:** To activate OTA mode, hold the turn-signal cancellation button for approximately **10 seconds**. The NAV indication should disappear from the motorcycle's TFT display, confirming that the Arduino has entered OTA maintenance mode. The `Motorcycle-OTA` Wi-Fi network should then become available for **two minutes**. If it does not appear, verify that bench simulator mode is disabled and, if possible, check the Arduino Serial Monitor for diagnostic messages.

**iPhone does not reconnect after changing editions:** In iPhone Bluetooth settings, forget the existing keyboard entry and pair again.

**Compilation fails with a missing header:** Verify that all required libraries are installed and that their versions match those listed in the **Required software and libraries** section above. In particular, this edition requires ESP32 NIMBLE Keyboard version 2.0.1 and NimBLE-Arduino version 2.5.2, together with Arduino ESP32 Boards version 2.0.18-arduino.5.

## Project provenance and acknowledgments

- **Development, hardware integration, bench and motorcycle testing:** Bixio Rimoldi.
- **LIN slave implementation:** [gicking/LIN_slave_portable_Arduino](https://github.com/gicking/LIN_slave_portable_Arduino).
- **Insta360 BLE protocol research reference:** [xaionaro-go/insta360ctl](https://github.com/xaionaro-go/insta360ctl).
- **Development, debugging, BLE protocol analysis, documentation assistance:** ChatGPT (OpenAI).

Review and preserve any third-party licensing terms where code is incorporated or redistributed. A license for this project has not been inferred from the supplied files.

## Feedback and related documentation

- [Main repository README](../README.md) — overall project, wiring and Wonder Wheel history.
- [Original demonstration video](https://www.youtube.com/watch?v=-cVcGIMZuJ0) — original Wonder Wheel interface.
- Contact: **WWtoBluetooth@icloud.com**.

*Tested setup reported in the sketch: BMW M 1000 R, Arduino Nano ESP32, iPhone with Scenic, Insta360 GO 3S.*