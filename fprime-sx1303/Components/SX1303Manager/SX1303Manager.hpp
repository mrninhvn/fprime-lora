// ======================================================================
// \title  SX1303Manager.hpp
// \author ninhdh4
// \brief  hpp file for SX1303Manager component implementation class
// ======================================================================

#ifndef SX1303_SX1303Manager_HPP
#define SX1303_SX1303Manager_HPP

#include "fprime-sx1303/Components/SX1303Manager/SX1303ManagerComponentAc.hpp"

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

    //! Handler implementation for command ReportNodeIdentifier
    //!
    //! Report the radio serial number
    void ReportNodeIdentifier_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                                         U32 cmdSeq            //!< The command sequence number
                                         ) override;
};

}  // namespace SX1303

#endif
