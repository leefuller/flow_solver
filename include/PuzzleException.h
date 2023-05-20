#ifndef PUZZLEEXCEPTION_H
#define PUZZLEEXCEPTION_H

#include <exception>
#include <string>
#include <vector>
#include <cstdarg>

class PuzzleException : public std::exception
{
public:
    PuzzleException (const char * fmt,...) noexcept;

    PuzzleException (const std::string & msg) noexcept
        : m_msg(msg)
    {}

    PuzzleException (const std::string && msg) noexcept
        : m_msg(std::move(msg))
    {}

	PuzzleException (const PuzzleException & ex) = default;

	const std::string & getMessage () const noexcept
	{ return m_msg; }

	const char * what () const noexcept
	{ return m_msg.c_str(); }

	void addContext (const std::string & c) noexcept
	{ m_context.push_back(c); }

    void addContext (const char * p) noexcept
    { m_context.push_back(std::string(p)); }

    std::ostream & outputContext (std::ostream & os) const noexcept
    {
        for (const std::string & context : m_context)
            os << context;
        return os;
    }

protected:
    PuzzleException ()
      : PuzzleException("PuzzleException")
    {}

    std::string buildString (const char * fmt, va_list & args) noexcept;

    void setMsg (const std::string & msg) noexcept
    { m_msg = msg; }

private:
    PuzzleException (const char * fmt, va_list & args) noexcept;

	std::string m_msg;
	std::vector<std::string> m_context;
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
    PuzzleIntegrityCheckFail (const char * msg) noexcept
        : PuzzleException(msg)
    {}

    PuzzleIntegrityCheckFail () noexcept
        : PuzzleIntegrityCheckFail("integrity check failed")
    {}
};

#endif
