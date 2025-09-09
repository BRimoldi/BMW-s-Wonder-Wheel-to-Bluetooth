{\rtf1\ansi\ansicpg1252\cocoartf2639
\cocoatextscaling0\cocoaplatform0{\fonttbl\f0\fnil\fcharset0 HelveticaNeue;\f1\fnil\fcharset0 HelveticaNeue-Bold;\f2\fnil\fcharset0 AppleColorEmoji;
\f3\fnil\fcharset0 LucidaGrande;}
{\colortbl;\red255\green255\blue255;\red0\green0\blue0;\red23\green90\blue226;}
{\*\expandedcolortbl;;\cssrgb\c0\c0\c0\c87059;\cssrgb\c10196\c45098\c90980;}
{\*\listtable{\list\listtemplateid1\listhybrid{\listlevel\levelnfc23\levelnfcn23\leveljc0\leveljcn0\levelfollow0\levelstartat1\levelspace360\levelindent0{\*\levelmarker \{disc\}}{\leveltext\leveltemplateid1\'01\uc0\u8226 ;}{\levelnumbers;}\fi-360\li720\lin720 }{\listlevel\levelnfc23\levelnfcn23\leveljc0\leveljcn0\levelfollow0\levelstartat1\levelspace360\levelindent0{\*\levelmarker \{hyphen\}}{\leveltext\leveltemplateid2\'01\uc0\u8259 ;}{\levelnumbers;}\fi-360\li1440\lin1440 }{\listname ;}\listid1}
{\list\listtemplateid2\listhybrid{\listlevel\levelnfc23\levelnfcn23\leveljc0\leveljcn0\levelfollow0\levelstartat1\levelspace360\levelindent0{\*\levelmarker \{disc\}}{\leveltext\leveltemplateid101\'01\uc0\u8226 ;}{\levelnumbers;}\fi-360\li720\lin720 }{\listname ;}\listid2}
{\list\listtemplateid3\listhybrid{\listlevel\levelnfc23\levelnfcn23\leveljc0\leveljcn0\levelfollow0\levelstartat1\levelspace360\levelindent0{\*\levelmarker \{disc\}}{\leveltext\leveltemplateid201\'01\uc0\u8226 ;}{\levelnumbers;}\fi-360\li720\lin720 }{\listname ;}\listid3}
{\list\listtemplateid4\listhybrid{\listlevel\levelnfc23\levelnfcn23\leveljc0\leveljcn0\levelfollow0\levelstartat1\levelspace360\levelindent0{\*\levelmarker \{disc\}}{\leveltext\leveltemplateid301\'01\uc0\u8226 ;}{\levelnumbers;}\fi-360\li720\lin720 }{\listname ;}\listid4}
{\list\listtemplateid5\listhybrid{\listlevel\levelnfc0\levelnfcn0\leveljc0\leveljcn0\levelfollow0\levelstartat1\levelspace360\levelindent0{\*\levelmarker \{decimal\}.}{\leveltext\leveltemplateid401\'02\'00.;}{\levelnumbers\'01;}\fi-360\li720\lin720 }{\listlevel\levelnfc23\levelnfcn23\leveljc0\leveljcn0\levelfollow0\levelstartat1\levelspace360\levelindent0{\*\levelmarker \{hyphen\}}{\leveltext\leveltemplateid402\'01\uc0\u8259 ;}{\levelnumbers;}\fi-360\li1440\lin1440 }{\listname ;}\listid5}
{\list\listtemplateid6\listhybrid{\listlevel\levelnfc23\levelnfcn23\leveljc0\leveljcn0\levelfollow0\levelstartat1\levelspace360\levelindent0{\*\levelmarker \{disc\}}{\leveltext\leveltemplateid501\'01\uc0\u8226 ;}{\levelnumbers;}\fi-360\li720\lin720 }{\listname ;}\listid6}}
{\*\listoverridetable{\listoverride\listid1\listoverridecount0\ls1}{\listoverride\listid2\listoverridecount0\ls2}{\listoverride\listid3\listoverridecount0\ls3}{\listoverride\listid4\listoverridecount0\ls4}{\listoverride\listid5\listoverridecount0\ls5}{\listoverride\listid6\listoverridecount0\ls6}}
\paperw11900\paperh16840\margl1440\margr1440\vieww17660\viewh12980\viewkind0
\deftab720
\pard\pardeftab720\partightenfactor0

\f0\fs28 \cf2 \expnd0\expndtw0\kerning0
\
\pard\pardeftab720\sa274\partightenfactor0

\f1\b\fs32 \cf2 Wonder Wheel to Bluetooth Keystroke Converter\
\pard\pardeftab720\partightenfactor0

\f0\b0\fs28 \cf2 Control the Scenic navigation app via your BMW motorcycle\'92s Wonder Wheel. \
\pard\pardeftab720\partightenfactor0

\f1\b \cf2 \

\fs30 Project Overview\
\pard\pardeftab720\partightenfactor0

\f0\b0\fs28 \cf2 \
This project enables the use of a BMW motorcycle\'92s Multi-Function Controller, often called the 
\f1\b Wonder Wheel
\f0\b0  (WW) to control the 
\f1\b Scenic navigation app
\f0\b0  (running on an iPhone; an Android version has been announced). It works by implementing a 
\f1\b LIN slave protocol
\f0\b0  that captures WW inputs and converts them into
\f1\b  Bluetooth keystrokes
\f0\b0 , which are then interpreted by the app. \
\
This implementation can also serve as a foundation for other projects that generate Bluetooth keystrokes in response to LIN bus messages.\
\
A commercial version with similar functionality is the {\field{\*\fldinst{HYPERLINK "https://blackboxembedded.com/blogs/news/announcing-the-wunderlinq-x"}}{\fldrslt WunderLINQ X}} productd by Black Box Embedded. (We started this project when WunderLINQ X was not yet on the market.)\
\
\pard\pardeftab720\partightenfactor0

\f1\b\fs30 \cf2 Safety & Liability\
\pard\pardeftab720\partightenfactor0

\f0\b0\fs28 \cf2 \
\pard\pardeftab720\partightenfactor0

\f2 \cf2 \uc0\u9888 \u65039 
\f0  
\f1\b Disclaimer
\f0\b0 \
\pard\tx220\tx720\pardeftab720\li720\fi-720\partightenfactor0
\ls1\ilvl0\cf2 \kerning1\expnd0\expndtw0 {\listtext	\uc0\u8226 	}\expnd0\expndtw0\kerning0
This project is provided "as is" with no guarantees of functionality or compatibility.\
\ls1\ilvl0\kerning1\expnd0\expndtw0 {\listtext	\uc0\u8226 	}
\f1\b \expnd0\expndtw0\kerning0
I am not responsible for accidents, injuries, or legal violations
\f0\b0  resulting from its use. Riding while distracted is dangerous. Always:\
\pard\tx940\tx1440\pardeftab720\li1440\fi-1440\partightenfactor0
\ls1\ilvl1\cf2 \kerning1\expnd0\expndtw0 {\listtext	\uc0\u8259 	}\expnd0\expndtw0\kerning0
Test features in a safe environment first.\
\ls1\ilvl1\kerning1\expnd0\expndtw0 {\listtext	\uc0\u8259 	}\expnd0\expndtw0\kerning0
Prioritize road safety over interface interactions.\
\ls1\ilvl1\kerning1\expnd0\expndtw0 {\listtext	\uc0\u8259 	}\expnd0\expndtw0\kerning0
Comply with local laws regarding device use while riding.\
\pard\pardeftab720\partightenfactor0
\cf2 \
By using this project, you assume all risks associated with its operation.\
\
\pard\pardeftab720\partightenfactor0

\f1\b\fs30 \cf2 Hardware & Implementation\
\pard\pardeftab720\partightenfactor0

\f0\b0\fs28 \cf2 \
Core components:\
\pard\tx220\tx720\pardeftab720\li720\fi-720\partightenfactor0
\ls2\ilvl0\cf2 \kerning1\expnd0\expndtw0 {\listtext	\uc0\u8226 	}\expnd0\expndtw0\kerning0
Arduino Nano ESP32 (LIN + Bluetooth)\
\ls2\ilvl0\kerning1\expnd0\expndtw0 {\listtext	\uc0\u8226 	}\expnd0\expndtw0\kerning0
An NXP TJA1021 LIN transceiver\
\pard\pardeftab720\partightenfactor0
\cf2 \
The electrical circuit is implemented on a compact 7cm \'d7 3cm PCB.\
\
Provided files: \
\pard\tx220\tx720\pardeftab720\li720\fi-720\partightenfactor0
\ls3\ilvl0\cf2 \kerning1\expnd0\expndtw0 {\listtext	\uc0\u8226 	}\expnd0\expndtw0\kerning0
Arduino source code\
\ls3\ilvl0\kerning1\expnd0\expndtw0 {\listtext	\uc0\u8226 	}\expnd0\expndtw0\kerning0
Circuit schematics\
\ls3\ilvl0\kerning1\expnd0\expndtw0 {\listtext	\uc0\u8226 	}\expnd0\expndtw0\kerning0
3D-printable case design\
\pard\pardeftab720\partightenfactor0
\cf2 \
\pard\pardeftab720\partightenfactor0

\f1\b\fs30 \cf2 Installation\
\pard\pardeftab720\partightenfactor0

\f0\b0\fs28 \cf2 \
The device connects to the motorcycle via a 3-wire harness (typically located under the left-side tank cover), providing:\
\pard\tx220\tx720\pardeftab720\li720\fi-720\partightenfactor0
\ls4\ilvl0\cf2 \kerning1\expnd0\expndtw0 {\listtext	\uc0\u8226 	}\expnd0\expndtw0\kerning0
+12V, GND, LIN bus\
\pard\pardeftab720\partightenfactor0
\cf2 \
\pard\pardeftab720\partightenfactor0

\f1\b\fs30 \cf2 Prerequisites\
\pard\pardeftab720\partightenfactor0

\f0\b0\fs28 \cf2 \
The motorcycle\'92s software must be navigation-ready. This is standard on models with a factory-installed navigator cradle, like the GS or the XR. For a sportier bike, a BMW dealer might have to make it navigation-ready via a software update.\
\
A navigation-ready bike checks at regular intervals for a response from the dedicated LIN bus indicating that BMW\'92s Sat Nav is present. (This project\'92s LIN slave provides the same response.) If it is the case, a "TFT" message appears in the top-left corner of the TFT display. This tells the rider that the WW  inputs control the TFT menu. (In the absence of a navigator, the WW always controls the TFT menu.)\
\
\pard\pardeftab720\partightenfactor0

\f1\b\fs30 \cf2 Operation\
\pard\pardeftab720\partightenfactor0

\f0\b0\fs28 \cf2 \
\pard\tx220\tx720\pardeftab720\li720\fi-720\partightenfactor0
\ls5\ilvl0
\f1\b \cf2 \kerning1\expnd0\expndtw0 {\listtext	1.	}\expnd0\expndtw0\kerning0
Switching to NAV Mode: 
\f0\b0 High-press and hold the Menu rocker-button (left handlebar) to switch to NAV mode. At this point the "TFT" message on the display is replaced by "NAV". In NAV mode, WW inputs are ignored by the TFT display and instead they are forwarded to the LIN bus dedicated to the navigator, allowing this project\'92s LIN slave to capture and convert them into Bluetooth keystrokes. \
\ls5\ilvl0
\f1\b \kerning1\expnd0\expndtw0 {\listtext	2.	}\expnd0\expndtw0\kerning0
Input Mappings:\
\pard\tx940\tx1440\pardeftab720\li1440\fi-1440\partightenfactor0
\ls5\ilvl1
\f0\b0 \cf2 \kerning1\expnd0\expndtw0 {\listtext	\uc0\u8259 	}\expnd0\expndtw0\kerning0
Left/Right WW pushes 
\f3 \uc0\u8594 
\f0  Arrow keys (
\f3 \uc0\u8592 
\f0  / 
\f3 \uc0\u8594 
\f0 )\
\ls5\ilvl1\kerning1\expnd0\expndtw0 {\listtext	\uc0\u8259 	}\expnd0\expndtw0\kerning0
Up/Down WW rotations 
\f3 \uc0\u8594 
\f0  Arrow keys (
\f3 \uc0\u8593 
\f0  / 
\f3 \uc0\u8595 
\f0 )\
\ls5\ilvl1\kerning1\expnd0\expndtw0 {\listtext	\uc0\u8259 	}\expnd0\expndtw0\kerning0
Long right WW push 
\f3 \uc0\u8594 
\f0  ENTER\
\ls5\ilvl1\kerning1\expnd0\expndtw0 {\listtext	\uc0\u8259 	}Long left WW push 
\f3 \expnd0\expndtw0\kerning0
\uc0\u8594 
\f0  ESCAPE\
\ls5\ilvl1\kerning1\expnd0\expndtw0 {\listtext	\uc0\u8259 	}\expnd0\expndtw0\kerning0
(A low-press of the Menu button reverts to TFT mode.)\
\pard\pardeftab720\partightenfactor0
\cf2 \
\pard\pardeftab720\partightenfactor0

\f1\b\fs30 \cf2 Compatibility\
\pard\pardeftab720\partightenfactor0

\f0\b0\fs28 \cf2 \
Works with any app supporting Bluetooth keyboard inputs (arrows + ENTER).\
\
\pard\pardeftab720\partightenfactor0

\f1\b\fs30 \cf2 Credit
\f0\b0 \
\pard\pardeftab720\partightenfactor0

\fs28 \cf2 \
\pard\tx220\tx720\pardeftab720\li720\fi-720\partightenfactor0
\ls6\ilvl0\cf2 The LIN slave implementation is based on {\field{\*\fldinst{HYPERLINK "https://github.com/gicking/LIN_slave_portable_Arduino"}}{\fldrslt Georg Icking-Konert\'92s work}}. I am grateful to him for his interest to this project and for his guidance in getting the LIN slave to work on the Arduino Nano ESP32. \
\pard\tx566\pardeftab720\partightenfactor0
\cf2 \
\pard\pardeftab720\partightenfactor0

\f1\b\fs30 \cf2 Installation Instructions\
\pard\pardeftab720\partightenfactor0

\f0\b0 \cf2 (To be added)
\f1\b \
\pard\pardeftab720\partightenfactor0

\f0\b0\fs28 \cf2 \
\pard\pardeftab720\partightenfactor0

\f1\b\fs30 \cf2 Demo Video\
\pard\pardeftab720\partightenfactor0

\f0\b0\fs28 \cf2 (To be added)\
\
\pard\pardeftab720\partightenfactor0

\f1\b\fs30 \cf2 Feedback\
\pard\pardeftab720\partightenfactor0

\f0\b0\fs28 \cf2 Send feedback at WWtoBluetooth@icloud.com\
\pard\pardeftab720\partightenfactor0
\cf3 \
\pard\pardeftab720\partightenfactor0
\cf2 \
\
}