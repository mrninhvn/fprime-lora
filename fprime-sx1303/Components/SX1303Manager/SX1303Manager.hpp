// ======================================================================
// \title  SX1303Manager.hpp
// \author ninhdh4
// \brief  hpp file for SX1303Manager component implementation class
// ======================================================================

#ifndef SX1303_SX1303Manager_HPP
#define SX1303_SX1303Manager_HPP

#include "fprime-sx1303/Components/SX1303Manager/SX1303ManagerComponentAc.hpp"
#include "fprime-sx1303/Subtopology/SubtopologyTopologyDefs.hpp"

namespace SX1303 {

class SX1303Manager final : public SX1303ManagerComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct SX1303Manager object
    SX1303Manager(const char* const compName  //!< The component name
    );

    //! Destroy SX1303Manager object
    ~SX1303Manager();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for run
    //!
    //! Example port: receiving calls from the rate group
    void run_handler(FwIndexType portNum,  //!< The port number
                     U32 context           //!< The call order
                     ) override;

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for commands
    // ----------------------------------------------------------------------

    //! Handler implementation for command GW_ON_OFF
    //!
    //! Command to turn on or off the gateway
    void GW_ON_OFF_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                              U32 cmdSeq,           //!< The command sequence number
                              Fw::On onOff          //!< Indicates whether the gateway should be on or off
                              ) override;

    //! Handler implementation for command GwReset
    //!
    //! Reset the gateway
    void GW_RESET_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                            U32 cmdSeq            //!< The command sequence number
                            ) override;

    //! Handler implementation for command GW_TEST_TX
    //!
    //! Lora test transmission command
    void GW_TEST_TX_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                             U32 cmdSeq            //!< The command sequence number
                             ) override;

    //! Handler implementation for command GW_TEST_RX
    //!
    //! Lora test reception command
    void GW_TEST_RX_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                             U32 cmdSeq            //!< The command sequence number
                             ) override;

    //! Handler implementation for command ReportNodeIdentifier
    //!
    //! Report the radio serial number
    void ReportNodeIdentifier_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                                         U32 cmdSeq            //!< The command sequence number
                                         ) override;
  
  private:
    SubtopologyState sx1303State;
    //! Tracks the state of the SX1303
    SX1303::SX1303Manager_gwState m_state = SX1303::SX1303Manager_gwState::POWER_OFF;
    Fw::On led_state;

  public:
    //! Power on gateway
    bool power_on(bool on);
    //! Reset the gateway by toggling the appropriate GPIO pins
    bool reset();
    //! Get SPI device
    SX1303Device& get_device() { return sx1303State.device; }
    //! Write to the SPI bus and handle errors
    bool spi_transfer(Fw::Buffer& writeBuffer, Fw::Buffer& readBuffer);
    //! Debug Logging
    void log_debug(const Fw::LogStringArg& msg);
    //! Delay ms
    void delay_ms(U32 ms) { Os::Task::delay(Fw::TimeInterval(0, ms)); }
    void lora_out(const uint8_t* data, size_t len);
};

}  // namespace SX1303

extern "C" {
    void sx1303_delay_ms(U32 ms);
    void sx1303_log_debug(const char* fmt, ...);
    void sx1303_power_on(bool on);
    void sx1303_reset(void);
    void *sx1303_spi_device(void);
    int sx1303_spi_rw(const uint8_t* writeData, uint8_t* readData, size_t len);
    void sx1303_lora_out(const uint8_t* data, size_t len);
}

#endif
