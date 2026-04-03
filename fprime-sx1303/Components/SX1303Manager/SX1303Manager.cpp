// ======================================================================
// \title  SX1303Manager.cpp
// \author ninhdh4
// \brief  cpp file for SX1303Manager component implementation class
// ======================================================================

#include "fprime-sx1303/Components/SX1303Manager/SX1303Manager.hpp"

// Static pointer to the manager instance for C callback
static SX1303::SX1303Manager* g_sx1303_manager = nullptr;

extern "C" {
    void sx1303_manager_reset(void) {
        if (g_sx1303_manager) {
            g_sx1303_manager->reset();
        }
    }
}

namespace SX1303 {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

SX1303Manager ::SX1303Manager(const char* const compName) : SX1303ManagerComponentBase(compName) {
    g_sx1303_manager = this;
}

SX1303Manager ::~SX1303Manager() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void SX1303Manager ::run_handler(FwIndexType portNum, U32 context) {
    // TODO
    // Toggle state
    this->led_state = (this->led_state == Fw::On::ON) ? Fw::On::OFF : Fw::On::ON;
    if (this->isConnected_resetGpioWrite_OutputPort(0)) {
        this->resetGpioWrite_out(0, (Fw::On::ON == this->led_state) ? Fw::Logic::HIGH : Fw::Logic::LOW);
    }
}

bool SX1303Manager ::power_on(bool on) {
    if (this->isConnected_powerGpioWrite_OutputPort(0)) {
        this->powerGpioWrite_out(0, on ? Fw::Logic::HIGH : Fw::Logic::LOW);
        m_state = on ? SX1303::SX1303Manager_gwState::POWER_ON : SX1303::SX1303Manager_gwState::POWER_OFF;
        this->log_ACTIVITY_HI_GwState(m_state);
        this->tlmWrite_GatewayState(m_state);
        return true;
    }
    return false;
}

bool SX1303Manager ::reset() {
    // resetGateway();
    if (this->isConnected_resetGpioWrite_OutputPort(0)) {
        this->resetGpioWrite_out(0, Fw::Logic::HIGH);
        Os::Task::delay(Fw::TimeInterval(0, 100));
        this->resetGpioWrite_out(0, Fw::Logic::LOW);
        Os::Task::delay(Fw::TimeInterval(0, 100));

        m_state = SX1303::SX1303Manager_gwState::RESET;
        this->log_ACTIVITY_HI_GwState(m_state);
        this->tlmWrite_GatewayState(m_state);
        return true;
    }
    return false;
}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

void SX1303Manager ::GW_ON_OFF_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, Fw::On onOff) {
    // TODO
    if (this->power_on(onOff == Fw::On::ON)) {
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
    }
    else {
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
    }
}

void SX1303Manager ::GW_RESET_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    if (this->reset()) {
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
    }
    else {
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
    }
}

void SX1303Manager ::ReportNodeIdentifier_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    // TODO
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace SX1303
