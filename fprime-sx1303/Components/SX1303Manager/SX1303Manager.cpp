// ======================================================================
// \title  SX1303Manager.cpp
// \author ninhdh4
// \brief  cpp file for SX1303Manager component implementation class
// ======================================================================

#include "fprime-sx1303/Components/SX1303Manager/SX1303Manager.hpp"

namespace SX1303 {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

SX1303Manager ::SX1303Manager(const char* const compName) : SX1303ManagerComponentBase(compName) {}

SX1303Manager ::~SX1303Manager() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void SX1303Manager ::run_handler(FwIndexType portNum, U32 context) {
    // TODO
}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

void SX1303Manager ::ReportNodeIdentifier_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    // TODO
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace SX1303
