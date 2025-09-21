/**
  \file     LIN_slave_HardwareSerial_ESP32.h
  \brief    LIN slave emulation library using a HardwareSerial interface of ESP32
  \details  This library provides a slave node emulation for a LIN bus via a HardwareSerial interface of ESP32.
            For an explanation of the LIN bus and protocol e.g. see https://en.wikipedia.org/wiki/Local_Interconnect_Network
  \note     Use ESP32 HardwareSerial for BREAK detection using framing error
  \author   Georg Icking-Konert
*/

// assert ESP32 platform
#if defined(ARDUINO_ARCH_ESP32)

/*-----------------------------------------------------------------------------
  MODULE DEFINITION FOR MULTIPLE INCLUSION
-----------------------------------------------------------------------------*/
#ifndef _LIN_SLAVE_HW_SERIAL_ESP32_H_
#define _LIN_SLAVE_HW_SERIAL_ESP32_H_


/*-----------------------------------------------------------------------------
  INCLUDE FILES
-----------------------------------------------------------------------------*/

// include required libraries
#include <LIN_slave_Base.h>


/*-----------------------------------------------------------------------------
  GLOBAL MACROS
-----------------------------------------------------------------------------*/

/// Set board number of ESP32 Serial interfaces (0..2)
#if !defined(LIN_SLAVE_ESP32_MAX_SERIAL)
  #define LIN_SLAVE_ESP32_MAX_SERIAL   3 
#endif

/// Override for boards with fewer Serial interfaces
#ifdef ARDUINO_ESP32S2
  #undef LIN_SLAVE_ESP32_MAX_SERIAL
  #define LIN_SLAVE_ESP32_MAX_SERIAL   2 
#endif


/*-----------------------------------------------------------------------------
  GLOBAL CLASS
-----------------------------------------------------------------------------*/

/**
  \brief  LIN slave node class via ESP32 HardwareSerial

  \details LIN slave node class via ESP32 HardwareSerial.
*/
class LIN_Slave_HardwareSerial_ESP32 : public LIN_Slave_Base
{
  // PRIVATE VARIABLES
  public:

    HardwareSerial        *pSerial;                              //!< pointer to serial interface used for LIN
    uint8_t               pinRx;                                 //!< pin used for receive
    uint8_t               pinTx;                                 //!< pin used for transmit
    uint8_t               idxSerial;                             //!< index to flagBreak[] of this instance
    static bool           flagBreak[LIN_SLAVE_ESP32_MAX_SERIAL]; //!< break flags for Serial0..N


  // PRIVATE METHODS
  private:
  
    #if (LIN_SLAVE_ESP32_MAX_SERIAL >= 1)
      /// @brief Static callback function for ESP32 Serial0 error
      static void _onSerialReceiveError0(hardwareSerial_error_t Err);
	  #else
      #error no HardwareSerial available for this board
    #endif
    #if (LIN_SLAVE_ESP32_MAX_SERIAL >= 2)
      /// @brief Static callback function for ESP32 Serial1 error
      static void _onSerialReceiveError1(hardwareSerial_error_t Err);
    #endif
    #if (LIN_SLAVE_ESP32_MAX_SERIAL >= 3)
      /// @brief Static callback function for ESP32 Serial2 error
      static void _onSerialReceiveError2(hardwareSerial_error_t Err);
    #endif
  

  // PROTECTED METHODS
  protected:

    /// @brief Get break detection flag
    bool _getBreakFlag(void);

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
    LIN_Slave_HardwareSerial_ESP32(HardwareSerial &Interface, uint8_t PinRx, uint8_t PinTx, 
      LIN_Slave_Base::version_t Version = LIN_Slave_Base::LIN_V2, const char NameLIN[] = "Slave", uint32_t TimeoutRx = 1500L, const int8_t PinTxEN = INT8_MIN);
     
    /// @brief Open serial interface
    void begin(uint16_t Baudrate = 19200);
    
    /// @brief Close serial interface
    void end(void);

    /// @brief check if a byte is available in Rx buffer
    inline bool available(void) { return this->pSerial->available(); }

}; // class LIN_Slave_HardwareSerial_ESP32


/*-----------------------------------------------------------------------------
    END OF MODULE DEFINITION FOR MULTIPLE INLUSION
-----------------------------------------------------------------------------*/
#endif // _LIN_SLAVE_HW_SERIAL_ESP32_H_

#endif // ARDUINO_ARCH_ESP32

/*-----------------------------------------------------------------------------
    END OF FILE
-----------------------------------------------------------------------------*/
