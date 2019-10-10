#include <mculib/device_selector.hpp>

#if defined(MCULIB_DEVICE_SUBFAMILY_STM32F1)
#include "usbserial_stm32f1.cpp.hpp"

#else
#error "usb serial library does not support the device selected"
#endif
