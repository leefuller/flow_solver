#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>
#include "PuzzleException.h"

class InvalidOperation : public PuzzleException
{
    public:
        //InvalidOperation (const char * fmt,...) noexcept;

        InvalidOperation (const std::string & msg) noexcept
            : PuzzleException(msg)
        {}

        InvalidOperation (const std::string && msg) noexcept
            : PuzzleException(msg)
        {}

        InvalidOperation (const InvalidOperation & ex) = default;
};

#endif
