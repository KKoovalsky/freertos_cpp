/**
 * @file        thread_config.hpp
 * @brief       Configuration for the thread class
 */
#ifndef THREAD_CONFIG_HPP
#define THREAD_CONFIG_HPP

#include <algorithm>
#include <string>

namespace freertos
{

struct StackSize
{
    const unsigned value;
};

struct ThreadPriority
{
    const int value;
};

template<std::size_t N>
struct ThreadName
{
    constexpr ThreadName(const char (&str)[N])
    {
        std::copy_n(str, N, value);
    }

    char value[N];
};

struct ThreadConfig
{
    const char* name;
    unsigned stack_size;
    int priority;
};

void set_default_thread_config(ThreadConfig);
ThreadConfig get_default_thread_config();

} // namespace freertos

#endif /* THREAD_CONFIG_HPP */
