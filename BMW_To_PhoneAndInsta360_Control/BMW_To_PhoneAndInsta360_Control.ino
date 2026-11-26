/*
 * BMW WONDER WHEEL TO BLUETOOTH KEYBOARD
 * AND INSTA360 GO 3S CAMERA CONTROL
 * ================================================================
 *
 * Edition      : Wonder Wheel + Insta360 Edition
 * Version      : 1.0
 * Author       : Bixio Rimoldi
 * Release date : November 2026
 *
 * PURPOSE
 * -------
 * This sketch provides two functions on a BMW motorcycle:
 *
 * 1. Wonder Wheel control of Scenic on an iPhone. The Arduino
 *    reads the motorcycle's LIN bus and sends Bluetooth Low
 *    Energy (BLE) keyboard events to the phone.
 *
 * 2. Insta360 GO 3S camera control using the separate
 *    turn-signal cancellation button and a BLE connection
 *    between the Arduino and camera.
 *
 * WONDER WHEEL CONTROLS (NAV MODE)
 * --------------------------------
 * Rotate up/down      : UP / DOWN arrow
 * Short push right/left: RIGHT / LEFT arrow
 * Long push right     : RETURN / ENTER
 * Long pull left      : ESCAPE
 *
 * Long lateral presses are recognized after approximately
 * 10 consecutive LIN frames (~0.5 seconds), without waiting
 * for the rider to release the wheel. Only one long-push event
 * is sent per hold; short pushes are reported on release.
 *
 * CAMERA CONTROLS (TURN-SIGNAL CANCELLATION BUTTON)
 * -------------------------------------------------
 * One short press (<1 second):
 *   Take a photo if not recording; stop video if recording.
 *
 * Two short presses (second within 250 ms):
 *   Start video when not recording; if already recording,
 *   stop video.
 *
 * Press for 1 to <10 seconds, then release:
 *   Request camera wake-up or reconnection.
 *
 * Hold for approximately 10 seconds:
 *   Enter Wi-Fi OTA firmware update mode.
 *
 * Ordinary turn-signal cancellation is excluded from camera
 * gestures. Before capture, the firmware requests fresh camera
 * status and may wake or reconnect the camera if necessary.
 * A capture gesture may be discarded if connection or status
 * confirmation fails.
 *
 * OTA FIRMWARE UPDATES
 * --------------------
 * Wi-Fi does not start automatically at power-up. A ~10-second
 * button hold activates the "Motorcycle-OTA" Wi-Fi access
 * point for approximately two minutes (120 seconds).
 * Firmware upload page: http://192.168.4.1
 * Updates require a compiled .bin file and authentication.
 * A successful upload restarts the Arduino.
 *
 * HARDWARE AND SOFTWARE
 * ---------------------
 * Arduino Nano ESP32; NXP TJA1021 LIN transceiver;
 * navigation-ready BMW motorcycle; Insta360 GO 3S;
 * iPhone running Scenic.
 *
 * Tested versions of required library interfaces:
 *   LIN_slave_HardwareSerial_ESP32.h (repository LIN library)
 *   ESP32 NIMBLE Keyboard 2.0.1 (NimBleKeyboard.h)
 *   NimBLE-Arduino 2.5.2 (NimBLEDevice.h)
 *   Arduino ESP32 Boards 2.0.18-arduino.5
 * Board core supplies WiFi, WebServer, Update, FS and FreeRTOS.
 *
 * CONFIGURATION
 * -------------
 * CAMERA_NAME identifies the camera by BLE advertised name.
 * WIFI_SSID and WIFI_PASSWORD configure OTA Wi-Fi.
 * OTA_USERNAME and OTA_PASSWORD protect the upload page.
 * CAMERA_BENCH_SIMULATOR enables simulation of turn-signal
 * cancellation button gestures via the Serial Monitor.
 * VERBOSE_INSTA360_RESPONSES enables raw camera BLE logging.
 *
 * Change the example passwords before deploying the sketch;
 * never publish personal passwords.
 *
 * VALIDATION AND COMPATIBILITY
 * ----------------------------
 * The source reports bench and motorcycle road testing with
 * BMW M 1000 R, Arduino Nano ESP32, Insta360 GO 3S and Scenic.
 * Compatibility with other motorcycle and camera models has
 * not been established.
 *
 * AUTHOR AND ACKNOWLEDGMENTS
 * --------------------------
 * Developed and hardware-tested by Bixio Rimoldi, with
 * ChatGPT (OpenAI) assistance in development, debugging,
 * BLE protocol analysis and documentation.
 *
 * LIN implementation:
 *   https://github.com/gicking/LIN_slave_portable_Arduino
 * ESP32 NIMBLE keyboard:
 *   https://github.com/Berg0162/ESP32-NIMBLE-Keyboard
 * Insta360 protocol research reference:
 *   https://github.com/xaionaro-go/insta360ctl
 *
 * See README.md in this folder for installation, operation,
 * configuration, troubleshooting and safety information.
 *
 * Provided "as is", without warranties. Test while stationary
 * and avoid distraction while riding.
 * ================================================================
 */


// Bench simulator: true for Serial Monitor tests; false on the motorcycle.
constexpr bool CAMERA_BENCH_SIMULATOR = false;  // set false for motorcycle.
// Optional raw BE82 packet logs; never disables recording-state parsing.
constexpr bool VERBOSE_INSTA360_RESPONSES = false;

// include files
#include <LIN_slave_HardwareSerial_ESP32.h>
#include <NimBleKeyboard.h>
#include <NimBLEDevice.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// board pin definitions (Dn is referred to as n on Nano ESP32)

#define PIN_LIN_RX 4  // D4, receive pin for LIN
#define PIN_LIN_TX 6  // D6, transmit pin for LIN


// global gesture variables
bool wheelUp = false;         // WW event: Up
bool wheelDown = false;       // WW event: Down
bool wheelRight = false;      // WW event: Right
bool wheelLeft = false;       // WW event: Left
bool wheelLongRight = false;  // WW event: Long Right
bool wheelLongLeft = false;   // WW event: Long Left

// OTA request and maintenance state
bool otaStartRequested = false;
bool otaModeActive = false;

// Wi-Fi access point configuration and state
const char* WIFI_SSID = "Motorcycle-OTA";
const char* WIFI_PASSWORD = "ChangeThisPassword";  // change it later
const unsigned long WIFI_TIMEOUT = 120000UL;       // 2 minutes
unsigned long wifiStartTime = 0;
bool wifiEnabled = false;
WebServer server(80);

// OTA upload configuration and state
const char* OTA_USERNAME = "admin";
const char* OTA_PASSWORD = "ChooseAnotherStrongPassword";  // change later
const unsigned long OTA_STALL_TIMEOUT = 30000UL;
bool otaInProgress = false;
bool otaSuccess = false;
bool otaFailed = false;
unsigned long otaLastActivity = 0;

// LIN communication state
bool linStarted = false;


// setup LIN node
LIN_Slave_HardwareSerial_ESP32 LIN(Serial2, PIN_LIN_RX, PIN_LIN_TX, LIN_Slave_Base::LIN_V2, "Slave");

// instantiate BleKeyboard
BleKeyboard bleKeyboard("BMW's WW");

// Insta360 GO 3S camera
const char* CAMERA_NAME = "GO 3S 59JUXP";

