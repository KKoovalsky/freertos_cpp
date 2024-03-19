/**
 * @file	thread_config.cpp
 * @brief	Implements the configuration of the threads.
 */

#include "jungles_os_helpers/freertos/thread_config.hpp"

static freertos::ThreadConfig default_thread_config = {.name = "NoName", .stack_size = 1024, .priority = 1};

void freertos::set_default_thread_config(ThreadConfig config)
{
    default_thread_config = config;
}

freertos::ThreadConfig freertos::get_default_thread_config()
{
    return default_thread_config;
}
