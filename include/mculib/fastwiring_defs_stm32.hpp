#pragma once
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>

namespace mculib {
	struct Pad {
		uint32_t _bank;
		uint32_t _index;
		uint32_t _bsrr;
		uint32_t _idr;


		uint32_t bank() const __attribute__((always_inline)) {
			return _bank;
		}
		uint16_t bankNumber() const {
			switch(_bank) {
				case GPIOA: return 0;
				case GPIOB: return 1;
				case GPIOC: return 2;
				case GPIOD: return 3;
				case GPIOE: return 4;
				case GPIOF: return 5;
				default: return -1;
			}
		}
		uint32_t index() const __attribute__((always_inline)) {
			return _index;
		}
		uint32_t mask() const __attribute__((always_inline)) {
			return uint32_t(1) << _index;
		}
		uint32_t maskUpper() const __attribute__((always_inline)) {
			return uint32_t(1 << 16) << _index;
		}

		volatile uint32_t& bsrr() const __attribute__((always_inline)) {
			return *(volatile uint32_t*) _bsrr;
		}
		volatile uint32_t& idr() const __attribute__((always_inline)) {
			return *(volatile uint32_t*) _idr;
		}
		constexpr Pad(): _bank(0), _index(0), _bsrr(0), _idr(0) {}
		constexpr Pad(uint32_t bank, uint32_t index):
			_bank(bank), _index(index),
			_bsrr(BSRR(bank)),
			_idr(IDR(bank)) {
		}

		// device-specific functions

		// return the address of the BSRR register for the bank
		static constexpr uint32_t BSRR(uint32_t bank) {
			#if defined(MCULIB_DEVICE_SUBFAMILY_STM32F0)
				return bank + 0x18;
			#elif defined(MCULIB_DEVICE_SUBFAMILY_STM32F1)
				return bank + 0x10;
			#elif defined(MCULIB_DEVICE_SUBFAMILY_STM32F3)
				return bank + 0x18;
			#else
				#error "Device subfamily not supported"
			#endif
		}
		// return the address of the IDR register for the bank
		static constexpr uint32_t IDR(uint32_t bank) {
			#if defined(MCULIB_DEVICE_SUBFAMILY_STM32F0)
				return bank + 0x10;
			#elif defined(MCULIB_DEVICE_SUBFAMILY_STM32F1)
				return bank + 0x08;
			#elif defined(MCULIB_DEVICE_SUBFAMILY_STM32F3)
				return bank + 0x10;
			#else
				#error "Device subfamily not supported"
			#endif
		}
	};
	
	static inline bool operator==(const Pad& a, const Pad& b) {
		return (a.bank() == b.bank()) && (a.index() == b.index());
	}

	struct irqNumber {
		uint32_t bank, index;
	};
	static inline bool operator==(irqNumber a, irqNumber b) {
		return (a.bank == b.bank) && (a.index == b.index);
	}
	static irqNumber digitalPinToInterrupt(Pad p) {
		return irqNumber {p.bank(), p.index()};
	}

	struct irqTrigger {
		exti_trigger_type type;
	};

	static constexpr irqTrigger RISING { EXTI_TRIGGER_RISING };
	static constexpr irqTrigger FALLING { EXTI_TRIGGER_FALLING };
	static constexpr irqTrigger RISING_AND_FALLING { EXTI_TRIGGER_BOTH };

	static constexpr Pad PA0 = {GPIOA, 0};
	static constexpr Pad PA1 = {GPIOA, 1};
	static constexpr Pad PA2 = {GPIOA, 2};
	static constexpr Pad PA3 = {GPIOA, 3};
	static constexpr Pad PA4 = {GPIOA, 4};
	static constexpr Pad PA5 = {GPIOA, 5};
	static constexpr Pad PA6 = {GPIOA, 6};
	static constexpr Pad PA7 = {GPIOA, 7};
	static constexpr Pad PA8 = {GPIOA, 8};
	static constexpr Pad PA9 = {GPIOA, 9};
	static constexpr Pad PA10 = {GPIOA, 10};
	static constexpr Pad PA11 = {GPIOA, 11};
	static constexpr Pad PA12 = {GPIOA, 12};
	static constexpr Pad PA13 = {GPIOA, 13};
	static constexpr Pad PA14 = {GPIOA, 14};
	static constexpr Pad PA15 = {GPIOA, 15};

