/**
  \file     LIN_slave_HardwareSerial_ESP32.cpp
  \brief    LIN slave emulation library using a HardwareSerial interface of ESP32
  \details  This library provides a slave node emulation for a LIN bus via a HardwareSerial interface of ESP32.
            For an explanation of the LIN bus and protocol e.g. see https://en.wikipedia.org/wiki/Local_Interconnect_Network
  \note     Use ESP32 HardwareSerial for BREAK detection using framing error
  \author   Georg Icking-Konert
*/

// assert ESP32 platform
#if defined(ARDUINO_ARCH_ESP32)

// include files
#include <LIN_slave_HardwareSerial_ESP32.h>


// definition of static class variables (see https://stackoverflow.com/a/51091696)
bool LIN_Slave_HardwareSerial_ESP32::flagBreak[LIN_SLAVE_ESP32_MAX_SERIAL];



/**************************
 * PRIVATE METHODS
**************************/

#if (LIN_SLAVE_ESP32_MAX_SERIAL >= 1)

  /**
    \brief      Static callback function for ESP32 Serial0 error
    \details    Static callback function for ESP32 Serial0 error. For a BRK this is called after onReceiveFunction() -> use polling in loop()
                Note: received BREAK byte is consumed here to support also sync on SYNC byte. 
    \param[in]  Err   type of UART error
  */
  void LIN_Slave_HardwareSerial_ESP32::_onSerialReceiveError0(hardwareSerial_error_t Err)
  {
    // on BREAK (=0x00 with framing error) set class variable and remove 0x00 from queue
    if ((Serial.peek() == 0x00) && (Err == UART_BREAK_ERROR))
    {
      // set BREAK flag for Serial0
      (LIN_Slave_HardwareSerial_ESP32::flagBreak)[0] = true;

      // remove 0x00 from queue
      uint8_t byte = Serial.read();

      // avoid unused variable warning in case of no debug output
      (void) byte; 

      // print debug message
      DEBUG_PRINT_STATIC(3, "Rx=0x%02X, BRK", byte);

    } // if BREAK detected

  } // LIN_Slave_HardwareSerial_ESP32::_onSerialReceiveError0()

#endif // LIN_SLAVE_ESP32_MAX_SERIAL >= 1



#if (LIN_SLAVE_ESP32_MAX_SERIAL >= 2)

  /**
    \brief      Static callback function for ESP32 Serial1 error
    \details    Static callback function for ESP32 Serial1 error. For a BRK this is called after onReceiveFunction() -> use polling in loop()
                Note: received BREAK byte is consumed here to support also sync on SYNC byte. 
    \param[in]  Err   type of UART error
  */
  void LIN_Slave_HardwareSerial_ESP32::_onSerialReceiveError1(hardwareSerial_error_t Err)
  {
    // on BREAK (=0x00 with framing error) set class variable and remove 0x00 from queue
    if ((Serial1.peek() == 0x00) && (Err == UART_BREAK_ERROR))
    {
      // set BREAK flag for Serial1
      (LIN_Slave_HardwareSerial_ESP32::flagBreak)[1] = true;

      // remove 0x00 from queue
      uint8_t byte = Serial1.read();

      // avoid unused variable warning in case of no debug output
      (void) byte; 

      // print debug message
      DEBUG_PRINT_STATIC(3, "Rx=0x%02X, BRK", byte);

    } // if BREAK detected

  } // LIN_Slave_HardwareSerial_ESP32::_onSerialReceiveError1()

#endif // LIN_SLAVE_ESP32_MAX_SERIAL >= 2 


#if (LIN_SLAVE_ESP32_MAX_SERIAL >= 3)

  /**
    \brief      Static callback function for ESP32 Serial2 error
    \details    Static callback function for ESP32 Serial2 error. For a BRK this is called after onReceiveFunction() -> use polling in loop()
                Note: received BREAK byte is consumed here to support also sync on SYNC byte. 
    \param[in]  Err   type of UART error
  */
  void LIN_Slave_HardwareSerial_ESP32::_onSerialReceiveError2(hardwareSerial_error_t Err)
  {
    // on BREAK (=0x00 with framing error) set class variable and remove 0x00 from queue
    if ((Serial2.peek() == 0x00) && (Err == UART_BREAK_ERROR))
    {
      // set BREAK flag for Serial2
      (LIN_Slave_HardwareSerial_ESP32::flagBreak)[2] = true;

      // remove 0x00 from queue
      uint8_t byte = Serial2.read();

      // avoid unused variable warning in case of no debug output
      (void) byte; 

      // print debug message
      DEBUG_PRINT_STATIC(3, "Rx=0x%02X, BRK", byte);

    } // if BREAK detected

  } // LIN_Slave_HardwareSerial_ESP32::_onSerialReceiveError2()

#endif // LIN_SLAVE_ESP32_MAX_SERIAL >= 3



/**************************
 * PROTECTED METHODS
**************************/

