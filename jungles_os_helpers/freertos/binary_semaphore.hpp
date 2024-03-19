/**
 * @file        binary_semaphore.hpp
 * @brief       Implements a binary semaphore that works with FreeRTOS
 */
#ifndef BINARY_SEMAPHORE_HPP
#define BINARY_SEMAPHORE_HPP

#include "FreeRTOS.h"
#include "semphr.h"

namespace freertos
{

// TODO: Introduce pimpl and do not include "FreeRTOS.h" and "semphr.h" in the header file.
class binary_semaphore
{
  public:
    binary_semaphore() noexcept : sem{xSemaphoreCreateBinaryStatic(&storage)}
    {
    }

    void give() noexcept
    {
        xSemaphoreGive(sem);
    }

    void take() noexcept
    {
        xSemaphoreTake(sem, portMAX_DELAY);
    }

    bool try_take() noexcept
    {
        return xSemaphoreTake(sem, 0) == pdTRUE;
    }

    ~binary_semaphore()
    {
        vSemaphoreDelete(sem);
    }

  private:
    StaticSemaphore_t storage = {};
    SemaphoreHandle_t sem;
};

} // namespace freertos

#endif /* BINARY_SEMAPHORE_HPP */
