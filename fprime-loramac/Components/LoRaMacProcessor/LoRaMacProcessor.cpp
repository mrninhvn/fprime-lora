// ======================================================================
// \title  LoRaMacProcessor.cpp
// \author ninhdh4
// \brief  cpp file for LoRaMacProcessor component implementation class
// ======================================================================

#include "fprime-loramac/Components/LoRaMacProcessor/LoRaMacProcessor.hpp"

extern "C" {
#include "fprime-loramac/Components/LoRaMac/src/mac/region/Region.h"
#include "fprime-loramac/Components/LoRaMac/src/mac/LoRaMacParser.h"
#include "fprime-loramac/Components/LoRaMac/src/mac/LoRaMac.h"
#include "fprime-loramac/Components/LoRaMac/src/mac/LoRaMacCrypto.h"
}

static LORAMAC::LoRaMacProcessor *g_loramac_processor = nullptr;

namespace LORAMAC {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

LoRaMacProcessor ::LoRaMacProcessor(const char* const compName) : LoRaMacProcessorComponentBase(compName) {
    g_loramac_processor = this;
}

LoRaMacProcessor ::~LoRaMacProcessor() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void LoRaMacProcessor ::bufferLikeIn_handler(FwIndexType portNum, Fw::Buffer& fwBuffer) {
    U8* data = fwBuffer.getData();
    U32 size = fwBuffer.getSize();

    ProcessLoraMac(data, size);
}

void LoRaMacProcessor ::byteStreamLikeIn_handler(FwIndexType portNum,
                                               Fw::Buffer& buffer,
                                               const Drv::ByteStreamStatus& status) {
    // TODO
}

void LoRaMacProcessor ::commLikeIn_handler(FwIndexType portNum, Fw::Buffer& data, const ComCfg::FrameContext& context) {
    // TODO
}

void LoRaMacProcessor ::log_debug(const Fw::LogStringArg& msg) {
    this->log_ACTIVITY_LO_Debug(msg);
}


}  // namespace LORAMAC

