cmake_minimum_required(VERSION 3.14)

project(DecentralizedSynchronization)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set the build type to Debug to include debugging symbols
set(CMAKE_BUILD_TYPE Debug)

include(FetchContent)

FetchContent_Declare(
  osc
  GIT_REPOSITORY https://github.com/dimitry-ishenko-cpp/osc.git
  GIT_TAG v1.3
)
FetchContent_MakeAvailable(osc)
include_directories(${osc_SOURCE_DIR}/include)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND EXISTS "/etc/os-release")
    file(READ "/etc/os-release" OS_RELEASE_CONTENTS)
    if(OS_RELEASE_CONTENTS MATCHES "Arch")
        message(STATUS "Detected Arch Linux, not using WiringPi")
    else()
        find_library(WIRINGPI_LIB wiringPi HINTS ${CMAKE_SYSROOT}/usr/lib)
        if(WIRINGPI_LIB)
            include_directories(${CMAKE_SYSROOT}/usr/include)
            set(USE_WIRINGPI TRUE)
        else()
            message(WARNING "WiringPi library not found, proceeding without it")
            set(USE_WIRINGPI FALSE)
        endif()
    endif()
endif()

add_executable(DecentralizedSynchronization main.cpp motor.cpp receiver.cpp)
include_directories(${CMAKE_SOURCE_DIR})

if(USE_WIRINGPI)
    target_link_libraries(DecentralizedSynchronization PRIVATE pthread osc++ ${WIRINGPI_LIB})
else()
    target_link_libraries(DecentralizedSynchronization PRIVATE pthread osc++)
endif()