class Insta360Camera {
public:
  // Public interface used by setup(), loop(), the gesture interpreter, and OTA.
  // Give the two callback objects a reference to their owning camera.
  // Their constructors run as part of the Insta360Camera constructor.
  Insta360Camera()
    : scanCallbacks(*this), clientCallbacks(*this) {}

  // Recording status is still read by the gesture interpreter.
  bool cameraRecording = false;

  void configureScanner() {
    if (!notificationQueue) {
      notificationQueue = xQueueCreate(8, sizeof(ResponseNotification));
      if (!notificationQueue) Serial.println("BE82 status queue allocation failed");
    }
    NimBLEScan* scan = NimBLEDevice::getScan();
    scan->setScanCallbacks(&scanCallbacks);
    scan->setActiveScan(true);
    scan->setInterval(100);
    scan->setWindow(80);
    Serial.println("Camera scanning configured");
  }

  void startInitialization() {
    Serial.println("Entering camera initialization mode");

    startupPending = true;
    state = CameraState::SCANNING;
    initStart = millis();
    startScan();
  }

  // Called from loop(). Return true once when the caller should start LIN.
  // Scan duration, connection guard, and startup timeout are unchanged.
  bool checkInitialization() {
    if (!startupPending) {
      return false;
    }

    if (state == CameraState::SCANNING && scanFinished) {
      scanFinished = false;

      if (found) {
        Serial.println("Insta360 discovered");

        if (millis() - initStart < 6000UL) {
          requestConnection();  // Sets state to CONNECTING if started.
        }
      } else {
        Serial.println("Insta360 not found");
      }
    }

    if (state == CameraState::CONNECTING && connectionResult == ConnectionResult::SUCCEEDED) {
      connectionResult = ConnectionResult::NONE;

      Serial.println("Camera BLE link established");

      if (setupServices()) {
        Serial.println("Camera initialization successful");
      } else {
        state = CameraState::DISCONNECTED;
        Serial.println("Camera initialization failed");
      }

      startupPending = false;
      return true;
    }

    if (state == CameraState::CONNECTING && connectionResult == ConnectionResult::FAILED) {
      connectionResult = ConnectionResult::NONE;

      Serial.println("Camera connection failed");
      state = CameraState::DISCONNECTED;

      startupPending = false;
      return true;
    }

    if (millis() - initStart >= INIT_TIMEOUT) {
      Serial.println("Camera initialization timeout");

      if (NimBLEDevice::getScan()->isScanning()) {
        NimBLEDevice::getScan()->stop();
      }

      if (state == CameraState::CONNECTING && client) {
        client->cancelConnect();
      }
      state = CameraState::DISCONNECTED;
      startupPending = false;

      Serial.println("Continuing without camera if unavailable");
      return true;
    }

    return false;
  }

  // Gesture handlers submit requests without knowing the camera's internals.
  // Processing is deferred until processRequests() runs in loop().
  // All capture gestures now wait for a fresh status query, even if BLE is
  // already connected. BLE READY does not prove the camera is awake.
  bool needsDeferredGesture() const {
    return pendingGesture != PendingGesture::NONE || state != CameraState::READY || !client || !client->isConnected() || !commandChar;
  }
  void requestDeferredShortPress() {
    if (pendingGesture == PendingGesture::NONE) {
      pendingGesture = PendingGesture::SHORT_PRESS;
      pendingGestureAt = millis();
      reconnectStatusConfirmed = false;
      wakeRetryUsed = false;
      if (state == CameraState::READY && client && client->isConnected() && commandChar) {
        // The camera can retain BLE connectivity while sleeping. Probe first;
        // do not transmit PHOTO/VIDEO until a current status is returned.
        Serial.println("Camera gesture queued - checking connected camera responsiveness");
        reconnectStatusQueryPending = true;
        reconnectStatusQueryAt = millis();
      } else {
        Serial.println("Camera gesture queued - reconnecting and checking status");
        requestWakeup();
      }
    }
  }
  void requestDeferredDoublePress() {
    if (pendingGesture == PendingGesture::SHORT_PRESS) {
      pendingGesture = PendingGesture::DOUBLE_PRESS;
      Serial.println("Camera gesture updated: DOUBLE PRESS");
    }
  }

  void requestPhoto() {
    photoRequested = true;
  }
  void requestWakeup() {
    wakeRequested = true;
  }
  void requestVideoToggle() {
    videoToggleRequested = true;
  }
  void requestDirectVideoStart() {
    directVideoStartRequested = true;
  }

  // Request current capture status (command 0x0F).
  void requestStatusQuery() {
    statusQueryRequested = true;
  }

  // Used by the OTA handler before switching off Bluetooth.
  void clearPendingRequests() {
    photoRequested = false;
    wakeRequested = false;
    videoToggleRequested = false;
    statusQueryRequested = false;
    reconnectStatusQueryPending = false;
    reconnectStatusAwaiting = false;
    reconnectStatusConfirmed = false;
    pendingGesture = PendingGesture::NONE;
    wakeRetryUsed = false;
    wakeSettleUntil = 0;
    directVideoStartRequested = false;
  }

  // Process the pending requests in the same order as V16:
  // photograph, wake-up, then video toggle.
  void processRequests() {
    if (photoRequested) {
      photoRequested = false;
      requestCameraPhoto();
    }
    if (wakeRequested) {
      wakeRequested = false;
      requestCameraWakeup();
    }
    if (videoToggleRequested) {
      videoToggleRequested = false;

      if (!cameraRecording) {
        Serial.println("START VIDEO requested");

        if (sendCameraCommand(0x04)) {
          cameraRecording = true;
          Serial.println("Video START command transmitted");
        } else {
          Serial.println("Video START command failed");
        }
      } else {
        Serial.println("STOP VIDEO requested");

        if (sendCameraCommand(0x05)) {
          cameraRecording = false;
          Serial.println("Video STOP command transmitted");
        } else {
          Serial.println("Video STOP command failed");
        }
      }
    }
    // Double press always requests START VIDEO directly, without a photo.
    // Do not turn it into STOP based on an intermediate status update.
    if (directVideoStartRequested) {
      directVideoStartRequested = false;
      if (state == CameraState::READY && client && client->isConnected() && commandChar) {
        Serial.println("Direct START VIDEO requested");
        if (sendCameraCommand(0x04)) {
          cameraRecording = true;
          Serial.println("Video START command transmitted");
        } else {
          Serial.println("Direct START VIDEO command failed");
        }
      } else {
        Serial.println("Direct START VIDEO cancelled - camera disconnected");
      }
    }
    if (statusQueryRequested) {
      statusQueryRequested = false;
      Serial.println("GET_CURRENT_CAPTURE_STATUS (0x0F) requested");
      if (sendCameraCommand(0x0F)) {
        Serial.println("Capture-status query transmitted - awaiting BE82 status");
      } else {
        Serial.println("Capture-status query BLE write failed");
        if (reconnectStatusAwaiting) discardDeferredGesture("Status query BLE write failed");
      }
    }
  }  // processRequests

