/*********************

Description: This sketch implements the interface between the BMW Wonder Wheel (WW) and a cell phone (tested on iPhone). 
  The purpose is to be able to use the WW to zoom in and out of a navigation app (tested on Scenic).
  The sketch implements a LIN bus slave to interact with the motorcycle, and a Bluetooth keyboard to interact with the phone. 
  The action of turning the WW up/down by one notch is seen by the phone as pressing the arrow up/down on a bluetooth keyboard. 
  The action of pushing the  WW right/left is seen by the phone as pressing the arrow right/left.
  A long right push is seen as a RETURN and a long left push as an ESC.
  The code has been tested on an Arduino Nano ESP32 board (complemented with a TJA1021 Transceiver). 

Author: Bixio Rimoldi
Last revision: Seot 4, 2025

Thanks: The LIN slave part is borrowed from https://github.com/gicking/LIN_slave_portable_Arduino

**********************/

// include files
#include <LIN_slave_HardwareSerial_ESP32.h>
#include <BleKeyboard.h>

// board pin definitions (Dn is referred to as n on Nano ESP32) 
#define PIN_ERROR     14       // D14, red LED; indicates LIN errors
#define PIN_BLUE      16       // D16, blue LED; indicates bluetooth connection
#define PIN_LIN_RX    4        // D4, receive pin for LIN
#define PIN_LIN_TX    6        // D6, transmit pin for LIN


// global variables
bool    ID_14h_VISIBLE    = false;  // false until the LIN frame ID 14h (WW frame) has been received once
bool    WheelUp           = false;  // WW event: Up
bool    WheelDown         = false;  // WW event: Down
bool    WheelRight        = false;  // WW event: Right
bool    WheelLeft         = false;  // WW event: Left
bool    WheelLongRight    = false;  // WW event: Long Right
bool    WheelLongLeft     = false;  // WW event: Long Left
bool    MenuDown          = false;  // Menu event: Down

uint8_t OldWheelPosition;
uint8_t NewWheelPosition;
int     Increment         = 0;      // to track WW up/down rotations 
uint8_t RightCount        = 0;      // number of consecutives frames during a right push (to detect a long right push)
uint8_t LeftCount         = 0;      // number of consecutives frames during a left push (to detect a long left push)


// setup LIN node
LIN_Slave_HardwareSerial_ESP32  LIN(Serial2, PIN_LIN_RX, PIN_LIN_TX, LIN_Slave_Base::LIN_V2, "Slave");

// instantiate BleKeyboard
BleKeyboard bleKeyboard("BMW's WW");

// call once
void setup()
{
  // initiate LIN status LED (red)
  pinMode(PIN_ERROR, OUTPUT);
  digitalWrite(PIN_ERROR, HIGH); // HIGH means off for this LED

  // initiate Bluetooth status LED (blue) 
  pinMode(PIN_BLUE, OUTPUT);
  digitalWrite(PIN_BLUE, HIGH); // HIGH means off for this LED
   
  // initiate the LED (orange) indicating that frame 14h (WW frame) is received.
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // LOW means off for this LED

  // open LIN interface
  LIN.begin(19200);

  // Register callback function for frame 14h, which carries WW movements
  LIN.registerMasterRequestHandler(0x14, handle_Request14, 8);

  // Register callback function for frame 2Bh. 
  // Note1: Without this response, the bike assumes that there is no navigator, ...
  // ... the WW inputs always go to the TFT display, and neither TFT nor NAV are shown on the display. 
  LIN.registerSlaveResponseHandler(0x2B, handle_Response2B, 8); // can be used to reset, e.g., the daily Km count
  
  // Register callback function for frame 28h. 
  // Note2: same as Note 1.  
  LIN.registerSlaveResponseHandler(0x28, handle_Response28, 8); // can be used to override date and time
  
  bleKeyboard.begin(); 
} // setup()

void loop()
{ 
  if (bleKeyboard.isConnected()) {
    digitalWrite(PIN_BLUE, LOW); // LOW means on for this LED 
  }
  dealWithLIN();
  dealWithBluetooth(); 
} // loop()

// ++++++++++++++++++++++++++++++++++++++++++++++++++
void dealWithLIN(){
  if (LIN.available())
  {
    LIN.handler();
    // if LIN frame has finished, acto on it
    if (LIN.getState() == LIN_Slave_Base::STATE_DONE)
    {
      LIN_Slave_Base::frame_t   Type;
      LIN_Slave_Base::error_t   error;
      uint8_t                   Id;
      uint8_t                   NumData;
      uint8_t                   Data[8];

      // get frame data & error status
      LIN.getFrame(Type, Id, NumData, Data);
      error = LIN.getError();

      // indicate status via pin
      digitalWrite(PIN_ERROR, !error); // verify that LED os off when error is false

      // reset state machine & error
      LIN.resetStateMachine();
      LIN.resetError();
    } // LIN frame finished
  } // if LIN available
} // dealWithLin()

// ++++++++++++++++++++++++++++++++++++++++++++++++++
void dealWithBluetooth(){
  // Bixio: In a next iteration, I may make Wheel a set variable and use a case statement
  if (WheelUp)
  {
    bleKeyboard.write(KEY_UP_ARROW);
      WheelUp = false;
  } 
  else if (WheelDown)
  {
    bleKeyboard.write(KEY_DOWN_ARROW);
    WheelDown = false;
  }
  else if (WheelRight)
  {
    bleKeyboard.write(KEY_RIGHT_ARROW);
    WheelRight = false;
  }
  else if (WheelLeft)
  {
    bleKeyboard.write(KEY_LEFT_ARROW);
    WheelLeft = false;
  }
  else if (WheelLongLeft)
  {
    bleKeyboard.write(KEY_ESC);
    WheelLongLeft = false;
  }
  else if (WheelLongRight)
  {
    bleKeyboard.write(KEY_RETURN);
    WheelLongRight = false;
  }     
  else if (MenuDown)
  { 
    ID_14h_VISIBLE = false; 
    MenuDown = false;
  } 
} // dealWithBluetooth()


