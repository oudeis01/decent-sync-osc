#pragma once

#if defined(__x86_64__) || defined(_M_X64)
    #define ARCH

#elif defined(__aarch64__) || defined(__arm__) // ARM-based
    #ifndef PI_ZERO
        #define PI_ZERO
    #endif
#endif