  // Drain queued BE82 packets in loop(), not in the NimBLE callback.
  // The queue is essential to recording-state synchronization, not merely logging.
  void processResponseNotifications() {
    if (!notificationQueue) return;

    ResponseNotification packet;
    while (xQueueReceive(notificationQueue, &packet, 0) == pdTRUE) {
      synchronizeRecordingState(packet);
      if constexpr (VERBOSE_INSTA360_RESPONSES) {
        Serial.print("BE82 ");
        Serial.print(packet.isNotify ? "notify" : "indicate");
        Serial.print(" (");
        Serial.print(packet.originalLength);
        Serial.print(" bytes):");
        for (size_t i = 0; i < packet.capturedLength; ++i) {
          if (packet.bytes[i] < 0x10) Serial.print(" 0");
          else Serial.print(' ');
          Serial.print(packet.bytes[i], HEX);
        }
        if (packet.capturedLength < packet.originalLength) Serial.print(" [truncated]");
        Serial.println();
      }
    }

    if (droppedNotifications) {
      Serial.print("BE82 status packets dropped (queue full): ");
      Serial.println(droppedNotifications);
      droppedNotifications = 0;
    }
  }

  static constexpr unsigned long GESTURE_DECISION_DELAY_MS = 300UL;

  // A deferred gesture executes only after a fresh, solicited 0x0F response.
  // Allow the shortened double-press window to expire before acting.
  void processDeferredGesture() {
    if (pendingGesture == PendingGesture::NONE) return;
    if (millis() - pendingGestureAt >= GESTURE_RECONNECT_TIMEOUT) {
      discardDeferredGesture("Connection/status timeout");
      return;
    }
    if (reconnectStatusAwaiting && millis() - reconnectStatusQuerySentAt >= STATUS_REPLY_TIMEOUT) {
      reconnectStatusAwaiting = false;
      if (wakeRetryUsed || state != CameraState::READY || !client || !client->isConnected() || !commandChar) {
        discardDeferredGesture("No capture-status reply after wake-up attempt");
        return;
      }
      // First status query failed. The BLE link may exist while the camera
      // is asleep. Wake it once, then query again after a settling interval.
      wakeRetryUsed = true;
      reconnectStatusConfirmed = false;
      Serial.println("No status reply - sending wake-up over existing BLE connection");
      if (!sendCameraAuthorization()) {
        discardDeferredGesture("Wake-up authorization BLE write failed");
        return;
      }
      Serial.println("Camera wake-up command transmitted; waiting before status retry");
      wakeSettleUntil = millis() + WAKE_SETTLE_MS;
      reconnectStatusQueryPending = true;
      reconnectStatusQueryAt = millis() + WAKE_STATUS_RETRY_DELAY_MS;
      return;
    }
    if (state != CameraState::READY || !reconnectStatusConfirmed || millis() - pendingGestureAt < GESTURE_DECISION_DELAY_MS || (wakeSettleUntil && (int32_t)(millis() - wakeSettleUntil) < 0)) return;

    const PendingGesture gesture = pendingGesture;
    pendingGesture = PendingGesture::NONE;
    reconnectStatusConfirmed = false;
    wakeSettleUntil = 0;
    Serial.println("Camera status received - executing deferred gesture");

    if (cameraRecording) {
      // A short press always stops active video, even after a BLE drop.
      // A second press while recording does not force a new recording.
      if (gesture == PendingGesture::DOUBLE_PRESS)
        Serial.println("Camera already recording - double press will stop video");
      requestVideoToggle();
    } else {
      if (gesture == PendingGesture::DOUBLE_PRESS) {
        Serial.println("Deferred double press - starting video directly");
        directVideoStartRequested = true;
      } else {
        requestPhoto();
      }
    }
  }

  // Handle asynchronous BLE disconnection and rider-requested reconnection.
  // Preserve the established event ordering.
  void updateConnection() {
    if (takeDisconnection()) {
      state = CameraState::DISCONNECTED;
      commandChar = nullptr;
      directVideoStartRequested = false;

      Serial.println("Insta360 BLE disconnected");

      reconnectStatusQueryPending = false;
      reconnectStatusAwaiting = false;
      reconnectStatusConfirmed = false;
      wakeSettleUntil = 0;
      if (notificationQueue) xQueueReset(notificationQueue);

      // Deliberately leave the camera disconnected until wake-up is requested.
      Serial.println("Idle reconnection disabled - waiting for rider camera gesture");
    }

    // A CONNECTING camera outside startup is a rider-requested reconnection.
    // CameraState also replaces the former connecting flag.
    if (!initializationActive() && state == CameraState::CONNECTING) {

      if (connectionResult == ConnectionResult::SUCCEEDED) {

        connectionResult = ConnectionResult::NONE;

        Serial.println("CAMERA BLE RECONNECTED SUCCESSFULLY");
        Serial.println("Restoring camera services");

        unsigned long serviceStart = micros();

        bool servicesOK = setupServices();

        unsigned long serviceDuration = micros() - serviceStart;

        Serial.print("Camera service restoration duration: ");
        Serial.print(serviceDuration);
        Serial.println(" us");

        if (servicesOK) {

          Serial.println("CAMERA FULLY READY AFTER RECONNECTION");

          // Query actual recording state after wake-up authorization has been sent.
          // Give the camera a moment to handle the wake-up request first.
          reconnectStatusQueryPending = true;
          reconnectStatusQueryAt = millis() + 500UL;
          reconnectStatusConfirmed = false;

          // Was this reconnection requested to wake the camera?
          if (cameraWakeAfterReconnect) {

            cameraWakeAfterReconnect = false;

            Serial.println("Sending wake-up after reconnection");

            if (sendCameraAuthorization()) {
              Serial.println("Camera wake-up command transmitted");
              if (pendingGesture != PendingGesture::NONE)
                wakeSettleUntil = millis() + WAKE_SETTLE_MS;
            } else {
              Serial.println("Camera wake-up BLE write failed");
              discardDeferredGesture("Wake-up authorization BLE write failed");
            }
          }

        } else {

          Serial.println("Camera service restoration failed");
          state = CameraState::DISCONNECTED;
          cameraWakeAfterReconnect = false;
          discardDeferredGesture("Service restoration failed");
        }


      } else if (connectionResult == ConnectionResult::FAILED) {

        connectionResult = ConnectionResult::NONE;

        Serial.println("CAMERA BLE RECONNECTION FAILED");
        state = CameraState::DISCONNECTED;

        cameraWakeAfterReconnect = false;
        discardDeferredGesture("BLE reconnection failed");
      }
    }

    if (reconnectStatusQueryPending && state == CameraState::READY && (int32_t)(millis() - reconnectStatusQueryAt) >= 0) {
      reconnectStatusQueryPending = false;
      reconnectStatusAwaiting = true;
      reconnectStatusQuerySentAt = millis();
      requestStatusQuery();
      Serial.println("Capture-status query scheduled after reconnection");
    }

  }  // update

private:
  // BE82 status queue: transfers notification bytes safely from the NimBLE
  // callback task to loop() for parsing, without callback-side Serial output.
  static constexpr size_t RESPONSE_CAPTURE_SIZE = 96;
  struct ResponseNotification {
    size_t originalLength;
    size_t capturedLength;
    bool isNotify;
    uint8_t bytes[RESPONSE_CAPTURE_SIZE];
  };
  QueueHandle_t notificationQueue = nullptr;
  volatile uint32_t droppedNotifications = 0;

  // This callback is static; this pointer identifies its owning instance.
  static Insta360Camera*& responseOwner() {
    static Insta360Camera* owner = nullptr;
    return owner;
  }

