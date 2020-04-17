#pragma once

#include <hephaestus/Compiler.h>

// utility header for configuring the current platform

// platform specific configuration
#ifdef HEPHAESTUS_PLATFORM_WIN32
    #include <Windows.h>

#elif defined(HEPHAESTUS_PLATFORM_LINUX)
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
    #include <dlfcn.h>
    #include <cstdlib>

#elif defined(HEPHAESTUS_PLATFORM_ANDROID)
    #include <android/native_window.h>
    #include <dlfcn.h>

#else
    static_assert(false, "Unsupported platform");
#endif