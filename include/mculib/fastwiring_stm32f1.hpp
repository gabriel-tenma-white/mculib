#pragma once
#include <mculib/fastwiring_defs.hpp>
#include <mculib/fastwiring_defs_stm32.hpp>

namespace mculib {
	static inline void pinMode(Pad p, int mode) {
		if(mode == OUTPUT) {
			gpio_set_mode(p.bank, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, uint16_t(1) << p.index);
		} else if(mode == INPUT_PULLUP) {
			gpio_set(p.bank, uint16_t(1) << p.index);
			gpio_set_mode(p.bank, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, uint16_t(1) << p.index);
		} else if(mode == INPUT_PULLDOWN) {
			gpio_clear(p.bank, uint16_t(1) << p.index);
			gpio_set_mode(p.bank, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, uint16_t(1) << p.index);
		} else {
			gpio_set_mode(p.bank, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, uint16_t(1) << p.index);
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
	static void _delay_3t(uint32_t cycles)
	{
		asm __volatile__ (
			"1: \n"
			"	subs %[cyc],#1 \n"	// 1Tck
			"	bne 1b \n"		// 2Tck
			"	bx lr \n"
			: // No output 
			: [cyc] "r" (cycles)
			: // No memory
		);
	}
	static void delayMicroseconds(uint32_t us) {
		_delay_3t(us*cpu_mhz/3);
	}
	static void delay(int ms) {
		delayMicroseconds(uint32_t(ms)*1000);
	}
}
