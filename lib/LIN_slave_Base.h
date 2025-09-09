/**
  \file     LIN_slave_Base.h
  \brief    Base class for LIN slave emulation (non-functional)
  \details  This library provides the base class for a slave node emulation of a LIN bus. 
            The base class is non-functional, as it lacks the actual communication interface.
            For an explanation of the LIN bus and protocol e.g. see https://en.wikipedia.org/wiki/Local_Interconnect_Network
  \author   Georg Icking-Konert
*/

/*-----------------------------------------------------------------------------
  MODULE DEFINITION FOR MULTIPLE INCLUSION
-----------------------------------------------------------------------------*/
#ifndef _LIN_SLAVE_BASE_H_
#define _LIN_SLAVE_BASE_H_


/*-----------------------------------------------------------------------------
  GLOBAL DEFINES
-----------------------------------------------------------------------------*/

// misc parameters
#define LIN_SLAVE_BUFLEN_NAME           30            //!< max. length of node name
#define LIN_SLAVE_LIN_PORT_TIMEOUT      3000          //!< optional LIN.begin() timeout [ms] (<=0 -> no timeout). Is relevant for native USB ports, if USB is not connected 

// required for CI test environment. Call arduino-cli with "-DINCLUDE_NEOHWSERIAL"
#if defined(INCLUDE_NEOHWSERIAL)
  #include <NeoHWSerial.h>
#endif

// optional LIN debug output @ 115.2kBaud. Comment out for none. When using together with NeoHWSerial on AVR must use NeoSerialx to avoid linker conflict
#if !defined(LIN_SLAVE_DEBUG_SERIAL)
  //#define LIN_SLAVE_DEBUG_SERIAL      Serial          //!< serial interface used for debug output
  //#define LIN_SLAVE_DEBUG_SERIAL      NeoSerial       //!< serial interface used for debug output (optional on AVR, not together with HardwareSerial!)
  //#include <NeoHWSerial.h>                            // comment in/out together with previous line
  //#define LIN_SLAVE_DEBUG_SERIAL     SerialUSB        //!< serial interface used for debug output (optional on Due)
#endif
#if !defined(LIN_SLAVE_DEBUG_LEVEL)
  #define LIN_SLAVE_DEBUG_LEVEL         2             //!< debug verbosity 0..3 (1=errors only, 3=chatty)
#endif
#if !defined(LIN_SLAVE_DEBUG_PORT_TIMEOUT)
  #define LIN_SLAVE_DEBUG_PORT_TIMEOUT  3000          //!< optional LIN_SLAVE_DEBUG_SERIAL.begin() timeout [ms] (<=0 -> no timeout). Is relevant for native USB ports, if USB is not connected 
#endif
#if !defined(LIN_SLAVE_DEBUG_BUFSIZE)
  #define LIN_SLAVE_DEBUG_BUFSIZE      128            //!< optional buffer for debug messages 
#endif

