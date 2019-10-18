#pragma once
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>

namespace mculib {
	struct Pad {
		uint32_t bank;
		uint32_t index;
		uint32_t _bsrr;
		uint32_t _idr;

		volatile uint32_t& bsrr() {
			return *(volatile uint32_t*) _bsrr;
		}
		volatile uint32_t& idr() {
			return *(volatile uint32_t*) _idr;
		}
		Pad(): bank(0), index(0), _bsrr(0), _idr(0) {}
		constexpr Pad(uint32_t bank, uint32_t index):
			bank(bank), index(index),
			_bsrr((uint32_t)&GPIO_BSRR(bank)),
			_idr((uint32_t)&GPIO_IDR(bank)) {
		}
	};

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
