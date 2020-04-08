#pragma once

// header for configuring the current platform

// platform
// TODO: iOS, MacOS
#if defined(ANDROID) || defined(__ANDROID__)    // check for Android before any other platform since
                                                // in case of cross compilation both platforms are defined
    #define HEPHAESTUS_PLATFORM_ANDROID 1
#elif defined(_WIN32)
    #define HEPHAESTUS_PLATFORM_WIN32 1
#elif defined(__linux__)
    #define HEPHAESTUS_PLATFORM_LINUX 1
#endif


