cmake_minimum_required(VERSION 3.12)
project(motor_control CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/oscpp/include
)

# Add OSCPP as subproject
add_subdirectory(oscpp)

# Find pigpio library
find_library(PIGPIO_LIBRARY pigpio)
if(NOT PIGPIO_LIBRARY)
    message(FATAL_ERROR "pigpio library not found!")
endif()

# Source files
set(SOURCES
    main.cpp
    motor.cpp
    receiver.cpp
    sender.cpp
)

# Add executable
add_executable(motor_control ${SOURCES})

# Link libraries
target_link_libraries(motor_control
    ${PIGPIO_LIBRARY}
    oscpp  # From the subdirectory
    pthread
    rt
    m
)

# Compiler flags
target_compile_options(motor_control PRIVATE 
    -Wall
    -O2
    -Wno-unused-parameter
)

# Installation (optional)
install(TARGETS motor_control RUNTIME DESTINATION /usr/local/bin)