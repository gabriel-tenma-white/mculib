#pragma once
// sets MCULIB_DEVICE_FAMILY_xx and MCULIB_DEVICE_SUBFAMILY_xxx based on MCULIB_DEVICE_xxx

#if defined(MCULIB_DEVICE_STM32F030)
#define MCULIB_DEVICE_FAMILY_STM32
#define MCULIB_DEVICE_SUBFAMILY_STM32F0

#elif defined(MCULIB_DEVICE_STM32F072)
#define MCULIB_DEVICE_FAMILY_STM32
#define MCULIB_DEVICE_SUBFAMILY_STM32F0

#elif defined(MCULIB_DEVICE_STM32F103)
#define MCULIB_DEVICE_FAMILY_STM32
#define MCULIB_DEVICE_SUBFAMILY_STM32F1

#elif defined(MCULIB_DEVICE_STM32F303)
#define MCULIB_DEVICE_FAMILY_STM32
#define MCULIB_DEVICE_SUBFAMILY_STM32F3


#else
#error "No MCU family selected. Please define MCULIB_DEVICE_xxx"
#endif