// define logging macros for optional debug output.
// Use with printf() format like: DEBUG_PRINT(2, "Text=%s, Value=%d", text, value);
#if defined(LIN_SLAVE_DEBUG_SERIAL)
  
  // debug output macro for normal class methods
  #define DEBUG_PRINT(level, fmt, ...) \
    do { \
      if (LIN_SLAVE_DEBUG_LEVEL >= level) { \
        LIN_SLAVE_DEBUG_SERIAL.print(this->nameLIN); \
        LIN_SLAVE_DEBUG_SERIAL.print(F(": ")); \
        LIN_SLAVE_DEBUG_SERIAL.print(__PRETTY_FUNCTION__); \
        LIN_SLAVE_DEBUG_SERIAL.print(F(": ")); \
        char debug_buf[LIN_SLAVE_DEBUG_BUFSIZE]; \
        snprintf(debug_buf, sizeof(debug_buf), (fmt), ##__VA_ARGS__); \
        LIN_SLAVE_DEBUG_SERIAL.println(debug_buf); \
      } \
    } while(0)
  
  // debug output macro for static class methods and functions
  #define DEBUG_PRINT_STATIC(level, fmt, ...) \
    do { \
      if (LIN_SLAVE_DEBUG_LEVEL >= level) { \
        LIN_SLAVE_DEBUG_SERIAL.print(__PRETTY_FUNCTION__); \
        LIN_SLAVE_DEBUG_SERIAL.print(F(": ")); \
        char debug_buf[LIN_SLAVE_DEBUG_BUFSIZE]; \
        snprintf(debug_buf, sizeof(debug_buf), (fmt), ##__VA_ARGS__); \
        LIN_SLAVE_DEBUG_SERIAL.println(debug_buf); \
      } \
    } while(0)
  

// no debug output -> omit logging macro
#else

  // do nothing. Use safe empty macros
  #define DEBUG_PRINT(level, fmt, ...) do {} while (0)
  #define DEBUG_PRINT_STATIC(level, fmt, ...) do {} while (0)

#endif // LIN_SLAVE_DEBUG_SERIAL


/*-----------------------------------------------------------------------------
  INCLUDE FILES
-----------------------------------------------------------------------------*/

// generic Arduino functions
#include <Arduino.h>


/*-----------------------------------------------------------------------------
  GLOBAL CLASS
-----------------------------------------------------------------------------*/

/**
  \brief  LIN slave node base class

  \details LIN slave node base class. From this class the actual LIN classes for a Serialx are derived.
*/
class LIN_Slave_Base
{
  // PUBLIC TYPEDEFS
  public:

    /// LIN protocol version 
    typedef enum : uint8_t
    {
      LIN_V1                = 1,                //!< LIN protocol version 1.x
      LIN_V2                = 2                 //!< LIN protocol version 2.x
    } version_t;


    /// LIN frame type. Use high nibble for type, low nibble for number of data bytes -> minimize callback[] size
    typedef enum : uint8_t
    {
      MASTER_REQUEST        = 0x10,             //!< LIN master request frame
      SLAVE_RESPONSE        = 0x20              //!< LIN slave response frame
    } frame_t;


    /// LIN state machine states. Use bitmasks for fast checking multiple states
    typedef enum : uint8_t
    { 
      STATE_OFF             = 0x01,             //!< LIN interface closed
      STATE_WAIT_FOR_BREAK  = 0x02,             //!< no LIN transmission ongoing, wait for BRK
      STATE_WAIT_FOR_SYNC   = 0x04,             //!< BRK received, wait for SYNC
      STATE_WAIT_FOR_PID    = 0x08,             //!< SYNC received, wait for frame PID
      STATE_RECEIVING_DATA  = 0x10,             //!< receiving master request data
      STATE_RECEIVING_ECHO  = 0x20,             //!< receiving slave response echo
      STATE_WAIT_FOR_CHK    = 0x40,             //!< waiting for checksum
      STATE_DONE            = 0x80              //!< frame is completed
    } state_t;


    /// LIN error codes. Use bitmasks, as error is latched. Use same as LIN_master_portable
    typedef enum : uint8_t
    {
      NO_ERROR              = 0x00,             //!< no error
      ERROR_STATE           = 0x01,             //!< error in LIN state machine
      ERROR_ECHO            = 0x02,             //!< error reading response echo
      ERROR_TIMEOUT         = 0x04,             //!< frame timeout error
      ERROR_CHK             = 0x08,             //!< LIN checksum error
      ERROR_SYNC            = 0x10,             //!< error in SYNC (not 0x55) 
      ERROR_PID             = 0x20,             //!< ID parity error 
      ERROR_MISC            = 0x80              //!< misc error, should not occur
    } error_t;


  // PROTECTED TYPEDEFS
  protected:

    /// Type for frame callback function
    typedef void (*LinMessageCallback)(uint8_t numData, uint8_t* data);

    /// User-defined callback function with data length
    typedef struct
    {
      uint8_t                 type_numData;     //!< frame type (high nibble) and number of data bytes (low nibble)
      LinMessageCallback      fct;              //!< frame callback function
    } callback_t;


  // PROTECTED VARIABLES
  protected:

    // node properties
    int8_t                    pinTxEN;          //!< optional Tx direction pin, e.g. for LIN via RS485 
    uint16_t                  baudrate;         //!< communication baudrate [Baud]
    LIN_Slave_Base::version_t version;          //!< LIN protocol version
    LIN_Slave_Base::state_t   state;            //!< status of LIN state machine
    LIN_Slave_Base::error_t   error;            //!< error state. Is latched until cleared
    bool                      flagBreak;        //!< flag for BREAK detected. Needs to be set in Rx-ISR 
    LIN_Slave_Base::callback_t  callback[64];   //!< array of user callback functions for IDs 0x00..0x3F

    // latest frame properties
    uint8_t                   pid;              //!< protected frame identifier (0..255)
    uint8_t                   id;               //!< unprotected frame identifier (0..63)
    LIN_Slave_Base::frame_t   type;             //!< frame type (master request or slave response)
    uint8_t                   numData;          //!< number of data bytes in frame
    uint8_t                   bufData[9];       //!< buffer for data bytes (max. 8B) + checksum
    uint8_t                   idxData;          //!< current index in bufData
    uint32_t                  timeoutRx;        //!< timeout [us] for bytes in frame
    uint32_t                  timeLastRx;       //!< time [us] of last received byte in frame


  // PUBLIC VARIABLES
  public:

    char                      nameLIN[LIN_SLAVE_BUFLEN_NAME];   //!< LIN node name, e.g. for debug


  // PROTECTED METHODS
  protected:
  
    /// @brief Calculate protected frame ID
    uint8_t _calculatePID(uint8_t ID);
  
    /// @brief Calculate LIN frame checksum
    uint8_t _calculateChecksum(uint8_t NumData, uint8_t Data[]);

    /// @brief Get break detection flag. Is hardware dependent
    virtual bool _getBreakFlag(void);

    /// @brief Clear break detection flag. Is hardware dependent
    virtual void _resetBreakFlag(void);


    /// @brief peek next byte from Rx buffer. Here dummy
    virtual inline uint8_t _serialPeek(void) { return 0x00; }

    /// @brief read next byte from Rx buffer. Here dummy
    virtual inline uint8_t _serialRead(void) { return 0x00; }

    /// @brief write bytes to Tx buffer. Here dummy
    virtual inline void _serialWrite(uint8_t buf[], uint8_t num) { (void) buf; (void) num; }
    

    /// @brief Enable RS485 transmitter (DE=high)
    inline void _enableTransmitter(void)
    {   
      // print debug message
      DEBUG_PRINT(3, " ");

      // enable tranmitter
      if (this->pinTxEN >= 0)
        digitalWrite(this->pinTxEN, HIGH);
    
    } // _enableTransmitter()
    
    /// @brief Disable RS485 transmitter (DE=low)
    inline void _disableTransmitter(void)
    { 
      // print debug message
      DEBUG_PRINT(3, " ");

      // disable tranmitter
      if (this->pinTxEN >= 0)
        digitalWrite(this->pinTxEN, LOW);
            
    } // _disableTransmitter()


  // PUBLIC METHODS
  public:
  
    /// @brief LIN slave node constructor
    LIN_Slave_Base(LIN_Slave_Base::version_t Version = LIN_Slave_Base::LIN_V2, const char NameLIN[] = "Slave", 
      uint32_t TimeoutRx = 1500L, const int8_t PinTxEN = INT8_MIN);

    /// @brief LIN slave node destructor, here dummy. Any class with virtual functions should have virtual destructor 
    virtual ~LIN_Slave_Base(void) {};


    /// @brief Open serial interface
    virtual void begin(uint16_t Baudrate = 19200);
    
    /// @brief Close serial interface
    virtual void end(void);
    
    /// @brief check if a byte is available in Rx buffer. Here dummy
    virtual inline bool available(void) { return false; }

    
    /// @brief Reset LIN state machine
    inline void resetStateMachine(void)
    {
      // print debug message
      DEBUG_PRINT(3, " ");

      // reset state      
      this->state = LIN_Slave_Base::STATE_WAIT_FOR_BREAK;
      
    } // resetStateMachine()

    /// @brief Getter for LIN state machine state
    inline LIN_Slave_Base::state_t getState(void)
    {
      // print debug message
      DEBUG_PRINT(3, " ");

      // return state
      return this->state;

    } // getState()


    /// @brief Clear error of LIN state machine
    inline void resetError(void) 
    {
      // print debug message
      DEBUG_PRINT(3, " ");

      // reset error
      this->error = LIN_Slave_Base::NO_ERROR;

    } // resetError()
    
    /// @brief Getter for LIN state machine error
    inline LIN_Slave_Base::error_t getError(void)
    {
      // print debug message
      DEBUG_PRINT(3, " ");

      // return error
      return this->error;

    } // getError()

    
    /// @brief Getter for LIN frame
    inline void getFrame(LIN_Slave_Base::frame_t &Type, uint8_t &Id, uint8_t &NumData, uint8_t Data[])
    { 
      // print debug message
      DEBUG_PRINT(3, " ");

      noInterrupts();                         // for data consistency temporarily disable ISRs
      Type    = this->type;                   // frame type 
      Id      = this->id;                     // received frame ID
      NumData = this->numData;                // number of data bytes (excl. BREAK, SYNC, ID, CHK)
      memcpy(Data, this->bufData, NumData);   // copy data bytes w/o checksum
      interrupts();                           // re-enable ISRs

    } // getFrame()


    /// @brief Attach user callback function for master request frame
    void registerMasterRequestHandler(uint8_t ID, LIN_Slave_Base::LinMessageCallback Fct, uint8_t NumData);

    /// @brief Attach user callback function for slave response frame
    void registerSlaveResponseHandler(uint8_t ID, LIN_Slave_Base::LinMessageCallback Fct, uint8_t NumData);


    /// @brief Handle LIN protocol and call user-defined frame callbacks
    virtual void handler(void);

}; // class LIN_Slave_Base

/*-----------------------------------------------------------------------------
    END OF MODULE DEFINITION FOR MULTIPLE INLUSION
-----------------------------------------------------------------------------*/
#endif // _LIN_SLAVE_BASE_H_

/*-----------------------------------------------------------------------------
    END OF FILE
-----------------------------------------------------------------------------*/
