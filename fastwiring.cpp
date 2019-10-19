
#include <mculib/device_selector.hpp>

#if defined(MCULIB_DEVICE_FAMILY_STM32)
#include "fastwiring_stm32.cpp.hpp"

#else
#error "fastwiring library does not support selected device family"

#endif
