Wonder Wheel to Bluetooth Keystroke Converter
Control the Scenic navigation app via your BMW motorcycle’s Wonder Wheel. 

Project Overview

This project enables the use of a BMW motorcycle’s Multi-Function Controller, often called the Wonder Wheel (WW) to control the Scenic navigation app (running on an iPhone; an Android version has been announced). It works by implementing a LIN slave protocol that captures WW inputs and converts them into Bluetooth keystrokes, which are then interpreted by the app. 

This implementation can also serve as a foundation for other projects that generate Bluetooth keystrokes in response to LIN bus messages.

A commercial version with similar functionality is the WunderLINQ X productd by Black Box Embedded. (We started this project when WunderLINQ X was not yet on the market.)

Safety & Liability

⚠️ Disclaimer
	•	This project is provided "as is" with no guarantees of functionality or compatibility.
	•	I am not responsible for accidents, injuries, or legal violations resulting from its use. Riding while distracted is dangerous. Always:
	⁃	Test features in a safe environment first.
	⁃	Prioritize road safety over interface interactions.
	⁃	Comply with local laws regarding device use while riding.

By using this project, you assume all risks associated with its operation.

Hardware & Implementation

Core components:
	•	Arduino Nano ESP32 (LIN + Bluetooth)
	•	An NXP TJA1021 LIN transceiver

The electrical circuit is implemented on a compact 7cm × 3cm PCB.

Provided files: 
	•	Arduino source code
	•	Circuit schematics
	•	3D-printable case design

Installation

The device connects to the motorcycle via a 3-wire harness (typically located under the left-side tank cover), providing:
	•	+12V, GND, LIN bus

Prerequisites

The motorcycle’s software must be navigation-ready. This is standard on models with a factory-installed navigator cradle, like the GS or the XR. For a sportier bike, a BMW dealer might have to make it navigation-ready via a software update.

A navigation-ready bike checks at regular intervals for a response from the dedicated LIN bus indicating that BMW’s Sat Nav is present. (This project’s LIN slave provides the same response.) If it is the case, a "TFT" message appears in the top-left corner of the TFT display. This tells the rider that the WW  inputs control the TFT menu. (In the absence of a navigator, the WW always controls the TFT menu.)

Operation

	1.	Switching to NAV Mode: High-press and hold the Menu rocker-button (left handlebar) to switch to NAV mode. At this point the "TFT" message on the display is replaced by "NAV". In NAV mode, WW inputs are ignored by the TFT display and instead they are forwarded to the LIN bus dedicated to the navigator, allowing this project’s LIN slave to capture and convert them into Bluetooth keystrokes. 
	2.	Input Mappings:
	⁃	Left/Right WW pushes → Arrow keys (← / →)
	⁃	Up/Down WW rotations → Arrow keys (↑ / ↓)
	⁃	Long right WW push → ENTER
	⁃	Long left WW push → ESCAPE
	⁃	(A low-press of the Menu button reverts to TFT mode.)

Compatibility

Works with any app supporting Bluetooth keyboard inputs (arrows + ENTER).

Credit

The LIN slave implementation is based on Georg Icking-Konert’s work. I am grateful to him for his interest to this project and for his guidance in getting the LIN slave to work on the Arduino Nano ESP32. 

Installation Instructions
(To be added)

Demo Video
(To be added)

Feedback
Send feedback at WWtoBluetooth@icloud.com
