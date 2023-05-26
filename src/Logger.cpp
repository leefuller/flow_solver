#include "Logger.h"

#define OUTPUT_FLUSH_POINTS     0

Logger & Logger::operator<< (Flags flag)
{
    if (is_null())
        return *this;
    switch (flag)
    {
    case L_time:
        logTime(); break;
    case L_flush:
        m_flags = static_cast<Flags>(m_flags & L_allwaysFlush);
#if OUTPUT_FLUSH_POINTS
        *this << " |F|\n";
#endif
        flush();
        break;
    case L_endl:
        {
#if OUTPUT_FLUSH_POINTS
        if (m_flags & L_allwaysFlush)
        { *this << " |F|"; }
        else if (m_flags == L_startWithFlushing)
        { *this << " |SF|"; }
#endif
        auto streamPtr = &stream();
        Logger* logger = this;
        do {
            *streamPtr << "\n";
            logger = logger->mirror_stream(streamPtr);
        } while (streamPtr);
        if (m_flags & L_allwaysFlush || m_flags == L_startWithFlushing)
            flush();
        }
        [[fallthrough]];
    case L_clearFlags:
        if (m_flags != L_startWithFlushing)
        {
            m_flags = static_cast<Flags>(m_flags & L_allwaysFlush);
        }
        break;
    case L_allwaysFlush:
        m_flags += L_allwaysFlush;
        break;
    case L_concat:
        removeFlag(L_tabs);
        break;
    default:
        addFlag(flag);
    }
    return *this;
}

Streamable& Logger::stream()
{ return null_ostream; }

tm* Logger::getTime()
{
    std::time_t now = std::time(nullptr);
    auto localTime = std::localtime(&now);
    log_date.dayNo = localTime->tm_mday;
    log_date.monthNo = localTime->tm_mon + 1;
    return localTime;
}

Logger& Logger::logTime ()
{
    *this << std::put_time(getTime(), "%d/%m/%y %H:%M:%S");
    m_flags += L_time;
    return *this;
}

// ----------------------------------------------------------

namespace logging {
    Logger& logger() {
        static Console_Logger std_log{};
        return std_log;
    }

    /*Logger& file1_logger() {
        static File_Logger logFile{ R"(C:\LF1_)", L_flush};
        return logFile;
    }*/
}

class NullLogger : public Logger
{
    public:
        NullLogger ()
          : Logger(L_null, null_ostream)
        {}
};

static NullLogger nullLogger;

/*void Logger::setDefaultLogger (Logger * logger)
{ m_defaultLogger = (logger == nullptr ? &nullLogger : logger); }*/

Logger & Logger::getDefaultLogger ()
{ return logging::logger(); }
//{ return nullLogger; }