// ++++++++++++++++++++++++++++++++++++++++++++++++++
// Master Request handler for ID 14h (frame 14h carries the WW inputs)
// From this frame we are just reaing out the infos
void handle_Request14(uint8_t NumData, uint8_t* Data)
{
  // avoid unused parameter warning
  (void) NumData;
  (void) Data;

  digitalWrite(LED_BUILTIN, HIGH); // indicates that frame 14h is received
  // Note3: frame 14h is not forwarded to the LIN bus until we do a long high push in the Menu button (handlebar)
  // Note4: a low push on the Menu button stops the forwarding (after 4 more frames)

  // Check wheel turned
  if (!ID_14h_VISIBLE) 
  // The first passage is special, we ust get the reference
  {
    OldWheelPosition = Data[5]; 
    ID_14h_VISIBLE = true;
  } 
  else  
    // ID 14h has been seen before, hence the OldWheelPosition has the correct value
  {
    // check if wheel has been turned
    // Note5: the wheel returns an integer modulo 255 (not modulo 256) ...
    // it increases from 0 to 254 as we rotate up (forward), and decreases as we rotate down (backwards). 
    // Note6: between consecutive frames (50 milliseconds), the wheel can only increment or decrement by 1 position 
    NewWheelPosition = Data[5];
    Increment = NewWheelPosition - OldWheelPosition; // takes values in {+/- 1, +/-254}
    Increment = (Increment + 255) % 255; // it evaluates to 1 for wheel up by one position and to 254 for wheel down by one position
    switch (Increment) {
      case 1:
        WheelUp = true;
        break;
      case 254:
        WheelDown = true;
        break;
      case 0:
        // do nothing
        break;
      default: 
        // keep this for debugging purposes 
        break; 
    }

    OldWheelPosition = NewWheelPosition;
  } // check wheel turned

  // chek wheel pushed right 
  // Note7: a short right push lasts long enough to be carried by, say 5 frames
  if (Data[3] == 0x02) // indicating right push
  {
    RightCount = RightCount+1;
  }
  else if ((Data[3] == 0x00) && (RightCount > 0)) // we just let go the right push
  {
    if (RightCount <= 10) // <= 500 milliseconds, the right push was a short one
    {
      WheelRight = true;
      RightCount = 0;
    }
    else // the right push was a long one
    {
      WheelLongRight = true;
      RightCount = 0;
    }
  } //check wheel pushed right

  // check wheel pushed left
  // if needed, see the comments regarding the right push
  if (Data[3] == 0x01)
  {
    LeftCount = LeftCount+1;
  }
  else if ((Data[3] == 0x00) && (LeftCount > 0))
  {
    if (LeftCount <= 10)
    {
      WheelLeft = true;
      LeftCount = 0;
    }
    else
    {
      WheelLongLeft = true;
      LeftCount = 0;
    }
  }

  // check Menu button pushed down
  if (Data[2] == 0x10)
  {
    MenuDown = true;
  }
  digitalWrite(LED_BUILTIN, LOW);
} // master request handler ID 14h

// ++++++++++++++++++++++++++++++++++++++++++++++++++
// Slave Response handler for ID 2Bh
// Note8: A response (not any) is needed to trick the bike into thinking that there is a navigator ...
// the response that we are giving is not changing any of the bike settings ...
// but we could respond in such a way as to reset, say, the daily Km count
void handle_Response2B(uint8_t NumData, uint8_t* Data)
{
  // avoid unused parameter warning
  (void) NumData;
  (void) Data;

  Data[0] = 0x01; // 0x01, 0x02, 0x04, 0x08 == do not reset, reset km 1, reset km 2, reset both
  Data[1] = 0x15; // don't reset (average fuel consumption, total fuel consumption, average speed)
  Data[2] = 0x15; // don't reset (various energy consumptions)
  Data[3] =0; 
  Data[4] =0; 
  Data[5] =0; 
  Data[6] =0; 
  Data[7] =0; 
} // handle_Response()


// ++++++++++++++++++++++++++++++++++++++++++++++++++
// Slave Response handler ID 28h
void handle_Response28(uint8_t NumData, uint8_t* Data) 
// Note9: a response is needed (see note8). 
// with this response we have the option to set the date and time on the bike's computer ...
// ... but we are not doing so. 
{
  // avoid unused parameter warning
  (void) NumData;
  (void) Data;

  Data[0] = 0xFF; // seconds; FF means invalid signal (any other value will force that value)
  Data[1] = 0xFF; // minutes; FF means invalid signal (any other value will force that value)
  Data[2] = 0xFF; // hours; FF means invalid signal (any other value will force that value)
  Data[3] =0xFF; // day; FF means invalid signal (any other value will force that value)
  Data[4] =0xFF; // year and month; ; FF means invalid signal (any other value will force that value)
  Data[5] =0xFF; // year (12 bits total); ; FF means invalid signal (any other value will force that value)
  Data[6] =0x00; // I am not sure if this value matters
  Data[7] =0x00; // 0x80 means error; 0x00 means no error; This signal must be present for LIN 2.0
  
} // handle_Response()