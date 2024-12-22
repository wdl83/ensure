#pragma once

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <unistd.h>
#undef _GNU_SOURCE

enum class TraceLevel
{
    begin,
    Error = begin,
    Warning,
    Info,
    Debug,
    Trace,
    end
};

inline
const char* toString(TraceLevel level)
{
    static const char *str[] = {"E", "W", "I", "D", "T"};
    static_assert(sizeof(str) / sizeof(str[0]) == int(TraceLevel::end) - int(TraceLevel::begin));
    return TraceLevel::end > level && TraceLevel::begin <= level ?  str[int(level)] : "?";
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
    static const char *env = ::getenv("TRACE_LEVEL");
    static const TraceLevel level =
        env ? static_cast<TraceLevel>(::atoi(env)) : TraceLevel::Info;
    return level;
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
        << oss.str() << std::endl;
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
