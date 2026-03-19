#pragma once
// ================================================================
//  Axion Digitaverse - platform.h
//  MSVC / Visual Studio compatibility header.
//  This is automatically pulled in by types.h which every
//  other file includes, so you never need to add it manually.
// ================================================================

#ifdef _WIN32
    // Define this before winsock2.h to avoid conflicts with old winsock
    // and to allow inclusion after other headers that might include winsock
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #ifndef _WINSOCKAPI_
        #include <winsock2.h>
        #include <ws2tcpip.h>
    #endif
#endif

#ifdef _MSC_VER
    // Prevent windows.h min/max macros from breaking std::min/std::max
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    // Suppress common MSVC warnings that are harmless in this project
    #pragma warning(disable: 4244)  // double -> int conversion (intentional in AVM)
    #pragma warning(disable: 4267)  // size_t -> int (container sizes)
    #pragma warning(disable: 4100)  // unreferenced parameter
    #pragma warning(disable: 4018)  // signed/unsigned compare
#endif

// Standard integer types - explicitly included for MSVC strictness
#include <cstdint>
#include <cstddef>
#include <cmath>