	static constexpr Pad PB0 = {GPIOB, 0};
	static constexpr Pad PB1 = {GPIOB, 1};
	static constexpr Pad PB2 = {GPIOB, 2};
	static constexpr Pad PB3 = {GPIOB, 3};
	static constexpr Pad PB4 = {GPIOB, 4};
	static constexpr Pad PB5 = {GPIOB, 5};
	static constexpr Pad PB6 = {GPIOB, 6};
	static constexpr Pad PB7 = {GPIOB, 7};
	static constexpr Pad PB8 = {GPIOB, 8};
	static constexpr Pad PB9 = {GPIOB, 9};
	static constexpr Pad PB10 = {GPIOB, 10};
	static constexpr Pad PB11 = {GPIOB, 11};
	static constexpr Pad PB12 = {GPIOB, 12};
	static constexpr Pad PB13 = {GPIOB, 13};
	static constexpr Pad PB14 = {GPIOB, 14};
	static constexpr Pad PB15 = {GPIOB, 15};

	static constexpr Pad PC0 = {GPIOC, 0};
	static constexpr Pad PC1 = {GPIOC, 1};
	static constexpr Pad PC2 = {GPIOC, 2};
	static constexpr Pad PC3 = {GPIOC, 3};
	static constexpr Pad PC4 = {GPIOC, 4};
	static constexpr Pad PC5 = {GPIOC, 5};
	static constexpr Pad PC6 = {GPIOC, 6};
	static constexpr Pad PC7 = {GPIOC, 7};
	static constexpr Pad PC8 = {GPIOC, 8};
	static constexpr Pad PC9 = {GPIOC, 9};
	static constexpr Pad PC10 = {GPIOC, 10};
	static constexpr Pad PC11 = {GPIOC, 11};
	static constexpr Pad PC12 = {GPIOC, 12};
	static constexpr Pad PC13 = {GPIOC, 13};
	static constexpr Pad PC14 = {GPIOC, 14};
	static constexpr Pad PC15 = {GPIOC, 15};

	static constexpr Pad PD0 = {GPIOD, 0};
	static constexpr Pad PD1 = {GPIOD, 1};
	static constexpr Pad PD2 = {GPIOD, 2};
	static constexpr Pad PD3 = {GPIOD, 3};
	static constexpr Pad PD4 = {GPIOD, 4};
	static constexpr Pad PD5 = {GPIOD, 5};
	static constexpr Pad PD6 = {GPIOD, 6};
	static constexpr Pad PD7 = {GPIOD, 7};
	static constexpr Pad PD8 = {GPIOD, 8};
	static constexpr Pad PD9 = {GPIOD, 9};
	static constexpr Pad PD10 = {GPIOD, 10};
	static constexpr Pad PD11 = {GPIOD, 11};
	static constexpr Pad PD12 = {GPIOD, 12};
	static constexpr Pad PD13 = {GPIOD, 13};
	static constexpr Pad PD14 = {GPIOD, 14};
	static constexpr Pad PD15 = {GPIOD, 15};

	static constexpr Pad PE0 = {GPIOE, 0};
	static constexpr Pad PE1 = {GPIOE, 1};
	static constexpr Pad PE2 = {GPIOE, 2};
	static constexpr Pad PE3 = {GPIOE, 3};
	static constexpr Pad PE4 = {GPIOE, 4};
	static constexpr Pad PE5 = {GPIOE, 5};
	static constexpr Pad PE6 = {GPIOE, 6};
	static constexpr Pad PE7 = {GPIOE, 7};
	static constexpr Pad PE8 = {GPIOE, 8};
	static constexpr Pad PE9 = {GPIOE, 9};
	static constexpr Pad PE10 = {GPIOE, 10};
	static constexpr Pad PE11 = {GPIOE, 11};
	static constexpr Pad PE12 = {GPIOE, 12};
	static constexpr Pad PE13 = {GPIOE, 13};
	static constexpr Pad PE14 = {GPIOE, 14};
	static constexpr Pad PE15 = {GPIOE, 15};

	static constexpr Pad PF0 = {GPIOF, 0};
	static constexpr Pad PF1 = {GPIOF, 1};
	static constexpr Pad PF2 = {GPIOF, 2};
	static constexpr Pad PF3 = {GPIOF, 3};
	static constexpr Pad PF4 = {GPIOF, 4};
	static constexpr Pad PF5 = {GPIOF, 5};
	static constexpr Pad PF6 = {GPIOF, 6};
	static constexpr Pad PF7 = {GPIOF, 7};
	static constexpr Pad PF8 = {GPIOF, 8};
	static constexpr Pad PF9 = {GPIOF, 9};
	static constexpr Pad PF10 = {GPIOF, 10};
	static constexpr Pad PF11 = {GPIOF, 11};
	static constexpr Pad PF12 = {GPIOF, 12};
	static constexpr Pad PF13 = {GPIOF, 13};
	static constexpr Pad PF14 = {GPIOF, 14};
	static constexpr Pad PF15 = {GPIOF, 15};
}
