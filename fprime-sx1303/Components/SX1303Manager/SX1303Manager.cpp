// ======================================================================
// \title  SX1303Manager.cpp
// \author ninhdh4
// \brief  cpp file for SX1303Manager component implementation class
// ======================================================================

#include "fprime-sx1303/Components/SX1303Manager/SX1303Manager.hpp"
#include "fprime-sx1303/Components/SX1303Manager/SX1303Gateway.hpp"
#include "fprime-sx1303/Components/SX1303Manager/SX1303TestRadio.hpp"
#include "fprime-sx1303/Subtopology/SubtopologyTopologyDefs.hpp"
extern "C" {
#include "fprime-sx1303/Components/SX1303Hal/libloragw/inc/config.h"
#include "fprime-sx1303/Components/SX1303Hal/libloragw/inc/loragw_hal.h"
}

// Static pointer to the manager instance for C callback
static SX1303::SX1303Manager* g_sx1303_manager = nullptr;

#define SX1303_DEBUG 1
#define LORA_MAC_EXT 0

#define LOG_DEBUG(fmt, ...) \
    do { \
        Fw::LogStringArg _msg; \
        _msg.format(fmt, ##__VA_ARGS__); \
        g_sx1303_manager->log_debug(_msg); \
    } while(0)

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
    switch (m_state) {
        case SX1303::SX1303Manager_gwState::RUNNING: {
            sx1303_gateway_receive();
        }
    }
}

void SX1303Manager ::log_debug(const Fw::LogStringArg& msg) {
    #if SX1303_DEBUG
    this->log_ACTIVITY_LO_Debug(msg);
    #endif
}

bool SX1303Manager ::power_on(bool on) {
    if (this->isConnected_powerGpioWrite_OutputPort(0)) {
        this->powerGpioWrite_out(0, on ? Fw::Logic::HIGH : Fw::Logic::LOW);
        m_state = on ? SX1303::SX1303Manager_gwState::POWER_ON : SX1303::SX1303Manager_gwState::POWER_OFF;
        this->log_ACTIVITY_HI_GwState(m_state);
        this->tlmWrite_GatewayState(m_state);
        // Os::Task::delay(Fw::TimeInterval(0, 100));
        Os::Task::delay(Fw::TimeInterval(0, 100000));
        return true;
    }
    return false;
}

bool SX1303Manager ::reset() {
    // resetGateway();
    if (this->isConnected_resetGpioWrite_OutputPort(0)) {
        this->resetGpioWrite_out(0, Fw::Logic::HIGH);
        // Os::Task::delay(Fw::TimeInterval(0, 100));
        Os::Task::delay(Fw::TimeInterval(0, 100000));
        this->resetGpioWrite_out(0, Fw::Logic::LOW);
        // Os::Task::delay(Fw::TimeInterval(0, 100));
        Os::Task::delay(Fw::TimeInterval(0, 100000));

        m_state = SX1303::SX1303Manager_gwState::RESET;
        this->log_ACTIVITY_HI_GwState(m_state);
        this->tlmWrite_GatewayState(m_state);
        return true;
    }
    return false;
}

bool SX1303Manager ::spi_transfer(Fw::Buffer& writeBuffer, Fw::Buffer& readBuffer) {
    // Validate buffer sizes
    FW_ASSERT(writeBuffer.getSize() != 0);

    FW_ASSERT(readBuffer.getSize() != 0);

    FW_ASSERT(writeBuffer.getSize() == readBuffer.getSize());

    // Perform the SPI transfer
    // Note: spiReadWrite_out calls the underlying SPI driver
    if (this->isConnected_spiReadWrite_OutputPort(0)) {
        // LOG_DEBUG("%s: writeBuffer size=%u, readBuffer size=%u", __func__, writeBuffer.getSize(), readBuffer.getSize());
        this->spiReadWrite_out(0, writeBuffer, readBuffer);
    }

    return true;
}

void SX1303Manager ::send_gw_packet(const SX1303::SX1303Data& packet){
    this->tlmWrite_GatewayPacket(packet);
}

bool SX1303Manager ::get_time(Fw::Time &systime) {
    if (!this->isConnected_timeCaller_OutputPort(0)) {
        return false;
    }
    systime = getTime();
    return true;
}

#if LORA_MAC_EXT
void SX1303Manager ::lora_out(const uint8_t* data, size_t len) {
    Fw::Buffer buffer(const_cast<uint8_t*>(data), len);
    this->loraOut_out(0, buffer);
}
#endif

void SX1303Manager ::set_gw_state(SX1303Manager_gwState state) {
    m_state = state;
    this->log_ACTIVITY_HI_GwState(m_state);
    this->tlmWrite_GatewayState(m_state);
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
    this->power_on(true);
    if (this->reset()) {
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
    }
    else {
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
    }
}

void SX1303Manager ::GW_TEST_TX_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    if (sx1303_test_hal_tx() == 0){
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
    }
    else {
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
    }
}

void SX1303Manager ::GW_START_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, bool publicNet, SX1303::SX1303Manager_gwChPlan region) {
    if (sx1303_gateway_start(publicNet, region) == 0){
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
        m_state = SX1303::SX1303Manager_gwState::RUNNING;
        this->log_ACTIVITY_HI_GwState(m_state);
        this->tlmWrite_GatewayState(m_state);
    }
    else {
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
    }
}

}  // namespace SX1303


extern "C" {
    void sx1303_delay_ms(U32 ms) {
        if (g_sx1303_manager) {
            g_sx1303_manager->delay_ms(ms);
        }
    }

    void sx1303_delay_us(U32 us) {
        if (g_sx1303_manager) {
            g_sx1303_manager->delay_us(us);
        }
    }

    void sx1303_log_debug(const char* fmt, ...) {
        char buf[200];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        Fw::LogStringArg arg(buf);
        g_sx1303_manager->log_debug(arg);
    }

    void sx1303_power_on(bool on) {
        if (g_sx1303_manager) {
            g_sx1303_manager->power_on(on);
        }
    }

    void sx1303_reset(void) {
        if (g_sx1303_manager) {
            g_sx1303_manager->reset();
        }
    }

    void *sx1303_spi_device(void) {
        return (void*)&g_sx1303_manager->get_device();
    }

    int sx1303_spi_rw(const uint8_t* writeData, uint8_t* readData, size_t len) {
        Fw::Buffer writeBuffer(const_cast<uint8_t*>(writeData), len);
        Fw::Buffer readBuffer(readData, len);
        // LOG_DEBUG("%s: writeData size=%u, readData size=%u", __func__, writeBuffer.getSize(), readBuffer.getSize());
        g_sx1303_manager->spi_transfer(writeBuffer, readBuffer);
        return 0;
    }

    void sx1303_set_state(U8 state) {
        if (g_sx1303_manager) {
            g_sx1303_manager->set_gw_state(static_cast<SX1303::SX1303Manager_gwState::T>(state));
        }
    }

    void sx1303_send_gw_packet(const SX1303::SX1303Data& packet){
        g_sx1303_manager->send_gw_packet(packet);
    }

    bool sx1303_get_time(Fw::Time &systime){
        return g_sx1303_manager->get_time(systime);
    }

#if LORA_MAC_EXT
    void sx1303_lora_out(const uint8_t* data, size_t len) {
        if (g_sx1303_manager) {
            g_sx1303_manager->lora_out(data, len);
        }
    }
#endif
}
