add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/SX1303Config")

register_fprime_module(
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/Subtopology.fpp"
    EXCLUDE_FROM_ALL
    DEPENDS
        fprime-sx1303_Subtopology_SX1303Config
)