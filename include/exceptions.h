#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>
#include "PuzzleException.h"

class InvalidOperation : public PuzzleException
{
    public:
        //InvalidOperation (const char * fmt,...) noexcept;

        InvalidOperation (const SourceRef & ref, const std::string & msg) noexcept
            : PuzzleException(ref, msg)
        {}

        InvalidOperation (const SourceRef & ref, const std::string && msg) noexcept
            : PuzzleException(ref, msg)
        {}

        InvalidOperation (const InvalidOperation & ex) = default;
};

#endif
