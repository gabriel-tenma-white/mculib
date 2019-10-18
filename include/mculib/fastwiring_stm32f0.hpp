#pragma once
#include <mculib/fastwiring_defs.hpp>
#include <mculib/fastwiring_defs_stm32.hpp>

namespace mculib {
	static inline void pinMode(Pad p, int mode) {
		if(mode == OUTPUT) {
			gpio_mode_setup(p.bank, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, 1 << p.index);
		} else if(mode == INPUT_PULLUP) {
			gpio_mode_setup(p.bank, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, 1 << p.index);
		} else if(mode == INPUT_PULLDOWN) {
			gpio_mode_setup(p.bank, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, 1 << p.index);
		} else {
			gpio_mode_setup(p.bank, GPIO_MODE_INPUT, GPIO_PUPD_NONE, 1 << p.index);
		}
	}
	static inline void digitalWrite(Pad p, int bit) {
		if(bit) p.bsrr() = uint32_t(1) << p.index;
		else p.bsrr() = uint32_t(1 << 16) << p.index;
	}
	static inline int digitalRead(Pad p) {
		auto tmp = (uint16_t) p.idr();
		return (tmp & (uint16_t(1) << p.index)) ? 1 : 0;
	}
	static void _delay_8t(uint32_t cycles)
	{
		asm __volatile__ (
			"1: \n"
			  " sub  %[cyc], %[cyc], #1 \n"  // 1 cycle
			  " nop             \n"  // 1 cycle
			  " nop             \n"  // 1 cycle
			  " nop             \n"  // 1 cycle
			  " nop             \n"  // 1 cycle
			  " nop             \n"  // 1 cycle
			  " bne  1b    \n"  // 2 if taken, 1 otherwise
			: [cyc] "+l" (cycles)
			: // no inputs
			: // No memory
		);
	}
	static void delayMicroseconds(uint32_t us) {
		_delay_8t(us*cpu_mhz/8);
	}
	static void delay(int ms) {
		delayMicroseconds(uint32_t(ms)*1000);
	}
}
