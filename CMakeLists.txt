cmake_minimum_required(VERSION 3.14)
project(DecentralizedSynchronization)

# Compiler options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/oscpp/include  # OSCPP headers
    ${CMAKE_SOURCE_DIR}               # Project headers
)

# WiringPi detection
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND EXISTS "/etc/os-release")
    file(READ "/etc/os-release" OS_RELEASE_CONTENTS)
    if(OS_RELEASE_CONTENTS MATCHES "Arch")
        message(STATUS "Detected Arch Linux, skipping WiringPi")
        set(USE_WIRINGPI FALSE)
    else()
        # Try to find WiringPi
        set(CMAKE_PREFIX_PATH "/usr/local")
        find_library(WIRINGPI_LIB wiringPi
            HINTS ${CMAKE_SYSROOT}/usr/lib
            PATH_SUFFIXES lib
        )
        if(WIRINGPI_LIB)
            message(STATUS "Found WiringPi: ${WIRINGPI_LIB}")
            include_directories(${CMAKE_SYSROOT}/usr/include)
            set(USE_WIRINGPI TRUE)
        else()
            message(WARNING "WiringPi library not found - using mock mode")
            set(USE_WIRINGPI FALSE)
        endif()
    endif()
endif()

# Executable setup
add_executable(DecentralizedSynchronization
    main.cpp
    motor.cpp
    receiver.cpp
)

# Link libraries
if(USE_WIRINGPI)
    target_link_libraries(DecentralizedSynchronization PRIVATE
        pthread
        ${WIRINGPI_LIB}
    )
else()
    target_link_libraries(DecentralizedSynchronization PRIVATE
        pthread
    )
endif()

# Add PI_ZERO definition if needed
if(USE_WIRINGPI)
    target_compile_definitions(DecentralizedSynchronization PRIVATE PI_ZERO)
endif()