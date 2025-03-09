# rpi-toolchain.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Toolchain paths (use Raspberry Pi's official toolchain)
set(TOOLCHAIN_PREFIX arm-linux-gnueabihf)
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)

# Sysroot (absolute path)
set(CMAKE_SYSROOT /home/choiharam/works/master2425/decent-sync-osc/rpi-sysroot)
set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})

# Linker flags (critical fix)
set(CMAKE_EXE_LINKER_FLAGS
  "-Wl,-rpath-link=${CMAKE_SYSROOT}/usr/lib/arm-linux-gnueabihf \
  -Wl,-rpath-link=${CMAKE_SYSROOT}/lib/arm-linux-gnueabihf"
)

# Compiler flags
set(CMAKE_C_FLAGS "--sysroot=${CMAKE_SYSROOT} -march=armv6zk -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard")
set(CMAKE_CXX_FLAGS "--sysroot=${CMAKE_SYSROOT} -march=armv6zk -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard")

# Search paths
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)