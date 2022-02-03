option(ENABLE_BLAKE3 "Enable BLAKE3" ${ENABLE_LIBRARIES})

if (NOT ENABLE_BLAKE3)
    message(STATUS "Not using BLAKE3")
    return()
endif()

include(build_rust_lib.cmake)

build_cargo(blake3 ${CMAKE_CURRENT_SOURCE_DIR})
