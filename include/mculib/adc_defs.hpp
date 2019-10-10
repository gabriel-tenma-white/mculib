#pragma once
#include <mculib/device_selector.hpp>

#if defined(MCULIB_DEVICE_FAMILY_STM32)
#include <mculib/adc_defs_stm32.hpp>

#else
#error "ADC library does not support selected device family"

#endif
