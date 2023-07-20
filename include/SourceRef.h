#ifndef INCLUDE_SOURCEREF_H
#define INCLUDE_SOURCEREF_H

#include <string>
#include <ostream>

/**
 * For a line number in a file.
 */
struct SourceRef
{
    const std::string file;
    const unsigned line;

    SourceRef (const std::string & str, unsigned l)
        : file(str), line(l)
    {}
};

inline std::ostream & operator<< (std::ostream & os, const SourceRef & src) noexcept
{
    os << src.file << ": " << src.line;
    return os;
}

// macro to create a SourceRef for the position where the macro is expanded
#define SOURCE_REF SourceRef(std::string(__FILE__), __LINE__)

#endif
