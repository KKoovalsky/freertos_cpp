/**
 * @file	simple_thread.hpp
 * @brief	Implements a simple_thread class run on top of FreeRTOS
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#ifndef FREERTOS_THREAD_IMPL_HPP
#define FREERTOS_THREAD_IMPL_HPP

#include "FreeRTOS.h"
#include "task.h"

#include "binary_semaphore.hpp"
#include "thread_config.hpp"

#include <cassert>
#include <functional>
#include <string>

#ifdef FREERTOS_EXCEPTIONS
#include <stdexcept>
#endif

namespace freertos
{

// TODO: This class can be implemented within a source file

//! Thread that can be started and joined, and is not detachable.
class simple_thread
{
  public:
    enum class Error
    {
        ok = 0,
        already_running,
        invalid_thread_code,
        thread_start_failed,
        not_started
    };

#ifdef FREERTOS_EXCEPTIONS
    struct Exception : std::runtime_error
    {
        Exception(Error e) : std::runtime_error{error_msgs[static_cast<unsigned>(e)]}
        {
        }

        static inline constexpr const char* error_msgs[] = {"OK",
                                                            "The thread is already running",
                                                            "The thread code is not set",
                                                            "Failed to create the thread",
                                                            "The thread is not started"};
    };
#endif // FREERTOS_EXCEPTIONS

    using ThreadCode = std::function<void(void)>;

    //! Default constructor uses global setting for the thread configuration.
    simple_thread()
    {
        auto config{get_default_thread_config()};
        name = config.name;
        stack_size = config.stack_size;
        priority = config.priority;
    }

    simple_thread(std::string name, StackSize stack_sz, ThreadPriority prio) :
        name{std::move(name)}, stack_size{stack_sz.value}, priority{prio.value}
    {
    }

    simple_thread(const simple_thread&) = delete;
    simple_thread& operator=(const simple_thread&) = delete;
    simple_thread(simple_thread&&) = delete;
    simple_thread& operator=(simple_thread&&) = delete;

#ifdef FREERTOS_EXCEPTIONS
    void start(ThreadCode thread_code)
    {
        auto error{do_start(std::move(thread_code))};
        if (error != Error::ok)
            throw Exception{error};
    }

    void join()
    {
        auto error{do_join()};
        if (error != Error::ok)
            throw Exception{error};
    }
#else
    Error start(ThreadCode thread_code)
    {
        return do_start(std::move(thread_code));
    }

    Error join()
    {
        return do_join();
    }
#endif

  private:
    Error do_start(ThreadCode&& thread_code)
    {
        auto is_not_started{task_started_sem.try_take() == true};
        if (auto is_started{not is_not_started}; is_started)
            return Error::already_running;

        if (not thread_code)
            return Error::invalid_thread_code;

        this->thread_code = std::move(thread_code);
        auto result{xTaskCreate(task_fun,
                                name.c_str(),
                                stack_size,
                                this, // Forward self to the task code.
                                priority,
                                nullptr)};
        if (result != pdPASS)
            return Error::thread_start_failed;
        return Error::ok;
    }

    //! @note Trying to join twice in a row is an error
    Error do_join()
    {
        auto is_not_started{task_started_sem.try_take() == true};
        if (is_not_started)
        {
            task_started_sem.give();
            return Error::not_started;
        }

        wait_task_finished();
        task_started_sem.give();
        return Error::ok;
    }

    static inline void task_fun(void* raw_pointer_to_simple_thread)
    {
        auto self{static_cast<simple_thread*>(raw_pointer_to_simple_thread)};
        self->thread_code();
        self->notify_task_finished();
        vTaskDelete(nullptr);
    }

    void notify_task_finished()
    {
        task_finished_sem.give();
    }

    void wait_task_finished()
    {
        task_finished_sem.take();
    }

    freertos::binary_semaphore task_finished_sem, task_started_sem;
    std::string name;
    unsigned stack_size;
    int priority;
    ThreadCode thread_code;
};

template<ThreadName Name, StackSize StackSize_, ThreadPriority Prio>
struct simple_tread_template : simple_thread
{
    simple_tread_template() : simple_thread(Name.value, StackSize_, Prio)
    {
    }
};

} // namespace freertos

#endif /* FREERTOS_THREAD_IMPL_HPP */
