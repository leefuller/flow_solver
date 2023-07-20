#ifndef PUZZLEEXCEPTION_H
#define PUZZLEEXCEPTION_H

#include <exception>
#include <string>
#include <vector>
#include <cstdarg>

#include "SourceRef.h"

class PuzzleException : public std::exception
{
    struct ExceptionContext
    {
        std::string msg;
        SourceRef ref;

        ExceptionContext (const std::string & s, const SourceRef & r)
            : msg(s), ref(r)
        {}
    };

public:
    PuzzleException (const SourceRef & ref, const char * fmt,...) noexcept;

    PuzzleException (const SourceRef & ref, const std::string & msg) noexcept
        : m_ref(ref), m_msg(msg)
    {}

    PuzzleException (const SourceRef & ref, const std::string && msg) noexcept
        : m_ref(ref), m_msg(std::move(msg))
    {}

	PuzzleException (const PuzzleException & ex) = default;

    const SourceRef & getSourceRef () const noexcept
    { return m_ref; }

	const std::string & getMessage () const noexcept
	{ return m_msg; }

	const char * what () const noexcept
	{ return m_msg.c_str(); }

	void addContext (const SourceRef & ref, const std::string & c) noexcept
	{ m_context.push_back(ExceptionContext(c, ref)); }

    void addContext (const SourceRef & ref, const char * p) noexcept
    { m_context.push_back(ExceptionContext(std::string(p), ref)); }

    std::ostream & outputContext (std::ostream & os) const noexcept
    {
        for (const ExceptionContext & context : m_context)
            os << " upon " << context.msg << " at " << context.ref;
        return os;
    }

protected:
    PuzzleException (const SourceRef & ref)
      : PuzzleException(ref, "PuzzleException")
    {}

    std::string buildString (const char * fmt, va_list & args) noexcept;

    void setMsg (const std::string & msg) noexcept
    { m_msg = msg; }

private:
    PuzzleException (const SourceRef & ref, const char * fmt, va_list & args) noexcept;

    SourceRef m_ref;
	std::string m_msg;
	std::vector<ExceptionContext> m_context;
};

inline std::ostream & operator<< (std::ostream & os, const PuzzleException & ex) noexcept
{
	os << ex.getMessage();
	ex.outputContext(os);
	return os;
}

class PuzzleIntegrityCheckFail : public PuzzleException
{
public:
    PuzzleIntegrityCheckFail (const SourceRef & ref, const char * msg) noexcept
        : PuzzleException(ref, msg)
    {}

    PuzzleIntegrityCheckFail (const SourceRef & ref) noexcept
        : PuzzleIntegrityCheckFail(ref, "integrity check failed")
    {}
};

#endif
