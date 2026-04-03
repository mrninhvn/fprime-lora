####
# FPrime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding files
# MOD_DEPS: (optional) module dependencies
# UT_SOURCE_FILES: list of source files for unit tests
#
# More information in the F´ CMake API documentation:
# https://fprime.jpl.nasa.gov/latest/docs/user-manual/build-system/cmake-api/
#
####

set(libloragw_src
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/src/loragw_ad5338r.c"
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/src/loragw_aux.c"
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/src/loragw_cal.c"
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/src/loragw_com.c"
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/src/loragw_debug.c"
    # "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/src/loragw_gps.c"
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/src/loragw_hal.c"
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/src/loragw_i2c.c"
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/src/loragw_lbt.c"
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/src/loragw_reg.c"
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/src/loragw_spi.c"
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/src/loragw_stts751.c"
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/src/loragw_sx1250.c"
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/src/loragw_sx125x.c"
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/src/loragw_sx1261.c"
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/src/loragw_sx1302.c"
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/src/loragw_sx1302_rx.c"
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/src/loragw_sx1302_timestamp.c"
    # "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/src/loragw_usb.c"
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/src/sx1250_com.c"
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/src/sx1250_spi.c"
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/src/sx1250_usb.c"
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/src/sx125x_com.c"
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/src/sx125x_spi.c"
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/src/sx1261_com.c"
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/src/sx1261_spi.c"
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/src/sx1261_usb.c"
)

set(libtools_src
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libtools/src/base64.c"
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libtools/src/parson.c"
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libtools/src/tinymt32.c"
)

include_directories(
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libloragw/inc"
    "${CMAKE_CURRENT_LIST_DIR}/../SX1303Hal/libtools/inc"
)

register_fprime_module(
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/SX1303Manager.fpp"
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/SX1303Manager.cpp"
        ${libloragw_src}
        ${libtools_src}
    DEPENDS
        Fw_Types
        Utils_Types
)

set_source_files_properties(
    ${libloragw_src}
    ${libtools_src}
    PROPERTIES LANGUAGE C
)