  // Only accept the two capture-status formats verified on the GO 3S.
  // Response to command 0x0F: message C8 00, protobuf payload 0A ... 08 00/01.
  // Unsolicited status: message 10 20, payload 03 01 08 00/01.
  // Ignore all other notifications, malformed packets, and truncated captures.
  void synchronizeRecordingState(const ResponseNotification& packet) {
    if (packet.capturedLength != packet.originalLength) return;
    const uint8_t* b = packet.bytes;
    const size_t n = packet.capturedLength;
    if (n < 25 || b[0] != 0xFF || b[1] != 0x06 || b[2] != 0x40) return;

    // The documented outer length counts bytes from offset 5 up to the CRC.
    const size_t declared = size_t(b[3]) | (size_t(b[4]) << 8);
    if (declared + 7 != n || b[5] != b[3] || b[6] != b[4]) return;

    bool recognized = false;
    bool recording = false;

    if (b[12] == 0xC8 && b[13] == 0x00 && b[21] == 0x0A && b[23] == 0x08 && b[24] <= 1) {
      // Explicit GET_CURRENT_CAPTURE_STATUS response.
      recognized = true;
      recording = b[24] == 1;
    } else if (b[12] == 0x10 && b[13] == 0x20 && b[19] == 0x03 && b[20] == 0x01 && b[21] == 0x08 && b[22] <= 1) {
      // CURRENT_CAPTURE_STATUS notification (also sent for manual changes).
      recognized = true;
      recording = b[22] == 1;
    }

    if (!recognized) return;
    cameraRecording = recording;
    // Only the explicit 0x0F answer can release a queued gesture.
    if (b[12] == 0xC8 && b[13] == 0x00 && reconnectStatusAwaiting) {
      reconnectStatusAwaiting = false;
      reconnectStatusConfirmed = true;
    }
    Serial.print("Camera capture status confirmed: ");
    Serial.println(recording ? "RECORDING" : "NOT RECORDING");
  }

  // Internal BLE state and initialization bookkeeping.
  bool found = false;
  bool scanFinished = false;
  NimBLEAddress address;

  NimBLEClient* client = nullptr;
  NimBLERemoteCharacteristic* commandChar = nullptr;
  // Connection-result events are owned internally by this camera object.

  // One state for the camera's BLE connection lifecycle.
  // Callback flags still deliver one-shot asynchronous events.
  enum class CameraState {
    IDLE,
    SCANNING,
    CONNECTING,
    DISCOVERING_SERVICES,
    READY,
    DISCONNECTED
  };
  CameraState state = CameraState::IDLE;

  bool cameraWakeAfterReconnect = false;

  enum class PendingGesture : uint8_t { NONE,
                                        SHORT_PRESS,
                                        DOUBLE_PRESS };
  PendingGesture pendingGesture = PendingGesture::NONE;
  unsigned long pendingGestureAt = 0;
  static constexpr unsigned long GESTURE_RECONNECT_TIMEOUT = 18000UL;
  static constexpr unsigned long STATUS_REPLY_TIMEOUT = 4000UL;
  static constexpr unsigned long WAKE_STATUS_RETRY_DELAY_MS = 2500UL;
  static constexpr unsigned long WAKE_SETTLE_MS = 3000UL;
  bool wakeRetryUsed = false;
  unsigned long wakeSettleUntil = 0;
  bool reconnectStatusAwaiting = false;
  bool reconnectStatusConfirmed = false;
  unsigned long reconnectStatusQuerySentAt = 0;
  bool directVideoStartRequested = false;

  void discardDeferredGesture(const char* reason) {
    if (pendingGesture != PendingGesture::NONE) {
      Serial.print("Deferred camera gesture discarded: ");
      Serial.println(reason);
    }
    pendingGesture = PendingGesture::NONE;
    reconnectStatusAwaiting = false;
    reconnectStatusConfirmed = false;
    reconnectStatusQueryPending = false;
    wakeSettleUntil = 0;
  }

  // This flag represents the one-time startup/LIN handoff, not a second
  // connection state. After initialization finishes, state remains READY
  // or DISCONNECTED, while startupPending becomes false.
  bool startupPending = false;
  unsigned long initStart = 0;
  static constexpr unsigned long INIT_TIMEOUT = 10000UL;

  // Keep startupPending private; expose its meaning through this internal helper.
  bool initializationActive() const {
    return startupPending;
  }

  // Pending one-shot requests, owned and consumed by the camera object.
  bool photoRequested = false;
  bool wakeRequested = false;
  bool videoToggleRequested = false;

  // The response to this request can synchronize cameraRecording.
  bool statusQueryRequested = false;
  bool reconnectStatusQueryPending = false;
  unsigned long reconnectStatusQueryAt = 0;

  // A one-shot result published by NimBLE callbacks and consumed by update()
  // or checkInitialization(). This is not a second lifecycle state.
  enum class ConnectionResult : uint8_t { NONE,
                                          SUCCEEDED,
                                          FAILED };
  volatile ConnectionResult connectionResult = ConnectionResult::NONE;
  volatile bool disconnectionPending = false;

  // Internal helpers and NimBLE callback implementations.
  void startScan() {
    found = false;
    scanFinished = false;
    Serial.println("Starting asynchronous camera scan");
    NimBLEDevice::getScan()->start(5000);
  }

  // Discover the BLE service and characteristics after connection.
  bool setupServices() {
    if (!client || !client->isConnected()) {
      state = CameraState::DISCONNECTED;
      return false;
    }

    state = CameraState::DISCOVERING_SERVICES;
    NimBLERemoteService* service = client->getService("BE80");

    if (!service) {
      Serial.println("Camera service not found");
      state = CameraState::DISCONNECTED;
      client->disconnect();
      return false;
    }

    commandChar = service->getCharacteristic("BE81");
    if (!commandChar) {
      Serial.println("Camera characteristics unavailable");
      state = CameraState::DISCONNECTED;
      client->disconnect();
      return false;
    }

    Serial.print("Camera supports write with response: ");
    Serial.println(commandChar->canWrite() ? "YES" : "NO");

    Serial.print("Camera supports write without response: ");
    Serial.println(commandChar->canWriteNoResponse() ? "YES" : "NO");

    NimBLERemoteCharacteristic* response = service->getCharacteristic("BE82");

    responseOwner() = this;
    if (!response || !response->subscribe(true, onCameraResponse)) {
      Serial.println("Camera characteristics unavailable");
      commandChar = nullptr;
      state = CameraState::DISCONNECTED;
      client->disconnect();
      return false;
    }

    state = CameraState::READY;
    Serial.println("Camera ready!");
    return true;
  }  // setupServices

  // Initiate the asynchronous BLE connection (does not wait for completion).
  bool requestConnection() {
    Serial.println("Connecting to camera asynchronously...");

    commandChar = nullptr;

    if (!client) {
      client = NimBLEDevice::createClient();
    }

    if (!client) {
      state = CameraState::DISCONNECTED;
      Serial.println("Could not create camera client");
      return false;
    }

    client->setClientCallbacks(&clientCallbacks, false);
    client->setConnectTimeout(3000);  // limits connection attempt to 3 seconds

    connectionResult = ConnectionResult::NONE;
    state = CameraState::CONNECTING;

    if (!client->connect(address, false, true)) {
      Serial.println("Could not start camera connection");
      state = CameraState::DISCONNECTED;
      return false;
    }

    Serial.println("Camera connection requested");
    return true;
  }  // requestConnection

