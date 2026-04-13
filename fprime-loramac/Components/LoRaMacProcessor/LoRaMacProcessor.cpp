// ======================================================================
// \title  LoRaMacProcessor.cpp
// \author ninhdh4
// \brief  cpp file for LoRaMacProcessor component implementation class
// ======================================================================

#include "fprime-loramac/Components/LoRaMacProcessor/LoRaMacProcessor.hpp"

namespace LORAMAC {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

LoRaMacProcessor ::LoRaMacProcessor(const char* const compName) : LoRaMacProcessorComponentBase(compName) {}

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
}

void LoRaMacProcessor ::byteStreamLikeIn_handler(FwIndexType portNum,
                                               Fw::Buffer& buffer,
                                               const Drv::ByteStreamStatus& status) {
    // TODO
}

void LoRaMacProcessor ::commLikeIn_handler(FwIndexType portNum, Fw::Buffer& data, const ComCfg::FrameContext& context) {
    // TODO
}

}  // namespace LORAMAC
