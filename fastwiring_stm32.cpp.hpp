#include <mculib/device_selector.hpp>
#include <mculib/fastwiring.hpp>


namespace mculib {
	small_function<void()> irqHandlers[16];

	void attachInterrupt(irqNumber irq, const small_function<void()>& handler, irqTrigger trigger) {
		if(irqHandlers[irq.index]) {
			return;
		}
		irqHandlers[irq.index] = handler;
		enableEXTIIRQs();
		exti_select_source(1 << irq.index, irq.bank);
		exti_set_trigger(1 << irq.index, trigger.type);
		exti_enable_request(1 << irq.index);
	}

	void handleEXTIIRQ(int first, int last) {
		uint32_t fired_irqs = exti_get_flag_status(0xffffffff);
		exti_reset_request(fired_irqs);
		for(int i=first; i<last; i++) {
			if(irqHandlers[i] && (fired_irqs & (1<<i)))
				irqHandlers[i]();
		}
	}
}



extern "C" {

	using namespace mculib;

#if defined(MCULIB_DEVICE_SUBFAMILY_STM32F0)
	void exti0_1_isr() {
		handleEXTIIRQ(0, 2);
	}
	void exti2_3_isr() {
		handleEXTIIRQ(2, 4);
	}
	void exti4_15_isr() {
		handleEXTIIRQ(4, 16);
	}


#elif defined(MCULIB_DEVICE_SUBFAMILY_STM32F1)
	
	void exti0_isr() {
		handleEXTIIRQ(0, 1);
	}
	void exti1_isr() {
		handleEXTIIRQ(1, 2);
	}
	void exti2_isr() {
		handleEXTIIRQ(2, 3);
	}
	void exti3_isr() {
		handleEXTIIRQ(3, 4);
	}
	void exti4_isr() {
		handleEXTIIRQ(4, 5);
	}
	void exti9_5_isr() {
		handleEXTIIRQ(5, 10);
	}
	void exti15_10_isr() {
		handleEXTIIRQ(10, 16);
	}

#elif defined(MCULIB_DEVICE_SUBFAMILY_STM32F3)
	void exti0_1_isr() {
		handleEXTIIRQ(0, 2);
	}
	void exti2_3_isr() {
		handleEXTIIRQ(2, 4);
	}
	void exti4_15_isr() {
		handleEXTIIRQ(4, 16);
	}


#else
#error "fastwiring library does not support selected device subfamily"

#endif

}