  // Send a command to the Insta360 GO 3S
  bool sendCameraCommand(uint8_t command) {
    if (!client || !client->isConnected() || !commandChar) {
      Serial.println("Camera not connected");
      return false;
    }

    uint8_t packet[] = {
      0xFF, 0x07, 0x40, 0x10, 0x00,
      0x10, 0x00, 0x00, 0x00,
      0x04, 0x00, 0x00,
      0x03, 0x00,
      0x02, 0x01,
      0x00, 0x00,
      0x80, 0x00, 0x00,
      0x00, 0x00
    };

    packet[12] = command;

    uint16_t crc = crc16(packet, sizeof(packet) - 2);

    packet[sizeof(packet) - 2] = crc & 0xFF;
    packet[sizeof(packet) - 1] = crc >> 8;

    unsigned long writeStart = micros();

    bool ok = commandChar->writeValue(packet, sizeof(packet), false);

    unsigned long writeDuration = micros() - writeStart;

    Serial.print("Camera BLE write duration: ");
    Serial.print(writeDuration);
    Serial.println(" us");

    Serial.println(ok ? "BLE command sent" : "BLE write failed");

    return ok;
  }  // sendCaeraCommand

  bool sendCameraAuthorization() {

    if (!client || !client->isConnected() || !commandChar) {

      Serial.println("Camera not connected");
      return false;
    }

    // Identifier captured from our own iPhone BLE trace.
    // Keep this identifier private.

    const char* authorizationID =
      "F314C6E2-50C4-4D82-A795-14643BD3204F";

    uint8_t packet[43] = {
      0xFF, 0x0C, 0x02, 0x24, 0x00
    };

    // Copy the 36-character identifier into the packet.
    for (int i = 0; i < 36; i++) {
      packet[5 + i] = authorizationID[i];
    }

    // Calculate the final two CRC bytes.
    uint16_t crc = crc16(packet, sizeof(packet) - 2);

    packet[41] = crc & 0xFF;
    packet[42] = crc >> 8;

    Serial.println("Sending captured authorization command");

    bool ok = commandChar->writeValue(
      packet,
      sizeof(packet),
      false);

    Serial.println(
      ok ? "Authorization BLE write succeeded"
         : "Authorization BLE write failed");

    return ok;
  }  // sendCameraAuthorization

  // Request one photograph from the Insta360.
  bool requestCameraPhoto() {

    if (!client || !client->isConnected() || !commandChar) {

      Serial.println("Camera not connected - photo request ignored");
      return false;
    }

    Serial.println("Sending immediate photo command");

    if (sendCameraCommand(0x03)) {
      Serial.println("Photo command transmitted");
      return true;
    } else {
      Serial.println("Photo command BLE write failed");
      return false;
    }
  }

  void requestCameraWakeup() {

    // Case 1: Camera already connected and ready.
    if (client && client->isConnected() && commandChar) {

      Serial.println("Camera connected - sending wake-up");

      unsigned long wakeStart = micros();

      bool wakeOK = sendCameraAuthorization();

      unsigned long wakeDuration = micros() - wakeStart;

      Serial.print("Camera authorization duration: ");
      Serial.print(wakeDuration);
      Serial.println(" us");

      if (wakeOK) {
        Serial.println("Camera wake-up command transmitted");
      } else {
        Serial.println("Camera wake-up BLE write failed");
      }

      return;
    }

    // Case 2: Camera disconnected.
    // Reconnect only because the rider requested wake-up.

    if (state == CameraState::CONNECTING || state == CameraState::DISCOVERING_SERVICES) {
      Serial.println("Camera connection already in progress");
      cameraWakeAfterReconnect = true;
      return;
    }

    Serial.println("Camera disconnected - reconnecting on request");

    cameraWakeAfterReconnect = true;

    if (!requestConnection()) {

      Serial.println("Could not start camera reconnection");

      cameraWakeAfterReconnect = false;
      discardDeferredGesture("Could not start reconnection");
    }
  }  // requestCameraWakeup

  bool takeDisconnection() {
    if (!disconnectionPending) return false;
    disconnectionPending = false;
    return true;
  }

  // Calculate checksum for Insta360 command packets
  uint16_t crc16(const uint8_t* data, size_t length) {
    uint16_t crc = 0xFFFF;

    for (size_t i = 0; i < length; i++) {
      crc ^= data[i];

      for (int bit = 0; bit < 8; bit++) {
        if (crc & 1) {
          crc = (crc >> 1) ^ 0xA001;
        } else {
          crc >>= 1;
        }
      }
    }

    return crc;
  }  // crc16

  // NimBLE callback: queue bytes only; status parsing happens in loop().
  static void onCameraResponse(NimBLERemoteCharacteristic* characteristic,
                               uint8_t* data, size_t length, bool isNotify) {
    Insta360Camera* owner = responseOwner();
    if (!owner || !owner->notificationQueue) return;
    ResponseNotification packet = {};
    packet.originalLength = length;
    packet.capturedLength = length < RESPONSE_CAPTURE_SIZE ? length : RESPONSE_CAPTURE_SIZE;
    packet.isNotify = isNotify;
    if (packet.capturedLength) memcpy(packet.bytes, data, packet.capturedLength);

    if (xQueueSend(owner->notificationQueue, &packet, 0) != pdTRUE) {
      ++owner->droppedNotifications;
    }
  }

  // NimBLE calls these methods; they report results to the owning camera.
  class ScanCallbacks : public NimBLEScanCallbacks {
  public:
    explicit ScanCallbacks(Insta360Camera& camera)
      : owner(camera) {}

    void onResult(const NimBLEAdvertisedDevice* device) override {
      if (device->getName() == CAMERA_NAME) {
        owner.address = device->getAddress();
        owner.found = true;
      }
    }

    void onScanEnd(const NimBLEScanResults& results, int reason) override {
      owner.scanFinished = true;
    }

  private:
    Insta360Camera& owner;
  };  // Class ScanCallbacks

  // Connection events are reported to the owning camera object.
  class ClientCallbacks : public NimBLEClientCallbacks {
  public:
    explicit ClientCallbacks(Insta360Camera& camera)
      : owner(camera) {}

    void onConnect(NimBLEClient* client) override {
      owner.connectionResult = ConnectionResult::SUCCEEDED;
      owner.disconnectionPending = false;
    }

    void onConnectFail(NimBLEClient* client, int reason) override {
      owner.connectionResult = ConnectionResult::FAILED;
    }

    void onDisconnect(NimBLEClient* client, int reason) override {
      owner.disconnectionPending = true;
    }

  private:
    Insta360Camera& owner;
  };

  ScanCallbacks scanCallbacks;
  ClientCallbacks clientCallbacks;
};  // class Insta360Camera

Insta360Camera camera;

// Both connected and disconnected camera gestures are resolved by the same
// deferred status-confirmed path. Double-press recognition is 250 ms.
constexpr unsigned long DOUBLE_PRESS_WINDOW_MS = 250UL;

