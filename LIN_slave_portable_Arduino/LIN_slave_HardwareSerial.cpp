/**
  \file     LIN_slave_HardwareSerial.cpp
  \brief    LIN slave emulation library using a generic HardwareSerial interface.
  \details  This library provides a slave node emulation for a LIN bus via a generic HardwareSerial interface.
            Due to "approximate" BREAK detection is less robust than specific board implementations! 
            For an explanation of the LIN bus and protocol e.g. see https://en.wikipedia.org/wiki/Local_Interconnect_Network
  \author   Georg Icking-Konert
*/

// include files
#include <LIN_slave_HardwareSerial.h>

// optional on AVR, see LIN_slave_NeoHWSerial_AVR.h
#if defined(_LIN_SLAVE_HW_SERIAL_H_)


/**************************
 * PROTECTED METHODS
**************************/

/**
  \brief      Get break detection flag
  \details    Get break detection flag. Is hardware dependent
  \return status of break detection
*/
bool LIN_Slave_HardwareSerial::_getBreakFlag()
{
  // return BREAK detection flag
  return this->flagBreak;

} // LIN_Slave_HardwareSerial::_getBreakFlag()



/**
  \brief      Clear break detection flag
  \details    Clear break detection flag
*/
void LIN_Slave_HardwareSerial::_resetBreakFlag()
{
  // clear BREAK detection flag
  this->flagBreak = false;

} // LIN_Slave_HardwareSerial::_resetBreakFlag()



/**************************
 * PUBLIC METHODS
**************************/

/**
  \brief      Constructor for LIN node class using generic HardwareSerial
  \details    Constructor for LIN node class for using generic HardwareSerial. Inherit all methods from LIN_Slave_Base, only different constructor
  \param[in]  Interface       serial interface for LIN
  \param[in]  MinFramePause   min. inter-frame pause [us] to detect new frame (default = 1000)
  \param[in]  Version         LIN protocol version (default = v2)
  \param[in]  NameLIN         LIN node name (default = "Slave")
  \param[in]  TimeoutRx       timeout [us] for bytes in frame (default = 1500)
  \param[in]  PinTxEN     optional Tx enable pin (high active) e.g. for LIN via RS485 (default = -127/none)
*/
LIN_Slave_HardwareSerial::LIN_Slave_HardwareSerial(HardwareSerial &Interface, uint16_t MinFramePause, 
  LIN_Slave_Base::version_t Version, const char NameLIN[], uint32_t TimeoutRx, const int8_t PinTxEN) : 
  LIN_Slave_Base::LIN_Slave_Base(Version, NameLIN, TimeoutRx, PinTxEN)
{  
  // Debug serial initialized in begin() -> no debug output here

  // store parameters in class variables
  this->pSerial       = &Interface;
  this->minFramePause = MinFramePause;
  
  // must not open connection here, else (at least) ESP32 and ESP8266 fail

} // LIN_Slave_HardwareSerial::LIN_Slave_HardwareSerial()



/**
  \brief      Open serial interface
  \details    Open serial interface with specified baudrate
  \param[in]  Baudrate    communication speed [Baud] (default = 19200)
*/
void LIN_Slave_HardwareSerial::begin(uint16_t Baudrate)
{
  // call base class method
  LIN_Slave_Base::begin(Baudrate);  

  // open serial interface with optional timeout
  this->pSerial->begin(this->baudrate);
  #if defined(LIN_SLAVE_LIN_PORT_TIMEOUT) && (LIN_SLAVE_LIN_PORT_TIMEOUT > 0)
    uint32_t startMillis = millis();
    while ((!(*(this->pSerial))) && (millis() - startMillis < LIN_SLAVE_LIN_PORT_TIMEOUT));
  #else
    while(!(*(this->pSerial)));
  #endif

  // initialize variables
  this->_resetBreakFlag();

  // print debug message
  DEBUG_PRINT(2, "ok");

} // LIN_Slave_HardwareSerial::begin()



/**
  \brief      Close serial interface
  \details    Close serial interface. Here dummy!
*/
void LIN_Slave_HardwareSerial::end()
{
  // call base class method
  LIN_Slave_Base::end();
    
  // close serial interface
  this->pSerial->end();

  // print debug message
  DEBUG_PRINT(2, " ");

} // LIN_Slave_HardwareSerial::end()



/**
  \brief      Handle LIN protocol and call user-defined frame handlers
  \details    Handle LIN protocol and call user-defined frame handlers, both for master request and slave response frames. 
              BREAK detection is based on inter-frame timing only (Arduino doesn't store framing error) -> less reliable.
              Note: received BREAK byte is consumed here to support also sync on SYNC byte if Rx byte w/o stop bit is ignored 
*/
void LIN_Slave_HardwareSerial::handler()
{
  // sync frames based on inter-frame pause (not standard compliant!) 
  static uint32_t   usLastByte = 0;
  
  // print debug message
  //DEBUG_PRINT(3, "state=%d", (int) this->state);

  // byte received -> check it
  if (this->pSerial->available())
  {
    // 0x00 received and long time since last byte (=BREAK) -> start new frame and remove 0x00 from queue
    if ((this->pSerial->peek() == 0x00) && ((micros() - usLastByte) > this->minFramePause))
    {
      this->flagBreak = true;
      this->pSerial->read();
    }

    // store time of this receive
    usLastByte = micros();

    // call base-class handler
    LIN_Slave_Base::handler();

  } // if byte received

} // LIN_Slave_HardwareSerial::handler()

#endif // !ARDUINO_ARCH_AVR

/*-----------------------------------------------------------------------------
    END OF FILE
-----------------------------------------------------------------------------*/
