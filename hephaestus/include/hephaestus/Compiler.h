#pragma once

// helpers and convenience include for compilation

#include <hephaestus/Platform.h>

// compiler
#if defined(_MSC_VER)
    #define HEPHAESTUS_COMPILER_MSVC 1
#elif defined(__clang__) // NOTE: This NEEDS to be checked before __GNUC__ as clang defines both
    #define HEPHAESTUS_COMPILER_CLANG 1
#elif defined(__GNUC__) || defined(__GNUG__)
    #define HEPHAESTUS_COMPILER_GCC 1
#endif

// forward header with standard types
#include <cstdint>
#include <float.h>
#ifdef __GLIBCXX__
    using std::size_t; // GNU stdlib++ defines size_t in std (as per standard)
#endif

// helpers
// very simple, use with caution!
#define HEPHAESTUS_STRINGIFYX(a) HEPHAESTUS_STRINGIFY(a)      // stringify with macro expansion
#define HEPHAESTUS_STRINGIFY(a) #a
#define HEPHAESTUS_MACRO_ON 1
#define HEPHAESTUS_MACRO_OFF 0
#define HEPHAESTUS_UNUSED(a) (void)(a);

// build configuration
// TODO: release, optimized, final, etc
#ifdef HEPHAESTUS_PLATFORM_WIN32
    #ifdef _DEBUG
        #define HEPHAESTUS_DEBUG_BUILD 1
    #endif
#else // default general case based on NDEBUG
    #ifndef NDEBUG
        #define HEPHAESTUS_DEBUG_BUILD 1
    #endif
#endif

// Adapter for assert, mainly for more convenient use with the VC debugger 
#ifdef HEPHAESTUS_DEBUG_BUILD
    #if defined(HEPHAESTUS_PLATFORM_WIN32)
        #include <CRTDBG.h>
        #include <cstdlib>
        #define HEPHAESTUS_ASSERT(expr) if(!(expr)) { _CrtDbgBreak(); std::abort(); } // breakpoint for easier debugging
    #else
        #include <cassert>
        #define HEPHAESTUS_ASSERT(expr) assert(expr)
    #endif
#else
    #define HEPHAESTUS_ASSERT(expr) HEPHAESTUS_UNUSED(expr)
#endif