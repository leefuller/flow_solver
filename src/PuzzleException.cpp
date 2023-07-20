#include "PuzzleException.h"

#include <cstdarg>
#include <sstream>

std::string PuzzleException::buildString (const char * fmt, va_list & args) noexcept
{
    std::ostringstream ss;

    while (*fmt != '\0')
    {
        if (*fmt == '%')
        {
            ++fmt;
            if (*fmt == '\0')
                break;
            if (*fmt == 'd')
                ss << va_arg(args, int);
            if (*fmt == 'u')
                ss << va_arg(args, unsigned int);
            else if (*fmt == 'c')
                ss << static_cast<char>(va_arg(args, int));
            else if (*fmt == 'f')
                ss << va_arg(args, double);
            else if (*fmt == 's')
                ss << va_arg(args, const char*);
            else
                ss << *fmt;
        }
        else
            ss << *fmt;
        ++fmt;
    }
    return ss.str();
}

PuzzleException::PuzzleException (const SourceRef & ref, const char * fmt, va_list & args) noexcept
    : m_ref(ref), m_msg(buildString(fmt, args))
{}

/**
 * Exception message created as in C printf arguments
 */
PuzzleException::PuzzleException (const SourceRef & ref, const char * fmt,...) noexcept
    : m_ref(ref)
{
    va_list args;
    va_start(args, fmt);
    m_msg = buildString(fmt, args);
    va_end(args);
}
