/**
 * @file	queue.hpp
 * @brief	Implements queue basing on the interface in queue.hpp for FreeRTOS.
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#ifndef FREERTOS_QUEUE_HPP
#define FREERTOS_QUEUE_HPP

#include "FreeRTOS.h"
#include "projdefs.h"
#include "queue.h"
#include "semphr.h"

#include "lockguard.hpp"

#include <array>
#include <exception>
#include <optional>
#include <vector>

namespace jungles
{

namespace freertos
{

// TODO: Templated Size is uneeded here.
template<typename ElementType, std::size_t Size>
class queue
{
  public:
    static_assert(Size > 1, "This implementation will not work with size smaller than 2");

    queue(const queue&) = delete;
    queue& operator=(const queue&) = delete;
    queue(queue&&) = delete;
    queue& operator=(queue&&) = delete;

    queue() :
        queue_depot_mux{xSemaphoreCreateMutexStatic(&queue_depot_mux_storage)},
        num_elements_counting_sem{xSemaphoreCreateCountingStatic(Size, 0, &num_elements_counting_sem_storage)}
    {
        assert(queue_depot_mux != nullptr);
        assert(num_elements_counting_sem != nullptr);
    }

    ~queue()
    {
        vSemaphoreDelete(queue_depot_mux);
        vSemaphoreDelete(num_elements_counting_sem);
    }

    void send(ElementType&& elem)
    {
        insert(std::move(elem));

        auto r{xSemaphoreGive(num_elements_counting_sem)};
        assert(r == pdTRUE);
    }

    void send_from_isr(ElementType&& elem)
    {
        insert_lock_free(std::move(elem));

        BaseType_t higher_priority_task_woken{pdFALSE};
        [[maybe_unused]] auto r{xSemaphoreGiveFromISR(num_elements_counting_sem, &higher_priority_task_woken)};
        assert(r == pdTRUE);

        portYIELD_FROM_ISR(higher_priority_task_woken);
    }

    ElementType receive()
    {
        return *receive_impl(portMAX_DELAY);
    }

    std::optional<ElementType> receive(unsigned timeout_ms)
    {
        return receive_impl(pdMS_TO_TICKS(timeout_ms));
    }

    struct error : public std::exception
    {
    };

    struct queue_full_error : public error
    {
    };

  private:
    void insert(ElementType&& elem)
    {
        lockguard g(queue_depot_mux);
        insert_lock_free(std::move(elem));
    }

    void insert_lock_free(ElementType&& elem)
    {
        auto is_full{queue_depot_elem_count == Size};
        if (is_full)
            throw queue_full_error{};

        queue_depot[queue_depot_head] = std::move(elem);
        increment_circular_buffer_index(queue_depot_head);
        ++queue_depot_elem_count;
    }

    void increment_circular_buffer_index(unsigned& index)
    {
        index = (index + 1) % Size;
    }

    std::optional<ElementType> receive_impl(TickType_t timeout)
    {
        if (xSemaphoreTake(num_elements_counting_sem, timeout) == pdTRUE)
        {
            lockguard g(queue_depot_mux);
            auto r{std::move(queue_depot[queue_depot_tail])};
            increment_circular_buffer_index(queue_depot_tail);
            --queue_depot_elem_count;
            return r;
        } else
        {
            return {};
        }
    }

    //! @todo Shall be circular buffer.
    std::array<ElementType, Size> queue_depot;
    unsigned queue_depot_tail{0}, queue_depot_head{0}, queue_depot_elem_count{0};
    SemaphoreHandle_t queue_depot_mux;
    SemaphoreHandle_t num_elements_counting_sem;
    StaticSemaphore_t queue_depot_mux_storage;
    StaticSemaphore_t num_elements_counting_sem_storage;
};

} // namespace freertos

} // namespace jungles

#endif /* FREERTOS_QUEUE_HPP */
