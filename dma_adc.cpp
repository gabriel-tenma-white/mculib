#include <mculib/device_selector.hpp>

#if defined(MCULIB_DEVICE_SUBFAMILY_STM32F1)
#include "dma_adc_stm32f1.cpp.hpp"

#elif defined(MCULIB_DEVICE_SUBFAMILY_STM32F3)
#include "dma_adc_stm32f3.cpp.hpp"

#elif defined(MCULIB_DEVICE_SUBFAMILY_STM32F0)
#include "dma_adc_stm32f0.cpp.hpp"

#else
#error "DMA ADC library does not support selected device family"

#endif
