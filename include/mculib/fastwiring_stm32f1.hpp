#pragma once
#include <mculib/fastwiring_defs.hpp>
#include <mculib/fastwiring_defs_stm32.hpp>
#include <mculib/fastwiring_stm32.hpp>

namespace mculib {
	static inline void pinMode(Pad p, int mode) {
		if(mode == OUTPUT) {
			gpio_set_mode(p.bank(), GPIO_MODE_OUTPUT_10_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, p.mask());
		} else if(mode == INPUT_PULLUP) {
			gpio_set(p.bank(), p.mask());
			gpio_set_mode(p.bank(), GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, p.mask());
		} else if(mode == INPUT_PULLDOWN) {
			gpio_clear(p.bank(), p.mask());
			gpio_set_mode(p.bank(), GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, p.mask());
		} else {
			gpio_set_mode(p.bank(), GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, p.mask());
		}
	}
	static void _delay_8t(uint32_t cycles)
	{
		if(cycles <= 5) return;
		asm __volatile__ (
			"1: \n"
			  " subs  %[cyc], %[cyc], #1 \n"  // 1 cycle
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
	static void enableEXTIIRQs() {
		nvic_enable_irq(NVIC_EXTI0_IRQ);
		nvic_enable_irq(NVIC_EXTI1_IRQ);
		nvic_enable_irq(NVIC_EXTI2_IRQ);
		nvic_enable_irq(NVIC_EXTI3_IRQ);
		nvic_enable_irq(NVIC_EXTI4_IRQ);
		nvic_enable_irq(NVIC_EXTI9_5_IRQ);
		nvic_enable_irq(NVIC_EXTI15_10_IRQ);
	}
}
