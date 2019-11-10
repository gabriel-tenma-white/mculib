#pragma once

#ifdef MCULIB_ENABLE_PRINTK
#include <mculib/message_log.hpp>
#define printk(...) mculib::messageLog.printf(__VA_ARGS__)

#else
#define printk(...) /**/

#endif
