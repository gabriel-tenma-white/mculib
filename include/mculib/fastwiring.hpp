#pragma once
#include <mculib/device_selector.hpp>

#if defined(MCULIB_DEVICE_SUBFAMILY_STM32F0)
#include <mculib/fastwiring_stm32f0.hpp>

#elif defined(MCULIB_DEVICE_SUBFAMILY_STM32F1)
#include <mculib/fastwiring_stm32f1.hpp>

#elif defined(MCULIB_DEVICE_SUBFAMILY_STM32F3)
#include <mculib/fastwiring_stm32f3.hpp>

#else
#error "fastwiring does not support the device selected"
#endif
