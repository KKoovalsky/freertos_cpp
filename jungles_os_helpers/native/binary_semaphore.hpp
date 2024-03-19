/**
 * @file        binary_semaphore.hpp
 * @brief       Implements a binary semaphore that works with native std:: .
 */
#ifndef BINARY_SEMAPHORE_NATIVE_HPP
#define BINARY_SEMAPHORE_NATIVE_HPP

#include <semaphore>

namespace os::native
{

class binary_semaphore
{
  public:
    void give()
    {
        sem.release();
    }

    void take()
    {
        sem.acquire();
    }

    bool try_take()
    {
        return sem.try_acquire();
    }

  private:
    std::binary_semaphore sem{0};
};

} // namespace os::native

#endif /* BINARY_SEMAPHORE_NATIVE_HPP */
