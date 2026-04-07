// ======================================================================
// \title  SubtopologyTopologyDefs.hpp
// \brief subtopology definitions header
//
// ======================================================================
#ifndef SX1303_SubtopologyTopologyDefs_hpp
#define SX1303_SubtopologyTopologyDefs_hpp

#include <Fw/Logger/Logger.hpp>
// #include <fprime-sx1303/Subtopology/SX1303Config/FppConstantsAc.hpp>
#include "SX1303Config/SX1303SubtopologyConfig.hpp"

namespace SX1303 {
    struct SubtopologyState {
        SX1303Device device;
    };

    struct TopologyState {
        SubtopologyState sx1303;
    };
}  // namespace SX1303
#endif  // SX1303_SubtopologyTopologyDefs_hpp