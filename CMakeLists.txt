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

find_library(WIRINGPI_LIB wiringPi 
  HINTS ${CMAKE_SYSROOT}/usr/lib
)
include_directories(${CMAKE_SYSROOT}/usr/include)

add_executable(DecentralizedSynchronization main.cpp motor.cpp receiver.cpp)
include_directories(${CMAKE_SOURCE_DIR})
target_link_libraries(DecentralizedSynchronization PRIVATE pthread osc++ ${WIRINGPI_LIB})