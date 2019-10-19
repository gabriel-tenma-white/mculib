#pragma once
#include <mculib/small_function.hpp>

namespace mculib {
	static void enableEXTIIRQs();
	static inline void digitalWrite(Pad p, int bit) {
		if(bit) p.bsrr() = p.mask();
		else p.bsrr() = p.maskUpper();
	}
	static inline int digitalRead(Pad p) {
		auto tmp = (uint16_t) p.idr();
		return (tmp & p.mask()) ? 1 : 0;
	}

	extern small_function<void()> irqHandlers[16];
	void attachInterrupt(irqNumber irq, const small_function<void()>& handler, irqTrigger trigger);
}
