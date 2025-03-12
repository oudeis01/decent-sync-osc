cmake_minimum_required(VERSION 3.12)
project(motor_control CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable strict warnings for OSCPP
add_compile_options(-Wno-error=deprecated-copy -Wno-deprecated-copy)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/oscpp/include
)

# Disable OSCPP tests
set(OSCPP_BUILD_TESTS OFF CACHE BOOL "Disable tests")
add_subdirectory(oscpp EXCLUDE_FROM_ALL)

find_library(PIGPIO_LIBRARY pigpio)
if(NOT PIGPIO_LIBRARY)
    message(FATAL_ERROR "pigpio library not found!")
endif()

set(SOURCES
    main.cpp
    motor.cpp
    receiver.cpp
    sender.cpp
)

add_executable(motor_control ${SOURCES})

target_link_libraries(motor_control
    ${PIGPIO_LIBRARY}
    oscpp
    pthread
    rt
    m
)

target_compile_options(motor_control PRIVATE 
    -Wall
    -O2
    -Wno-unused-parameter
)