// ======================================================================
// \title  SX1303TestRadio.hpp
// \author ninhdh4
// \brief  hpp file for SX1303TestRadio funtions
// ======================================================================

#ifndef SX1303_SX1303TestRadio_HPP
#define SX1303_SX1303TestRadio_HPP

extern "C" {
    int sx1303_test_hal_tx(void);
    int sx1303_test_hal_rx(void);
}

#endif