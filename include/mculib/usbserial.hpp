#pragma once
#include <mculib/device_selector.hpp>

#if defined(MCULIB_DEVICE_SUBFAMILY_STM32F1)
#include <mculib/usbserial_stm32f1.hpp>

#else
#error "usb serial library does not support the device selected"
#endif