void setup() {

  Serial.begin(115200);
  // Give the USB Serial Monitor up to two seconds to establish a connection
  // Only needed for monitoring
  unsigned long serialWaitStart = millis();
  while (!Serial && millis() - serialWaitStart < 2000) {
    delay(10);
  }

  // Firmware identification for USB Serial Monitor diagnostics.
  Serial.println("Wonder Wheel + Insta360 Edition - Version 1.0");

  // Wi-Fi OTA is disabled during normal startup.
  // It will be activated later by the 10-second reset gesture.
  Serial.println("Normal startup - skipping Wi-Fi OTA");
  wifiEnabled = false;
  WiFi.mode(WIFI_OFF);

  // Initialize the NimBLE Bluetooth stack through the keyboard.
  // Used to control the Scenic navigation app
  bleKeyboard.begin();

  // Configure Insta360 BLE scanning.
  camera.configureScanner();

  // Start scanning only after Bluetooth and callbacks are ready.
  camera.startInitialization();
}  // setup()

void loop() {

  if (handleOTAMode()) {
    return;
  }

  if (CAMERA_BENCH_SIMULATOR) {
    runCameraBenchSimulator();  // it emulates the turn-signal gestures and the LIN
  } else if (linStarted) {
    dealWithLIN();
  }

  dealWithBluetooth();

  // Start LIN once camera startup completes or times out.
  if (camera.checkInitialization()) {
    startLIN();
  }

  if (handleOTAStartRequest()) {
    return;
  }

  // Process camera disconnect/reconnect events without changing their order.
  camera.updateConnection();
  camera.processResponseNotifications();

  // Consume camera gesture requests in the same order as V16.
  camera.processRequests();
  camera.processDeferredGesture();

}  // loop()

// The interpreter is defined later in this sketch.
void interpretTurnButton(uint8_t data);

void runCameraBenchSimulator() {
  static bool simulatedCenterDown = false;
  static bool simulatedLeftPush = false;
  static bool simulatedRightPush = false;
  static unsigned long simulatedPressStart = 0;

  enum SimState { IDLE,
                  P,
                  V1,
                  V2,
                  W,
                  O,
                  L,
                  R };
  static SimState simState = IDLE;

  char input = '\0';
  if (Serial.available()) {
    input = Serial.read();
  }

  switch (simState) {
    case IDLE:
      if (input == 's') {
        Serial.println("BENCH TEST: Querying current capture status");
        camera.requestStatusQuery();
      } else if (input == 'p' || input == 'v' || input == 'w' || input == 'o') {
        simulatedCenterDown = true;
        simulatedPressStart = millis();
        if (input == 'p') {
          Serial.println("BENCH TEST: Simulating short press");
          simState = P;
        } else if (input == 'v') {
          Serial.println("BENCH TEST: Simulating double press");
          simState = V1;
        } else if (input == 'w') {
          Serial.println("BENCH TEST: Simulating wake-up press");
          simState = W;
        } else {
          Serial.println("BENCH TEST: Simulating OTA hold");
          simState = O;
        }
      } else if (input == 'l' || input == 'r') {
        simulatedLeftPush = (input == 'l');
        simulatedRightPush = (input == 'r');
        simulatedPressStart = millis();
        Serial.println(input == 'l' ? "BENCH TEST: Simulating left turn cancellation"
                                    : "BENCH TEST: Simulating right turn cancellation");
        simState = (input == 'l') ? L : R;
      }
      break;

    case P:
      if (millis() - simulatedPressStart >= 200UL) {
        simulatedCenterDown = false;
        simState = IDLE;
      }
      break;

    case V1:
      if (millis() - simulatedPressStart >= 200UL) {
        simulatedCenterDown = false;
        simulatedPressStart = millis();
        simState = V2;
      }
      break;

    case V2:
      if (millis() - simulatedPressStart >= 200UL) {
        simulatedCenterDown = true;
        simulatedPressStart = millis();
        simState = P;
      }
      break;

    case W:
      if (millis() - simulatedPressStart >= 3000UL) {
        simulatedCenterDown = false;
        simState = IDLE;
      }
      break;

    case O:
      if (millis() - simulatedPressStart >= 11000UL) {
        simulatedCenterDown = false;
        simState = IDLE;
      }
      break;

    case L:
    case R:
      if (millis() - simulatedPressStart >= 2000UL) {
        simulatedCenterDown = true;
        simulatedPressStart = millis();
        simState = P;
      } else if (millis() - simulatedPressStart >= 1000UL) {
        simulatedLeftPush = false;
        simulatedRightPush = false;
      }
      break;
  }

  uint8_t simulatedLIN = 0x00;
  if (simulatedLeftPush) simulatedLIN |= 0x10;
  if (simulatedRightPush) simulatedLIN |= 0x20;
  if (simulatedCenterDown) simulatedLIN |= 0x40;
  interpretTurnButton(simulatedLIN);
}



// Handle a pending OTA gesture; return true when loop() must stop processing.
bool handleOTAStartRequest() {
  // Bench tests must observe the 10-second gesture without entering Wi-Fi OTA.
  // Disable CAMERA_BENCH_SIMULATOR to restore the original OTA behavior.
  if (CAMERA_BENCH_SIMULATOR && otaStartRequested) {
    Serial.println("BENCH TEST: OTA request received (OTA not started)");
    otaStartRequested = false;
  }

  if (otaStartRequested) {
    otaStartRequested = false;

    Serial.println("OTA request received by main loop");

    // Stop LIN first.
    stopLIN();

    // Suspend riding-mode processing, but keep the BLE stack running.
    otaModeActive = true;

    // Discard pending camera actions.
    camera.clearPendingRequests();

    Serial.println("OTA maintenance mode entered - Bluetooth stays active");
    Serial.println("Starting Wi-Fi OTA access point");

    WiFi.mode(WIFI_AP);

    if (WiFi.softAP(WIFI_SSID, WIFI_PASSWORD)) {

      wifiStartTime = millis();
      wifiEnabled = true;

      Serial.println("Wi-Fi OTA started successfully");
      Serial.print("OTA address: ");
      Serial.println(WiFi.softAPIP());

      // Reuse our existing firmware update handlers.
      server.on("/", HTTP_GET, handleRoot);

      server.on(
        "/update",
        HTTP_POST,
        handleUpdateComplete,
        handleUpdateUpload);

      server.begin();

      Serial.println("OTA web server ready");

    } else {

      wifiEnabled = false;
      WiFi.mode(WIFI_OFF);

      Serial.println("Wi-Fi OTA failed to start - returning to Riding mode");
      otaModeActive = false;
      startLIN();
    }

    return true;
  }

  return false;
}

// Service OTA maintenance mode without processing riding-mode inputs.
// Return true when loop() must skip its normal work.
bool handleOTAMode() {
  if (!otaModeActive) {
    return false;
  }
  if (wifiEnabled) {
    server.handleClient();
  }
  checkOTAStall();
  checkOTATimeout();
  return true;
}



void startLIN() {
  if (linStarted) {
    return;
  }
  // open LIN interface
  LIN.begin(19200);
  // Register callback function for frame 14h, which carries WW movements
  LIN.registerMasterRequestHandler(0x14, handle_Request14, 8);
  // Register callback function for frame 2Bh.
  // Note1: Without this response, the bike assumes that there is no navigator, ...
  // ... then the WW inputs always go to the TFT display, and neither TFT nor NAV are shown on the display.
  LIN.registerSlaveResponseHandler(0x2B, handle_Response2B, 8);  // can be used to reset, e.g., the daily Km count
  // Register callback function for frame 28h.
  // Note2: same as Note 1.
  LIN.registerSlaveResponseHandler(0x28, handle_Response28, 8);  // can be used to override date and time
  linStarted = true;
  Serial.println("LIN started - Riding mode");
}

