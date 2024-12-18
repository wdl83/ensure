#pragma once

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <unistd.h>

enum class TraceLevel
{
    begin,
    Error = begin,
    Warning,
    Info,
    Debug,
    end
};

inline
const char* toString(TraceLevel level)
{
    const char *str[] = {"E", "W", "I", "D"};
    return
        TraceLevel::end > level && TraceLevel::begin <= level
        ?  str[int(level)] : "U";
}

using LogLevel = TraceLevel;

namespace impl_ {

template <typename T>
void traceSerializeImpl(std::ostream &os, const T &value)
{
    os << value;
}

inline
void traceImpl(std::ostream &)
{}

template <typename T, typename ...T_n>
void traceImpl(std::ostream &os, const T &value, const T_n &...tail)
{
    traceSerializeImpl(os, value);
    traceImpl(os, tail...);
}

inline
TraceLevel currTraceLevel()
{
    static const TraceLevel traceLevel =
        ::getenv("TRACE_LEVEL")
        ? static_cast<TraceLevel>(::atoi(::getenv("TRACE_LEVEL")))
        : TraceLevel::Info;
    return traceLevel;
}

} /* impl_ */


inline
void trace(TraceLevel traceLevel, const std::ostringstream &oss)
{
    if(int(impl_::currTraceLevel()) < int(traceLevel)) return;

    std::ostream &dst =
        TraceLevel::Error == traceLevel || TraceLevel::Warning == traceLevel
        ? std::cerr : std::cout;
    dst << '['
        << ::getpid() << '|'
        << ::gettid() << '|'
        << toString(traceLevel) << "] "
        << oss.str() << '\n';
}

template <typename ...T_n>
void trace(TraceLevel traceLevel, const T_n &...tail)
{
    if(int(impl_::currTraceLevel()) < int(traceLevel)) return;

    std::ostringstream ss;

    impl_::traceImpl(ss, tail...);
    trace(traceLevel, ss);
}


#ifndef ENABLE_TRACE

#define TRACE(...)

#else

#define TRACE(traceLevel, ...) \
    do \
    { \
        trace(traceLevel, __FILE__, ':', __LINE__, ' ', __FUNCTION__, ' ', __VA_ARGS__); \
    } while(false)

#endif /* ENABLE_TRACE */


#ifndef ENABLE_LOG

#define LOG(...)

#else

#define LOG(logLevel, ...) \
    do \
    { \
        trace(logLevel, __VA_ARGS__); \
    } while(false)

#endif /* ENABLE_TRACE */
