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

# pigpio detection
find_library(PIGPIO_LIB pigpio)
if(PIGPIO_LIB)
    message(STATUS "Found pigpio: ${PIGPIO_LIB}")
    set(USE_PIGPIO TRUE)
else()
    message(WARNING "pigpio library not found - using mock mode")
    set(USE_PIGPIO FALSE)
endif()

# Executable setup
add_executable(DecentralizedSynchronization
    main.cpp
    motor.cpp
    receiver.cpp
)

# Link libraries
target_link_libraries(DecentralizedSynchronization PRIVATE
    pthread
)

if(USE_PIGPIO)
    target_link_libraries(DecentralizedSynchronization PRIVATE
        ${PIGPIO_LIB}
        rt  # Required for pigpio
    )
    target_compile_definitions(DecentralizedSynchronization PRIVATE PI_ZERO)
endif()