void stopLIN() {

  if (!linStarted) {
    return;
  }

  Serial.println("Stopping LIN communication");

  linStarted = false;
  LIN.end();

  Serial.println("LIN stopped - OTA preparation");
}

void handleRoot() {
  if (!server.authenticate(OTA_USERNAME, OTA_PASSWORD)) {
    server.requestAuthentication();
    return;
  }

  server.send(
    200,
    "text/html",
    "<html><body>"
    "<h1>Motorcycle OTA</h1>"
    "<p>Select a compiled firmware .bin file:</p>"
    "<form method='POST' action='/update' enctype='multipart/form-data'>"
    "<input type='file' name='firmware' accept='.bin' required>"
    "<input type='submit' value='Upload firmware'>"
    "</form>"
    "</body></html>");
}

// to receives and write the firmware
void handleUpdateUpload() {
  // Check credentials before processing firmware data.
  if (!server.authenticate(OTA_USERNAME, OTA_PASSWORD)) {
    return;
  }

  HTTPUpload& upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    otaInProgress = true;
    wifiStartTime = millis();
    otaSuccess = false;
    otaFailed = false;
    otaLastActivity = millis();

    Serial.println("OTA upload starting");

    if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) {
      otaFailed = true;
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    otaLastActivity = millis();
    if (!otaFailed && Update.isRunning()) {
      if (Update.write(upload.buf, upload.currentSize)
          != upload.currentSize) {
        otaFailed = true;
        Update.printError(Serial);
        Update.abort();
      }
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (!otaFailed && Update.isRunning() && Update.end(true)) {
      otaSuccess = true;
      Serial.println("OTA upload successful");
    } else {
      otaFailed = true;
      otaSuccess = false;

      Serial.println("OTA upload failed");
      Update.printError(Serial);
      Update.abort();
    }
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    Update.abort();

    otaFailed = true;
    otaSuccess = false;
    otaInProgress = false;

    wifiStartTime = millis();

    Serial.println("OTA upload aborted");
  }
}

// to report the result and restart the Nano after a successful update
void handleUpdateComplete() {
  if (!server.authenticate(OTA_USERNAME, OTA_PASSWORD)) {
    server.requestAuthentication();
    otaInProgress = false;
    return;
  }

  if (otaSuccess) {
    server.send(
      200,
      "text/plain",
      "Firmware updated successfully. Restarting...");
  } else {
    server.send(
      500,
      "text/plain",
      "Firmware update failed.");
  }
  if (otaSuccess) {
    delay(500);
    ESP.restart();
  }

  otaInProgress = false;
}

void checkOTAStall() {
  if (otaInProgress && millis() - otaLastActivity >= OTA_STALL_TIMEOUT) {

    Serial.println("OTA upload stalled - aborting");

    Update.abort();

    otaFailed = true;
    otaSuccess = false;
    otaInProgress = false;

    wifiStartTime = millis();
  }
}

