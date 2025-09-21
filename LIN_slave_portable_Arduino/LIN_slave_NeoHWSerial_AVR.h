/**
  \file     LIN_slave_NeoHWSerial_AVR.h
  \brief    LIN slave emulation library using a NeoHWSerial interface of AVR
  \details  This library provides a slave node emulation for a LIN bus via a NeoHWSerial interface of AVR
            For an explanation of the LIN bus and protocol e.g. see https://en.wikipedia.org/wiki/Local_Interconnect_Network
  \note     Use NeoHWSerial for BREAK detection using framing error (standard HardwareSerial only uses inter-frame pause)
  \author   Georg Icking-Konert
*/

// comment out to use HardwareSerial (sync on inter-frame pause) instead of NeoHWSerial (sync on BREAK)
#define USE_NEOSERIAL

// for AVR platform use NeoHWSerial or comment out USE_NEOSERIAL above
#if defined(ARDUINO_ARCH_AVR) && defined (USE_NEOSERIAL) && !defined(ARDUINO_AVR_TRINKET3) && !defined(ARDUINO_AVR_TRINKET5)


/*-----------------------------------------------------------------------------
  MODULE DEFINITION FOR MULTIPLE INCLUSION
-----------------------------------------------------------------------------*/
#ifndef _LIN_SLAVE_NEOHWSERIAL_AVR_H_
#define _LIN_SLAVE_NEOHWSERIAL_AVR_H_


/*-----------------------------------------------------------------------------
  INCLUDE FILES
-----------------------------------------------------------------------------*/

// include required libraries
#include <NeoHWSerial.h>
#include <LIN_slave_Base.h>


/*-----------------------------------------------------------------------------
  GLOBAL CLASS
-----------------------------------------------------------------------------*/

/**
  \brief  LIN slave node class via AVR NeoHWSerial

  \details LIN slave node class via AVR NeoHWSerial.
*/
class LIN_Slave_NeoHWSerial_AVR : public LIN_Slave_Base
{
  // PRIVATE VARIABLES
  private:

    NeoHWSerial           *pSerial;                             //!< pointer to serial interface used for LIN
    uint8_t               idxSerial;                            //!< index to flagBreak[] of this instance
    #if defined(HAVE_HWSERIAL3)
      static bool           flagBreak[4];                       //!< break flags for Serial0..3
    #elif defined(HAVE_HWSERIAL2)
      static bool           flagBreak[3];                       //!< break flags for Serial0..2
    #elif defined(HAVE_HWSERIAL1)
      static bool           flagBreak[2];                       //!< break flags for Serial0..1
    #elif defined(HAVE_HWSERIAL0)
      static bool           flagBreak[1];                       //!< break flags for Serial0
    #else
      #error no HardwareSerial available for this board
    #endif


  // PRIVATE METHODS
  private:

    #if defined(HAVE_HWSERIAL0)
      /// @brief Static callback function for AVR Serial0 receive ISR
      static bool _onSerialReceive0(uint8_t byte, uint8_t status);
    #endif
  
    #if defined(HAVE_HWSERIAL1)
      /// @brief Static callback function for AVR Serial1 receive ISR
      static bool _onSerialReceive1(uint8_t byte, uint8_t status);
    #endif
  
    #if defined(HAVE_HWSERIAL2)
      /// @brief Static callback function for AVR Serial2 receive ISR
      static bool _onSerialReceive2(uint8_t byte, uint8_t status);
    #endif
  
    #if defined(HAVE_HWSERIAL3)
      /// @brief Static callback function for AVR Serial3 receive ISR
      static bool _onSerialReceive3(uint8_t byte, uint8_t status);
    #endif
  

  // PROTECTED METHODS
  protected:

    /// @brief Get break detection flag
    bool _getBreakFlag(void);

    /// @brief Clear break detection flag
    void _resetBreakFlag(void);


    /// @brief peek next byte from Rx buffer
    inline uint8_t _serialPeek(void) { return pSerial->peek(); }

    /// @brief read next byte from Rx buffer
    inline uint8_t _serialRead(void) { return pSerial->read(); }

    /// @brief write bytes to Tx buffer
    inline void _serialWrite(uint8_t buf[], uint8_t num) { pSerial->write(buf, num); }


  // PUBLIC METHODS
  public:

    /// @brief Class constructor
    LIN_Slave_NeoHWSerial_AVR(NeoHWSerial &Interface, 
      LIN_Slave_Base::version_t Version = LIN_Slave_Base::LIN_V2, const char NameLIN[] = "Slave", uint32_t TimeoutRx = 1500L, const int8_t PinTxEN = INT8_MIN);
     
    /// @brief Open serial interface
    void begin(uint16_t Baudrate = 19200);
    
    /// @brief Close serial interface
    void end(void);

    /// @brief check if a byte is available in Rx buffer
    inline bool available(void) { return pSerial->available(); }

}; // class LIN_Slave_NeoHWSerial_AVR


/*-----------------------------------------------------------------------------
    END OF MODULE DEFINITION FOR MULTIPLE INLUSION
-----------------------------------------------------------------------------*/
#endif // _LIN_SLAVE_NEOHWSERIAL_AVR_H_

#endif // ARDUINO_ARCH_AVR && USE_NEOSERIAL


/*-----------------------------------------------------------------------------
    END OF FILE
-----------------------------------------------------------------------------*/
