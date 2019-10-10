#include <mculib/device_selector.hpp>

#if defined(MCULIB_DEVICE_FAMILY_STM32)
#include "dma_driver_stm32.cpp.hpp"

#else
#error "DMA library does not support selected device family"

#endif