/**
  \brief      Get break detection flag
  \details    Get break detection flag. Is hardware dependent
  \return status of break detection
*/
bool LIN_Slave_HardwareSerial_ESP32::_getBreakFlag()
{
  // return BREAK detection flag of respective Serialx
  return (this->flagBreak)[this->idxSerial];

} // LIN_Slave_HardwareSerial_ESP32::_getBreakFlag()



/**
  \brief      Clear break detection flag. Is hardware dependent
  \details    Clear break detection flag. Is hardware dependent
*/
void LIN_Slave_HardwareSerial_ESP32::_resetBreakFlag()
{
  // clear BREAK detection flag of respective Serialx
  (this->flagBreak)[this->idxSerial] = false;

} // LIN_Slave_HardwareSerial_ESP32::_resetBreakFlag()



/**************************
 * PUBLIC METHODS
**************************/

/**
  \brief      Constructor for LIN node class using ESP32 HardwareSerial
  \details    Constructor for LIN node class for using ESP32 HardwareSerial. Inherit all methods from LIN_Slave_Base, only different constructor
  \param[in]  Interface   serial interface for LIN
  \param[in]  PinRx       GPIO used for reception
  \param[in]  PinTx       GPIO used for transmission
  \param[in]  Version     LIN protocol version (default = v2)
  \param[in]  NameLIN     LIN node name (default = "Slave")
  \param[in]  PinTxEN     optional Tx enable pin (high active) e.g. for LIN via RS485 (default = -127/none)
  \param[in]  TimeoutRx   timeout [us] for bytes in frame (default = 1500)
*/
LIN_Slave_HardwareSerial_ESP32::LIN_Slave_HardwareSerial_ESP32(HardwareSerial &Interface, uint8_t PinRx, uint8_t PinTx,
  LIN_Slave_Base::version_t Version, const char NameLIN[], uint32_t TimeoutRx, const int8_t PinTxEN) : 
  LIN_Slave_Base::LIN_Slave_Base(Version, NameLIN, TimeoutRx, PinTxEN)
{
  // Debug serial initialized in begin() -> no debug output here

  // store parameters in class variables
  this->pSerial    = &Interface;          // pointer to used HW serial
  this->pinRx      = PinRx;               // receive pin
  this->pinTx      = PinTx;               // transmit pin

} // LIN_Slave_HardwareSerial_ESP32::LIN_Slave_HardwareSerial_ESP32()



/**
  \brief      Open serial interface
  \details    Open serial interface with specified baudrate
  \param[in]  Baudrate    communication speed [Baud] (default = 19200)
*/
void LIN_Slave_HardwareSerial_ESP32::begin(uint16_t Baudrate)
{
  // call base class method
  LIN_Slave_Base::begin(Baudrate);  

  // open serial interface incl. used pins with optional timeout
  this->pSerial->end();
  this->pSerial->begin(this->baudrate, SERIAL_8N1, this->pinRx, this->pinTx);
  #if defined(LIN_SLAVE_LIN_PORT_TIMEOUT) && (LIN_SLAVE_LIN_PORT_TIMEOUT > 0)
    uint32_t startMillis = millis();
    while ((!(*(this->pSerial))) && (millis() - startMillis < LIN_SLAVE_LIN_PORT_TIMEOUT));
  #else
    while(!(*(this->pSerial)));
  #endif

  // Attach corresponding error callback to Serialx receive handler
  #if (LIN_SLAVE_ESP32_MAX_SERIAL >= 1)
    if (this->pSerial == &Serial0)
    { 
      this->idxSerial = 0;
      this->pSerial->onReceiveError(this->_onSerialReceiveError0);
    }
  #endif
  #if (LIN_SLAVE_ESP32_MAX_SERIAL >= 2)
    if (this->pSerial == &Serial1)
    { 
      this->idxSerial = 1;
      this->pSerial->onReceiveError(this->_onSerialReceiveError1);
    }
  #endif
  #if (LIN_SLAVE_ESP32_MAX_SERIAL >= 3)
    if (this->pSerial == &Serial2)
    { 
      this->idxSerial = 2;
      this->pSerial->onReceiveError(this->_onSerialReceiveError2);
    }
  #endif

  // initialize variables
  this->_resetBreakFlag();

  // print debug message
  DEBUG_PRINT(2, "ok");
  
} // LIN_Slave_HardwareSerial_ESP32::begin()



/**
  \brief      Close serial interface
  \details    Close serial interface. Here dummy!
*/
void LIN_Slave_HardwareSerial_ESP32::end()
{
  // call base class method
  LIN_Slave_Base::end();
    
  // close serial interface
  this->pSerial->end();

  // print debug message
  DEBUG_PRINT(2, " ");

} // LIN_Slave_HardwareSerial_ESP32::end()

#endif // ARDUINO_ARCH_ESP32

/*-----------------------------------------------------------------------------
    END OF FILE
-----------------------------------------------------------------------------*/
