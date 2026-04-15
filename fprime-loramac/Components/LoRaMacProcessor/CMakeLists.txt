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

set(libloramac_src
    "${CMAKE_CURRENT_LIST_DIR}/../LoRaMac/src/mac/region/Region.c"
    "${CMAKE_CURRENT_LIST_DIR}/../LoRaMac/src/mac/region/RegionCommon.c"
    "${CMAKE_CURRENT_LIST_DIR}/../LoRaMac/src/mac/region/RegionAS923.c"
    "${CMAKE_CURRENT_LIST_DIR}/../LoRaMac/src/mac/LoRaMac.c"
    "${CMAKE_CURRENT_LIST_DIR}/../LoRaMac/src/mac/LoRaMacParser.c"
    "${CMAKE_CURRENT_LIST_DIR}/../LoRaMac/src/boards/mcu/utilities.c"
)

include_directories(
    "${CMAKE_CURRENT_LIST_DIR}/../LoRaMac/src/region"
    "${CMAKE_CURRENT_LIST_DIR}/../LoRaMac/src/mac"
    "${CMAKE_CURRENT_LIST_DIR}/../LoRaMac/src/board"
)

register_fprime_module(
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/LoRaMacProcessor.fpp"
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/LoRaMacProcessor.cpp"
        ${libloramac_src}
)

set_source_files_properties(
    ${libloramac_src}
    PROPERTIES LANGUAGE C
)
