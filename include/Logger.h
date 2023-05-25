#ifndef INCLUDED_LOGGER_H
#define INCLUDED_LOGGER_H

// Logger from Stephen Dolley
// https://www.cppstories.com/2021/stream-logger/

#include <iostream>
#include <iomanip>

enum Flags
{
    L_clearFlags,
    L_concat,
    L_time,
    L_flush,
    L_endl,
    L_startWithFlushing,
    L_null,
    L_cout = 8,
    L_tabs = 16,
    L_allwaysFlush = 32
};

inline Flags operator+= (Flags & l_flag, Flags r_flag)
{ return l_flag = static_cast<Flags>(l_flag | r_flag); }

inline Flags operator-= (Flags & l_flag, Flags r_flag)
{ return l_flag = static_cast<Flags>(l_flag & ~r_flag); }

using Streamable = std::ostream;

class Logger
{
  public:
    void activate(bool makeActive = true)
    { makeActive ? m_flags -= L_null : m_flags += L_null; }

    Flags addFlag (Flags flag)
    { return m_flags += flag; }

    Flags removeFlag (Flags flag)
    { return m_flags -= flag; }

    virtual void flush() {
        stream().flush();
        m_flags -= L_startWithFlushing;
    }

    virtual bool open()
    { return false; }

    /**
     * Stream any type that provides a stream operator.
     */
    template<typename T> Logger& log(T value);

    Logger& operator<< (Flags);

    Logger& operator<< (decltype(std::endl<char, std::char_traits<char>>))
    { return *this << L_endl; }

    Logger& operator<< (decltype(std::hex) manip)
    {
       stream() << manip;
       return *this;
    }

    Logger& operator<< (decltype(std::setw) manip)
    {
        stream() << manip;
        return *this;
    }

    /** @return the underlying ostream */
    virtual Streamable& stream();

    using ostreamPtr = Streamable*;

    /**
     * May be used by derived classes to mirror the log to another stream.
     */
    virtual Logger * mirror_stream (ostreamPtr& mirrorStream)
    {
        mirrorStream = nullptr;
        return this;
    }

    static void setDefaultLogger (Logger * logger);

    static Logger & getDefaultLogger ();

protected:
    /*
     * The base Logger class is of no use on its own (since it only provides null-logging),
     * so its constructors are protected, making Logger abstract.
     */

    Logger(Flags initFlag = L_null)
        : m_flags{ initFlag }
    {}

    Logger(Flags initFlag = L_null, Streamable& = std::clog) // TODO why does example do nothing with Streamable arg?
        : m_flags{ initFlag }
    {}

    virtual Logger& logTime();

    template<class T>
    friend Logger& operator <<(Logger& logger, T value);

    bool is_tabs() const { return m_flags & L_tabs || has_time(); }
    bool is_null() const { return m_flags == L_null; }
    bool is_cout() const { return m_flags & L_cout; }
    bool has_time() const { return (m_flags & 7) == L_time; }

    friend class FileNameGenerator;

    static tm* getTime();

    struct Log_date
    {
        unsigned char dayNo;
        unsigned char monthNo;
    } inline static log_date{ 0,0 };

    Flags m_flags = L_startWithFlushing;

  private:
    static Logger * m_defaultLogger;
};

template<typename T>
Logger& Logger::log(T value)
{
    if (is_null())
        return *this;
    auto streamPtr = &stream();
    Logger * logger = this;;
    do
    {
        if (is_tabs())
            *streamPtr << "\t";
        *streamPtr << value;
        logger = logger->mirror_stream(streamPtr);
    } while (streamPtr);
    removeFlag(L_time);
    return *this;
}

template<typename T>
Logger& operator<< (Logger& logger, T value)
{ return logger.log(value); }

class Null_Buff : public std::streambuf
{
// derive because std::streambuf constructor is protected
  public:
    Null_Buff()
    { setp(nullptr, nullptr); }

  private:
    int_type overflow(int_type ch) override
    { return std::char_traits<char>::not_eof(0); }
} inline null_buff{};

inline Streamable null_ostream{ &null_buff }; // TODO should be member of Logger class, allowing better handling for multiple threads


class Console_Logger : public Logger
{
    public:
        Console_Logger (std::ostream & os = std::cout)
            //: Logger(L_null, os) // and don't activate if you don't want logging immediately
            : Logger(L_allwaysFlush, os), m_ostream(os)
        { activate(true); }

        virtual Streamable& stream()
        { return m_ostream; }

        /**
         * Specialises mirror_stream() to return its _ostream the first time it is called,
         * and then return nullptr. TODO why?
         */
        virtual Logger * mirror_stream (ostreamPtr& mirrorStream)
        {
            if (!m_mirrored)
            {
                m_mirrored = true;
                mirrorStream = &m_ostream;
            }
            else
                mirrorStream = nullptr;
            return this;
        }

    private:
        std::ostream & m_ostream;
        bool m_mirrored{false};
};

#endif
