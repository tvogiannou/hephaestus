
#include <hephaestus/Log.h>

#include <cstdio>   // for fprintf & printf
#include <cstdarg>  // for var args


namespace hephaestus
{
// default log callback using std io (printf)
static
void s_LogStdIO(const char* message, Logger::MessageType msgType)
{
    constexpr const char* typeToString[] =
    {
        "ERROR",
        "WARNING",
        "HINT"
    };

    static_assert(Logger::MessageType::eMESSAGE_TYPE_MAX == 3u /*countof(typeToString)*/,
        "Logger::LogIOStream() mismatch local array length with Message Type count");

    if (msgType == Logger::MessageType::eMSG_ERROR)
        std::fprintf(stderr, "LOG %s: %s\n", typeToString[(uint32_t)msgType], message);
    else
        std::printf("LOG %s: %s\n", typeToString[(uint32_t)msgType], message);
}

Logger::LogCallbackType Logger::m_logCallback = s_LogStdIO; // default callback

void 
Logger::SetCallback(LogCallbackType _callback)
{
    m_logCallback = _callback;
}

void 
Logger::Log(MessageType msgType, const char* format, ...)
{
    if (m_logCallback)
    {
        constexpr uint32_t bufferSize = LOCAL_BUFFER_SIZE;
        char buffer[bufferSize];

        // TODO: investigate alternatives/improvements
        va_list args;
        va_start(args, format);
        int r = std::vsnprintf(buffer, bufferSize, format, args);
        va_end(args);

        // forward to the callback
        if (m_logCallback) // not sure if we should we check again here...
        {

            // log errors with the logger!
            if (r > 0)
            {
                if ((uint32_t)r > bufferSize)
                    m_logCallback("Logger::Log - input message too large for local buffer", MessageType::eMSG_ERROR);
                else
                    m_logCallback(buffer, msgType);
            }
            else
                m_logCallback("Logger::Log - Failed to format input message", MessageType::eMSG_ERROR);
        }
    }
}

}