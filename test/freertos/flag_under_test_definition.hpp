/**
 * @file	test_flag_impl.cpp
 * @brief	Defines the implementation for flag for the generic test.
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#include <memory>

#include "jungles_os_helpers/freertos/flag.hpp"

jungles::freertos::flag get_flag_implementation_under_test()
{
    return jungles::freertos::flag{};
}
