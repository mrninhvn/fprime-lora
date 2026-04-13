// ======================================================================
// \title  LoRaMacProcessor.hpp
// \author ninhdh4
// \brief  hpp file for LoRaMacProcessor component implementation class
// ======================================================================

#ifndef LORAMAC_LoRaMacProcessor_HPP
#define LORAMAC_LoRaMacProcessor_HPP

#include "fprime-loramac/Components/LoRaMacProcessor/LoRaMacProcessorComponentAc.hpp"

namespace LORAMAC {

class LoRaMacProcessor final : public LoRaMacProcessorComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct LoRaMacProcessor object
    LoRaMacProcessor(const char* const compName  //!< The component name
    );

    //! Destroy LoRaMacProcessor object
    ~LoRaMacProcessor();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for bufferLikeIn
    //!
    //! Port to receive buffer like data
    void bufferLikeIn_handler(FwIndexType portNum,  //!< The port number
                              Fw::Buffer& fwBuffer  //!< The buffer
                              ) override;

    //! Handler implementation for byteStreamLikeIn
    //!
    //! Port to receive byte stream like data
    void byteStreamLikeIn_handler(FwIndexType portNum,  //!< The port number
                                  Fw::Buffer& buffer,
                                  const Drv::ByteStreamStatus& status) override;

    //! Handler implementation for commLikeIn
    //!
    //! Port to receive comm like data
    void commLikeIn_handler(FwIndexType portNum,  //!< The port number
                            Fw::Buffer& data,
                            const ComCfg::FrameContext& context) override;
};

}  // namespace LORAMAC

#endif
