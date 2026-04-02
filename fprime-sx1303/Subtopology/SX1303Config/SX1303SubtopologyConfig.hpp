// ======================================================================
// \title  SX1303SubtopologyConfig.hpp
// \brief required header file containing the required definitions for the subtopology autocoder
//
// ======================================================================
#ifndef SX1303_SX1303SubtopologyConfig_hpp
#define SX1303_SX1303SubtopologyConfig_hpp

struct SX1303Device {
    int device;  // SPI bus number (e.g., 0 for SPI bus 0)
    int select;  // SPI chip select pin (e.g., 0 for CS0)
};

#endif // SX1303_SX1303SubtopologyConfig_hpp