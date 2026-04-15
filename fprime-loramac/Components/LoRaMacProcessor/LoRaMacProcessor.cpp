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

    char hexBuf[200] = {0};
    U32 offset = 0;
    for (U32 i = 0; i < size && offset < sizeof(hexBuf) - 3; i++) {
        offset += snprintf(hexBuf + offset, sizeof(hexBuf) - offset, "%02X ", data[i]);
    }

    Fw::String msg(hexBuf);
    this->log_ACTIVITY_LO_Debug(msg);

    ProcessLoraMac(data, size, 0, 0);
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

#define LORAMAC_PHY_MAXPAYLOAD      255

void ProcessLoraMac( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr ) {
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
    if( size == 0 ) {
        loramac_log_debug("empty radio frames");
        return;
    }

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
            loramac_log_debug("Message Type: %s", macHdr.Bits.MType == FRAME_TYPE_DATA_CONFIRMED_UP ? "DATA_CONFIRMED_UP" : "DATA_UNCONFIRMED_UP");
            // Check if the received payload size is valid
            getPhy.UplinkDwellTime = 0; // unlimited time
            getPhy.Datarate = DR_2;
            getPhy.Attribute = PHY_MAX_PAYLOAD;
            phyParam = RegionGetPhyParam( LORAMAC_REGION_AS923, &getPhy );
            if( ( MAX( 0, ( int16_t )( ( int16_t ) size - ( int16_t ) LORAMAC_FRAME_PAYLOAD_OVERHEAD_SIZE ) ) > ( int16_t )phyParam.Value ) ||
                ( size < LORAMAC_FRAME_PAYLOAD_MIN_SIZE ) ) {
                loramac_log_debug("Invalid payload size %d", phyParam.Value);
                return;
            }

            macMsgData.Buffer = payload;
            macMsgData.BufSize = size;
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
