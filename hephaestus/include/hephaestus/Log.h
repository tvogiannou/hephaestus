#pragma once

// Global singleton utility for logging; can forward to custom callback

#include <hephaestus/Platform.h>


#ifdef HEPHAESTUS_DISABLE_LOGGER
    #define HEPHAESTUS_LOG_ERROR(format, ...) ((void)0)
    #define HEPHAESTUS_LOG_WARNING(format, ...) ((void)0)
    #define HEPHAESTUS_LOG_INFO(format, ...) ((void)0)
    #define HEPHAESTUS_LOG_ASSERT(expr, msg) (expr)
#elif defined(HEPHAESTUS_PLATFORM_ANDROID) // custom logger for Android
    #include <android/log.h>
    #define HEPHAESTUS_LOG_TAG "HEPHAESTUS_LOG"
    #define HEPHAESTUS_LOG_INFO(fmt, args...) __android_log_print(ANDROID_LOG_INFO, HEPHAESTUS_LOG_TAG, fmt, ##args)
    #define HEPHAESTUS_LOG_WARNING(fmt, args...) __android_log_print(ANDROID_LOG_WARN, HEPHAESTUS_LOG_TAG, fmt, ##args)
    #define HEPHAESTUS_LOG_ERROR(fmt, args...) __android_log_print(ANDROID_LOG_ERROR, HEPHAESTUS_LOG_TAG, fmt, ##args)
    #define HEPHAESTUS_LOG_ASSERT(expr, msg) if (!(expr)) { __android_log_assert(#expr, HEPHAESTUS_LOG_TAG, msg); }
#else
    #define HEPHAESTUS_LOG_ERROR(format, ...) ::hephaestus::Logger::Log(::hephaestus::Logger::eMSG_ERROR, format, ##__VA_ARGS__)
    #define HEPHAESTUS_LOG_WARNING(format, ...) ::hephaestus::Logger::Log(::hephaestus::Logger::eMSG_WARNING, format, ##__VA_ARGS__)
    #define HEPHAESTUS_LOG_INFO(format, ...) ::hephaestus::Logger::Log(::hephaestus::Logger::eMSG_INFO, format, ##__VA_ARGS__)
    #define HEPHAESTUS_LOG_ASSERT(expr, msg) if (!(expr)) { ::hephaestus::Logger::Log(::hephaestus::Logger::eMSG_ERROR, msg); HEPHAESTUS_ASSERT(false); }
#endif


namespace hephaestus
{
// Simple stateless logger, handles formatting and forwards messages to callback
// Default backed by std io (printf)
class Logger
{
public:

    // Hints for output messages
    enum MessageType : uint32_t
    {
        eMSG_ERROR = 0,
        eMSG_WARNING = 1,
        eMSG_INFO = 2,

        eMESSAGE_TYPE_MAX
    };
    
    // customizable callback (stateless)
    // Does not support formating for simplification, formatting is handled by the Logger
    using LogCallbackType = void (*)(const char*, MessageType);
    static void SetCallback(LogCallbackType _callback);

    static void Log(MessageType msgType, const char* format, ...);

    // internal buffer size used by vnprintf, mostly for testing but could be relevant in other cases
    static constexpr uint32_t LOCAL_BUFFER_SIZE = 1024u;

private:

    static LogCallbackType m_logCallback;
};

}