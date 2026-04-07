// ======================================================================
// \title  SX1303Manager.cpp
// \author ninhdh4
// \brief  cpp file for SX1303Manager component implementation class
// ======================================================================

#include "fprime-sx1303/Components/SX1303Manager/SX1303Manager.hpp"
#include "fprime-sx1303/Subtopology/SubtopologyTopologyDefs.hpp"
extern "C" {
#include "fprime-sx1303/Components/SX1303Hal/libloragw/inc/config.h"
#include "fprime-sx1303/Components/SX1303Hal/libloragw/inc/loragw_hal.h"
}

// Static pointer to the manager instance for C callback
static SX1303::SX1303Manager* g_sx1303_manager = nullptr;

#define	EXIT_FAILURE	1	/* Failing exit status.  */
#define	EXIT_SUCCESS	0	/* Successful exit status.  */

#define LOG_DEBUG(fmt, ...) \
    do { \
        Fw::LogStringArg _msg; \
        _msg.format(fmt, ##__VA_ARGS__); \
        g_sx1303_manager->log_debug(_msg); \
    } while(0)

extern "C" {
    void sx1303_delay_ms(U32 ms) {
        if (g_sx1303_manager) {
            g_sx1303_manager->delay_ms(ms);
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
}

extern "C" {

    #define DEFAULT_CLK_SRC     0
    #define DEFAULT_FREQ_HZ     915000000U
    #define COM_TYPE_DEFAULT    LGW_COM_SPI
    #define COM_PATH_DEFAULT    "/dev/spidev0.0"

    int test_hal_tx_main(void)
    {
        int i, x;
        unsigned int ft = DEFAULT_FREQ_HZ;
        int8_t rf_power = 2;
        uint8_t sf = 0;
        uint16_t bw_khz = 125;
        unsigned int nb_pkt = 1;
        unsigned int nb_loop = 1, cnt_loop;
        uint8_t size = 0;
        char mod[64] = "LORA";
        float br_kbps = 50;
        uint8_t fdev_khz = 25;
        int8_t freq_offset = 0;
        double arg_d = 0.0;
        unsigned int arg_u;
        int arg_i;
        char arg_s[64];
        float xf = 0.0;
        uint8_t clocksource = 0;
        uint8_t rf_chain = 0;
        lgw_radio_type_t radio_type = LGW_RADIO_TYPE_SX1250;
        uint16_t preamble = 8;
        bool invert_pol = false;
        bool no_header = false;
        bool single_input_mode = false;
        bool full_duplex = false;

        struct lgw_conf_board_s boardconf;
        struct lgw_conf_rxrf_s rfconf;
        struct lgw_pkt_tx_s pkt;
        struct lgw_tx_gain_lut_s txlut; /* TX gain table */
        uint8_t tx_status;
        uint32_t count_us;
        uint32_t trig_delay_us = 1000000;
        bool trig_delay = false;

        /* SPI interfaces */
        const char com_path_default[] = COM_PATH_DEFAULT;
        const char * com_path = com_path_default;
        lgw_com_type_t com_type = COM_TYPE_DEFAULT;

        /* Initialize TX gain LUT */
        txlut.size = 0;
        memset(txlut.lut, 0, sizeof txlut.lut);

        /* Summary of packet parameters */
        if (strcmp(mod, "CW") == 0) {
            LOG_DEBUG("Sending %i CW on %u Hz (Freq. offset %d kHz) at %i dBm\n", nb_pkt, ft, freq_offset, rf_power);
        }
        else if (strcmp(mod, "FSK") == 0) {
            LOG_DEBUG("Sending %i FSK packets on %u Hz (FDev %u kHz, Bitrate %.2f, %i bytes payload, %i symbols preamble) at %i dBm\n", nb_pkt, ft, fdev_khz, br_kbps, size, preamble, rf_power);
        } else {
            LOG_DEBUG("Sending %i LoRa packets on %u Hz (BW %i kHz, SF %i, CR %i, %i bytes payload, %i symbols preamble, %s header, %s polarity) at %i dBm\n", nb_pkt, ft, bw_khz, sf, 1, size, preamble, (no_header == false) ? "explicit" : "implicit", (invert_pol == false) ? "non-inverted" : "inverted", rf_power);
        }

        /* Configure the gateway */
        memset( &boardconf, 0, sizeof boardconf);
        boardconf.lorawan_public = true;
        boardconf.clksrc = clocksource;
        boardconf.full_duplex = full_duplex;
        boardconf.com_type = com_type;
        strncpy(boardconf.com_path, com_path, sizeof boardconf.com_path);
        boardconf.com_path[sizeof boardconf.com_path - 1] = '\0'; /* ensure string termination */
        if (lgw_board_setconf(&boardconf) != LGW_HAL_SUCCESS) {
            LOG_DEBUG("ERROR: failed to configure board\n");
            return EXIT_FAILURE;
        }

        memset( &rfconf, 0, sizeof rfconf);
        rfconf.enable = true; /* rf chain 0 needs to be enabled for calibration to work on sx1257 */
        rfconf.freq_hz = ft;
        rfconf.type = radio_type;
        rfconf.tx_enable = true;
        rfconf.single_input_mode = single_input_mode;
        if (lgw_rxrf_setconf(0, &rfconf) != LGW_HAL_SUCCESS) {
            LOG_DEBUG("ERROR: failed to configure rxrf 0\n");
            return EXIT_FAILURE;
        }

        memset( &rfconf, 0, sizeof rfconf);
        rfconf.enable = (((rf_chain == 1) || (clocksource == 1)) ? true : false);
        rfconf.freq_hz = ft;
        rfconf.type = radio_type;
        rfconf.tx_enable = false;
        rfconf.single_input_mode = single_input_mode;
        if (lgw_rxrf_setconf(1, &rfconf) != LGW_HAL_SUCCESS) {
            LOG_DEBUG("ERROR: failed to configure rxrf 1\n");
            return EXIT_FAILURE;
        }

        if (txlut.size > 0) {
            if (lgw_txgain_setconf(rf_chain, &txlut) != LGW_HAL_SUCCESS) {
                LOG_DEBUG("ERROR: failed to configure txgain lut\n");
                return EXIT_FAILURE;
            }
        }

        if (com_type == LGW_COM_SPI) {
            sx1303_power_on(true);
            /* Board reset */
            sx1303_reset();
        }

        /* connect, configure and start the LoRa concentrator */
        x = lgw_start();
        if (x != 0) {
            LOG_DEBUG("ERROR: %s: failed to start the gateway\n", __func__);
            return EXIT_FAILURE;
        }

        /* Send packets */
        memset(&pkt, 0, sizeof pkt);
        pkt.rf_chain = rf_chain;
        pkt.freq_hz = ft;
        pkt.rf_power = rf_power;
        if (trig_delay == false) {
            pkt.tx_mode = IMMEDIATE;
        } else {
            if (trig_delay_us == 0) {
                pkt.tx_mode = ON_GPS;
            } else {
                pkt.tx_mode = TIMESTAMPED;
            }
        }
        if ( strcmp( mod, "CW" ) == 0 ) {
            pkt.modulation = MOD_CW;
            pkt.freq_offset = freq_offset;
            pkt.f_dev = fdev_khz;
        }
        else if( strcmp( mod, "FSK" ) == 0 ) {
            pkt.modulation = MOD_FSK;
            pkt.no_crc = false;
            pkt.datarate = br_kbps * 1e3;
            pkt.f_dev = fdev_khz;
        } else {
            pkt.modulation = MOD_LORA;
            pkt.coderate = CR_LORA_4_5;
            pkt.no_crc = true;
        }
        pkt.invert_pol = invert_pol;
        pkt.preamble = preamble;
        pkt.no_header = no_header;
        pkt.payload[0] = 0x40; /* Confirmed Data Up */
        pkt.payload[1] = 0xAB;
        pkt.payload[2] = 0xAB;
        pkt.payload[3] = 0xAB;
        pkt.payload[4] = 0xAB;
        pkt.payload[5] = 0x00; /* FCTrl */
        pkt.payload[6] = 0; /* FCnt */
        pkt.payload[7] = 0; /* FCnt */
        pkt.payload[8] = 0x02; /* FPort */
        for (i = 9; i < 255; i++) {
            pkt.payload[i] = i;
        }

        for (i = 0; i < (int)nb_pkt; i++) {
            if (trig_delay == true) {
                if (trig_delay_us > 0) {
                    lgw_get_instcnt(&count_us);
                    LOG_DEBUG("count_us:%u\n", count_us);
                    pkt.count_us = count_us + trig_delay_us;
                    LOG_DEBUG("programming TX for %u\n", pkt.count_us);
                } else {
                    LOG_DEBUG("programming TX for next PPS (GPS)\n");
                }
            }

            if( strcmp( mod, "LORA" ) == 0 ) {
                pkt.datarate = 7;
            }

            pkt.bandwidth = BW_125KHZ;
            pkt.size = 32;

            pkt.payload[6] = (uint8_t)(i >> 0); /* FCnt */
            pkt.payload[7] = (uint8_t)(i >> 8); /* FCnt */
            x = lgw_send(&pkt);
            if (x != 0) {
                LOG_DEBUG("ERROR: failed to send packet\n");
                break;
            }
            /* wait for packet to finish sending */
            do {
                sx1303_delay_ms(5);
                lgw_status(pkt.rf_chain, TX_STATUS, &tx_status); /* get TX status */
            } while (tx_status != TX_FREE);

            LOG_DEBUG("TX done\n");
        }

        LOG_DEBUG( "\nNb packets sent: %u (%u)\n", i, cnt_loop + 1 );

        /* Stop the gateway */
        x = lgw_stop();
        if (x != 0) {
            LOG_DEBUG("ERROR: failed to stop the gateway\n");
        }


        LOG_DEBUG("=========== Test End ===========\n");
        return 0;
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
}

void SX1303Manager ::log_debug(const Fw::LogStringArg& msg) {
    this->log_ACTIVITY_LO_Debug(msg);
}

bool SX1303Manager ::power_on(bool on) {
    if (this->isConnected_powerGpioWrite_OutputPort(0)) {
        this->powerGpioWrite_out(0, on ? Fw::Logic::HIGH : Fw::Logic::LOW);
        m_state = on ? SX1303::SX1303Manager_gwState::POWER_ON : SX1303::SX1303Manager_gwState::POWER_OFF;
        this->log_ACTIVITY_HI_GwState(m_state);
        this->tlmWrite_GatewayState(m_state);
        Os::Task::delay(Fw::TimeInterval(0, 100));
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
    if (test_hal_tx_main() == 0){
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
