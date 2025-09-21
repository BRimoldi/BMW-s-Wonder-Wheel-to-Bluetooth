/**
  \file     LIN_slave_HardwareSerial.h
  \brief    LIN slave emulation library using a generic HardwareSerial interface.
  \details  This library provides a slave node emulation for a LIN bus via a generic HardwareSerial interface.
            Due to "approximate" BREAK detection is less robust than specific board implementations! 
            For an explanation of the LIN bus and protocol e.g. see https://en.wikipedia.org/wiki/Local_Interconnect_Network
  \author   Georg Icking-Konert
*/

// for AVR selection of HardwareSerial (sync on inter-frame pause) or NeoHWSerial (sync on BREAK)
#include <LIN_slave_NeoHWSerial_AVR.h>

// for AVR platform use NeoHWSerial or comment out USE_NEOSERIAL in file LIN_slave_NeoHWSerial_AVR.h
#if !defined(ARDUINO_ARCH_AVR) || !defined(USE_NEOSERIAL)


/*-----------------------------------------------------------------------------
  MODULE DEFINITION FOR MULTIPLE INCLUSION
-----------------------------------------------------------------------------*/
#ifndef _LIN_SLAVE_HW_SERIAL_H_
#define _LIN_SLAVE_HW_SERIAL_H_


/*-----------------------------------------------------------------------------
  INCLUDE FILES
-----------------------------------------------------------------------------*/

// include required libraries
#include <LIN_slave_Base.h>


/*-----------------------------------------------------------------------------
  GLOBAL CLASS
-----------------------------------------------------------------------------*/

/**
  \brief  LIN slave node class via generic HardwareSerial

  \details LIN slave node class via generic HardwareSerial.
*/
class LIN_Slave_HardwareSerial : public LIN_Slave_Base
{
  // PROTECTED VARIABLES
  protected:

    HardwareSerial        *pSerial;             //!< pointer to serial interface used for LIN
    bool                  flagBreak;            //!< a break was detected, is set in handle
    uint16_t              minFramePause;        //!< min. inter-frame pause [us] to start new frame (not standard compliant!)


  // PROTECTED METHODS
  protected:

    /// @brief Get break detection flag
    virtual bool _getBreakFlag(void);

    /// @brief Clear break detection flag
    void _resetBreakFlag(void);


    /// @brief peek next byte from Rx buffer
    inline uint8_t _serialPeek(void) { return this->pSerial->peek(); }

    /// @brief read next byte from Rx buffer
    inline uint8_t _serialRead(void) { return this->pSerial->read(); }

    /// @brief write bytes to Tx buffer
    inline void _serialWrite(uint8_t buf[], uint8_t num) { this->pSerial->write(buf, num); }


  // PUBLIC METHODS
  public:

    /// @brief Class constructor
    LIN_Slave_HardwareSerial(HardwareSerial &Interface, uint16_t MinFramePause=1000L, 
      LIN_Slave_Base::version_t Version = LIN_Slave_Base::LIN_V2, const char NameLIN[] = "Slave", uint32_t TimeoutRx = 1500L, const int8_t PinTxEN = INT8_MIN);
     
    /// @brief Open serial interface
    void begin(uint16_t Baudrate = 19200);
    
    /// @brief Close serial interface
    void end(void);

    /// @brief check if a byte is available in Rx buffer
    inline bool available(void) { return this->pSerial->available(); }

    /// @brief Handle LIN protocol and call user-defined frame handlers
    virtual void handler(void);

}; // class LIN_Slave_HardwareSerial


/*-----------------------------------------------------------------------------
    END OF MODULE DEFINITION FOR MULTIPLE INLUSION
-----------------------------------------------------------------------------*/
#endif // _LIN_SLAVE_HW_SERIAL_H_

#endif // !ARDUINO_ARCH_AVR || !USE_NEOSERIAL

/*-----------------------------------------------------------------------------
    END OF FILE
-----------------------------------------------------------------------------*/