extern "C" {

#define LORAMAC_PHY_MAXPAYLOAD      256

struct lgw_pkt_rx_s {
    uint32_t    freq_hz;        /*!> central frequency of the IF chain */
    int32_t     freq_offset;
    uint8_t     if_chain;       /*!> by which IF chain was packet received */
    uint8_t     status;         /*!> status of the received packet */
    uint32_t    count_us;       /*!> internal concentrator counter for timestamping, 1 microsecond resolution */
    uint8_t     rf_chain;       /*!> through which RF chain the packet was received */
    uint8_t     modem_id;
    uint8_t     modulation;     /*!> modulation used by the packet */
    uint8_t     bandwidth;      /*!> modulation bandwidth (LoRa only) */
    uint32_t    datarate;       /*!> RX datarate of the packet (SF for LoRa) */
    uint8_t     coderate;       /*!> error-correcting code of the packet (LoRa only) */
    float       rssic;          /*!> average RSSI of the channel in dB */
    float       rssis;          /*!> average RSSI of the signal in dB */
    float       snr;            /*!> average packet SNR, in dB (LoRa only) */
    float       snr_min;        /*!> minimum packet SNR, in dB (LoRa only) */
    float       snr_max;        /*!> maximum packet SNR, in dB (LoRa only) */
    uint16_t    crc;            /*!> CRC that was received in the payload */
    uint16_t    size;           /*!> payload size in bytes */
    uint8_t     payload[256];   /*!> buffer containing the payload */
    bool        ftime_received; /*!> a fine timestamp has been received */
    uint32_t    ftime;          /*!> packet fine timestamp (nanoseconds since last PPS) */
};

void ProcessLoraMac( uint8_t *packet, uint16_t size ) {
    LoRaMacHeader_t macHdr;
    ApplyCFListParams_t applyCFList;
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;
    LoRaMacCryptoStatus_t macCryptoStatus = LORAMAC_CRYPTO_ERROR;

    LoRaMacMessageData_t macMsgData;
    uint8_t pktHeaderLen = 0;
    FCntIdentifier_t fCntID;
    uint8_t macCmdPayload[2] = { 0 };
    uint8_t rxPayload[LORAMAC_PHY_MAXPAYLOAD];

    // Abort on empty radio frames
    if( packet == nullptr || size == 0 ) {
        loramac_log_debug("empty radio frames");
        return;
    }

    const lgw_pkt_rx_s* gw_packet = reinterpret_cast<const lgw_pkt_rx_s*>(packet);
    const uint8_t *payload = gw_packet->payload;

    macHdr.Value = payload[pktHeaderLen++];

    // Accept frames of LoRaWAN Major Version 1 only
    if( macHdr.Bits.Major != 0 ) {
        loramac_log_debug("Wrong Major version: %d", macHdr.Bits.Major);
        return;
    }

    switch( macHdr.Bits.MType ) {
        case FRAME_TYPE_JOIN_ACCEPT: {
            loramac_log_debug("Message Type: JOIN_ACCEPT");
            break;
        }
        case FRAME_TYPE_DATA_CONFIRMED_DOWN: {
            loramac_log_debug("Message Type: DATA_CONFIRMED_DOWN");
            break;
        }
        case FRAME_TYPE_DATA_UNCONFIRMED_DOWN:{
            loramac_log_debug("Message Type: DATA_UNCONFIRMED_DOWN");
            break;
        }
        case FRAME_TYPE_DATA_CONFIRMED_UP:
        case FRAME_TYPE_DATA_UNCONFIRMED_UP: {
            loramac_log_debug("Message Type: %s, size: %d", macHdr.Bits.MType == FRAME_TYPE_DATA_CONFIRMED_UP ? "DATA_CONFIRMED_UP" : "DATA_UNCONFIRMED_UP", gw_packet->size);
            
            /* Remove by unkown region DR
            // Check if the received payload size is valid
            getPhy.UplinkDwellTime = 0; // unlimited time
            getPhy.Datarate = DR_0;
            getPhy.Attribute = PHY_MAX_PAYLOAD;
            phyParam = RegionGetPhyParam( LORAMAC_REGION_EU868, &getPhy );
            if( ( MAX( 0, ( int16_t )( ( int16_t ) gw_packet->size - ( int16_t ) LORAMAC_FRAME_PAYLOAD_OVERHEAD_SIZE ) ) > ( int16_t )phyParam.Value ) ||
                ( gw_packet->size < LORAMAC_FRAME_PAYLOAD_MIN_SIZE ) ) {
                loramac_log_debug("Invalid payload size %d", phyParam.Value);
                return;
            }
            */

            macMsgData.Buffer = const_cast<uint8_t*>(payload);
            macMsgData.BufSize = gw_packet->size;
            macMsgData.FRMPayload = rxPayload;
            macMsgData.FRMPayloadSize = LORAMAC_PHY_MAXPAYLOAD;

            if( LORAMAC_PARSER_SUCCESS != LoRaMacParserData( &macMsgData ) ) {
                loramac_log_debug("LoRaMacParserData error");
                return;
            }
            loramac_log_debug("DevAddr: 0x%08X, MIC: 0x%08X", macMsgData.FHDR.DevAddr, macMsgData.MIC);

            FType_t fType;
            if( LORAMAC_STATUS_OK != DetermineFrameType( &macMsgData, &fType ) ) {
                loramac_log_debug("DetermineFrameType Error");
                return;
            }

            switch( fType ) {
                case FRAME_TYPE_A: {
                   /* +----------+------+-------+--------------+
                    * | FOptsLen | Fopt | FPort |  FRMPayload  |
                    * +----------+------+-------+--------------+
                    * |    > 0   |   X  |  > 0  |       X      |
                    * +----------+------+-------+--------------+
                    */
                    loramac_log_debug("FRAME_TYPE_A");

                    // Decode MAC commands in FOpts field
                    // ProcessMacCommands( macMsgData.FHDR.FOpts, 0, macMsgData.FHDR.FCtrl.Bits.FOptsLen, snr, MacCtx.McpsIndication.RxSlot );
                    break;
                }
                case FRAME_TYPE_B: {
                   /* +----------+------+-------+--------------+
                    * | FOptsLen | Fopt | FPort |  FRMPayload  |
                    * +----------+------+-------+--------------+
                    * |    > 0   |   X  |   -   |       -      |
                    * +----------+------+-------+--------------+
                    */
                    loramac_log_debug("FRAME_TYPE_B");

                    // Decode MAC commands in FOpts field
                    // ProcessMacCommands( macMsgData.FHDR.FOpts, 0, macMsgData.FHDR.FCtrl.Bits.FOptsLen, snr, MacCtx.McpsIndication.RxSlot );
                    break;
                }
                case FRAME_TYPE_C: {
                   /* +----------+------+-------+--------------+
                    * | FOptsLen | Fopt | FPort |  FRMPayload  |
                    * +----------+------+-------+--------------+
                    * |    = 0   |   -  |  = 0  | MAC commands |
                    * +----------+------+-------+--------------+
                    */
                    loramac_log_debug("FRAME_TYPE_C");

                    // Decode MAC commands in FRMPayload
                    // ProcessMacCommands( macMsgData.FRMPayload, 0, macMsgData.FRMPayloadSize, snr, MacCtx.McpsIndication.RxSlot );
                    break;
                }
                case FRAME_TYPE_D: {
                   /* +----------+------+-------+--------------+
                    * | FOptsLen | Fopt | FPort |  FRMPayload  |
                    * +----------+------+-------+--------------+
                    * |    = 0   |   -  |  > 0  |       X      |
                    * +----------+------+-------+--------------+
                    */
                    loramac_log_debug("FRAME_TYPE_D");
                    // No MAC commands just application payload
                    break;
                }
                default:
                    loramac_log_debug("ERROR FRAME_TYPE");;
                    break;
            }
            break;
        }
        case FRAME_TYPE_PROPRIETARY: {
            loramac_log_debug("Message Type: PROPRIETARY");
            break;
        }
        default: {
            loramac_log_debug("Message Type: UNKNOWN");
            break;
        }
    }
}

void loramac_log_debug(const char* fmt, ...) {
        char buf[200];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        Fw::LogStringArg arg(buf);
        g_loramac_processor->log_debug(arg);
    }

}  // extern "C"