void checkOTATimeout() {
  if (wifiEnabled && !otaInProgress && millis() - wifiStartTime >= WIFI_TIMEOUT) {

    Serial.println("OTA window expired - returning to Riding mode");

    server.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    wifiEnabled = false;
    otaModeActive = false;
    // BLE was never shut down; retain its camera and keyboard state.
    startLIN();
  }
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++
void dealWithLIN() {
  if (LIN.available()) {
    LIN.handler();
    // if LIN frame has finished, acto on it
    if (LIN.getState() == LIN_Slave_Base::STATE_DONE) {
      LIN_Slave_Base::frame_t Type;
      LIN_Slave_Base::error_t error;
      uint8_t Id;
      uint8_t NumData;
      uint8_t Data[8];

      // get frame data & error status
      LIN.getFrame(Type, Id, NumData, Data);
      error = LIN.getError();

      if (error != 0) {
        Serial.print("LIN error detected: ");
        Serial.println(static_cast<int>(error));
      }

      // reset state machine & error
      LIN.resetStateMachine();
      LIN.resetError();
    }  // LIN frame finished
  }    // if LIN available
}  // dealWithLin()

// ++++++++++++++++++++++++++++++++++++++++++++++++++
void dealWithBluetooth() {
  if (wheelUp) {
    bleKeyboard.write(KEY_UP_ARROW);
    wheelUp = false;
  } else if (wheelDown) {
    bleKeyboard.write(KEY_DOWN_ARROW);
    wheelDown = false;
  } else if (wheelRight) {
    bleKeyboard.write(KEY_RIGHT_ARROW);
    wheelRight = false;
  } else if (wheelLeft) {
    bleKeyboard.write(KEY_LEFT_ARROW);
    wheelLeft = false;
  } else if (wheelLongLeft) {
    bleKeyboard.write(KEY_ESC);
    wheelLongLeft = false;
  } else if (wheelLongRight) {
    bleKeyboard.write(KEY_RETURN);
    wheelLongRight = false;
  }
}  // dealWithBluetooth()


// ++++++++++++++++++++++++++++++++++++++++++++++++++
// Master Request handler for ID 14h (frame 14h carries the WW inputs)
// From this frame we are just reaing out the infos
void handle_Request14(uint8_t NumData, uint8_t* Data) {
  // avoid unused parameter warning
  (void)NumData;

  interpretTurnButton(Data[1]);  // new version, sgate machine based, easier to follow

  interpretWonderWheel(Data);
}


// ++++++++++++++++++++++++++++++++++++++++++++++++++
// Slave Response handler for ID 2Bh
// Note8: A response (not any) is needed to trick the bike into thinking that there is a navigator ...
// the response that we are giving is not changing any of the bike settings ...
// but we could respond in such a way as to reset, say, the daily Km count
void handle_Response2B(uint8_t NumData, uint8_t* Data) {
  // avoid unused parameter warning
  (void)NumData;
  (void)Data;

  Data[0] = 0x01;  // 0x01, 0x02, 0x04, 0x08 == do not reset, reset km 1, reset km 2, reset both
  Data[1] = 0x15;  // don't reset (average fuel consumption, total fuel consumption, average speed)
  Data[2] = 0x15;  // don't reset (various energy consumptions)
  Data[3] = 0;
  Data[4] = 0;
  Data[5] = 0;
  Data[6] = 0;
  Data[7] = 0;
}  // handle_Response()


// ++++++++++++++++++++++++++++++++++++++++++++++++++
// Slave Response handler ID 28h
void handle_Response28(uint8_t NumData, uint8_t* Data)
// Note9: a response is needed (see note8).
// with this response we have the option to set the date and time on the bike's computer ...
// ... but we are not doing so.
{
  // avoid unused parameter warning
  (void)NumData;
  (void)Data;

  Data[0] = 0xFF;  // seconds; FF means invalid signal (any other value will force that value)
  Data[1] = 0xFF;  // minutes; FF means invalid signal (any other value will force that value)
  Data[2] = 0xFF;  // hours; FF means invalid signal (any other value will force that value)
  Data[3] = 0xFF;  // day; FF means invalid signal (any other value will force that value)
  Data[4] = 0xFF;  // year and month; ; FF means invalid signal (any other value will force that value)
  Data[5] = 0xFF;  // year (12 bits total); ; FF means invalid signal (any other value will force that value)
  Data[6] = 0x00;  // I am not sure if this value matters
  Data[7] = 0x00;  // 0x80 means error; 0x00 means no error; This signal must be present for LIN 2.0

}  // handle_Response()

// Turn-signal reset button gesture state machine

void interpretTurnButton(uint8_t data) {

  constexpr unsigned long SHORT_PRESS_TIME = 1000UL;  // UL means unsigned long
  constexpr unsigned long LONG_PRESS_TIME = 10000UL;


  // turn button positions
  bool buttonIsLeft = (data & 0x10) != 0;
  bool buttonIsRight = (data & 0x20) != 0;
  bool buttonIsDown = (data & 0x40) != 0;

  static bool oldButtonIsLeft = false;
  static bool oldButtonIsRight = false;
  static bool oldButtonIsDown = false;

  // convert into turn button movements
  bool leftPressed = (oldButtonIsLeft != buttonIsLeft) && buttonIsLeft;
  bool rightPressed = (oldButtonIsRight != buttonIsRight) && buttonIsRight;
  bool centerPressed = (oldButtonIsDown != buttonIsDown) && buttonIsDown;
  bool centerReleased = (oldButtonIsDown != buttonIsDown) && !buttonIsDown;

  static bool neglectNextPress = false;
  static bool neglectNextRelease = false;

  static unsigned long lastCenterPressTime = 0;
  static unsigned long lastCenterReleaseTime = 0;

  enum class GestureState {
    INITIAL,
    CENTER_PRESSED,
    SHORT_PRESS_REGISTERED
  };

  static GestureState gestureState = GestureState::INITIAL;

  switch (gestureState) {
    case GestureState::INITIAL:
      {
        if (leftPressed || rightPressed) {
          Serial.println("Turn signal is activated");
          neglectNextPress = true;
        } else if (centerPressed && neglectNextPress) {
          Serial.println("Turn signal cancellation - Nano ignores gesture");
          neglectNextPress = false;
          neglectNextRelease = true;
        } else if (centerReleased && neglectNextRelease) {
          Serial.println("Ignored release of already processed center press");
          neglectNextRelease = false;
        } else if (centerPressed && !neglectNextPress) {
          Serial.println("Turn reset pushed");
          lastCenterPressTime = millis();
          gestureState = GestureState::CENTER_PRESSED;
        }
        break;
      }
    case GestureState::CENTER_PRESSED:
      {
        unsigned long pressDuration = millis() - lastCenterPressTime;

        if (centerReleased && pressDuration < SHORT_PRESS_TIME) {
          Serial.print("Turn reset released after ");
          Serial.print(pressDuration);
          Serial.println(" ms");
          // Always solicit current status before capture. The second press
          // upgrades this gesture during the 250 ms recognition window.
          camera.requestDeferredShortPress();
          lastCenterReleaseTime = millis();
          gestureState = GestureState::SHORT_PRESS_REGISTERED;
        } else if (centerReleased && pressDuration >= SHORT_PRESS_TIME && pressDuration < LONG_PRESS_TIME) {
          Serial.print("Turn reseet released after ");
          Serial.print(pressDuration);
          Serial.println(" ms");
          Serial.println("Longer press detected -- camera wakeup requested");
          camera.requestWakeup();
          gestureState = GestureState::INITIAL;
        } else if (pressDuration >= LONG_PRESS_TIME) {
          Serial.print("OTA hold duration: ");
          Serial.print(pressDuration);
          Serial.println(" ms");
          Serial.println("requesting OTA");
          otaStartRequested = true;
          neglectNextRelease = !centerReleased;  // between function calls, the button could have been released
          gestureState = GestureState::INITIAL;
        }
        break;
      }

    case GestureState::SHORT_PRESS_REGISTERED:
      {
        if (leftPressed || rightPressed) {
          neglectNextPress = true;
          gestureState = GestureState::INITIAL;

        } else if (centerPressed && !neglectNextPress) {
          // While reconnecting, upgrade the deferred action instead of
          // transmitting a command before recording state is known.
          camera.requestDeferredDoublePress();
          neglectNextRelease = true;
          gestureState = GestureState::INITIAL;

        } else if (millis() - lastCenterReleaseTime >= DOUBLE_PRESS_WINDOW_MS) {
          // No second press: return to initial state
          gestureState = GestureState::INITIAL;
        }
        break;
      }
  }

  // Save button positions for the next call
  oldButtonIsLeft = buttonIsLeft;
  oldButtonIsRight = buttonIsRight;
  oldButtonIsDown = buttonIsDown;
}

void interpretWonderWheel(uint8_t data[]) {

  static uint8_t OldWheelPosition = 0;
  static uint8_t RightCount = 0;
  static uint8_t LeftCount = 0;
  static bool RightLongSent = false;
  static bool LeftLongSent = false;
  static bool ID_14h_VISIBLE = false;

  uint8_t NewWheelPosition;
  int Increment = 0;

  // Check if wheel turned
  if (!ID_14h_VISIBLE)
  // The first passage is special, we jst get the reference
  {
    OldWheelPosition = data[5];
    ID_14h_VISIBLE = true;
  } else
  // ID 14h has been seen before, hence the OldWheelPosition has the correct value
  {
    // check if wheel has been turned
    // Note5: the wheel returns an integer modulo 255 (not modulo 256) ...
    // it increases from 0 to 254 as we rotate up (forward), and decreases as we rotate down (backwards).
    // Note6: between consecutive frames (50 milliseconds), the wheel can only increment or decrement by 1 position
    NewWheelPosition = data[5];
    Increment = NewWheelPosition - OldWheelPosition;  // takes values in {+/- 1, +/-254}
    Increment = (Increment + 255) % 255;              // it evaluates to 1 for wheel up by one position and to 254 for wheel down by one position
    switch (Increment) {
      case 1:
        wheelUp = true;
        break;
      case 254:
        wheelDown = true;
        break;
      case 0:
        // do nothing
        break;
      default:
        // keep this for debugging purposes
        break;
    }

    OldWheelPosition = NewWheelPosition;
  }  // check wheel turned

  // Check wheel pushed right
  // Note7: a short right push lasts long enough to be carried by, say 5 frames
  if (data[3] == 0x02)  // indicating right push
  {
    if (!RightLongSent) {
      RightCount++;

      if (RightCount >= 10) {
        wheelLongRight = true;
        RightLongSent = true;
      }
    }
  } else if (data[3] == 0x00) {
    if (RightCount > 0 && !RightLongSent)  // we just let go the right push
    {
      wheelRight = true;
    }

    RightCount = 0;
    RightLongSent = false;
  }  //check wheel pushed right

  // Check wheel pushed left
  if (data[3] == 0x01) {
    if (!LeftLongSent) {
      LeftCount++;

      if (LeftCount >= 10) {
        wheelLongLeft = true;
        LeftLongSent = true;
      }
    }
  } else if (data[3] == 0x00) {
    if (LeftCount > 0 && !LeftLongSent) {
      wheelLeft = true;
    }  // Check wheel pushed left

    LeftCount = 0;
    LeftLongSent = false;
  }  // Check wheel pushed left

  // Menu Down: transfer control from Navigator to TFT
  if (data[2] == 0x10) {
    ID_14h_VISIBLE = false;
  }

}  // master request handler ID 14h