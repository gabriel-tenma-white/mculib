#pragma once
#include <stdint.h>

#if defined(MCULIB_DEVICE_FAMILY_STM32)
#include <mculib/dma_defs_stm32.hpp>

#else
#error "DMA library does not support selected device family"

#endif

namespace mculib {
	enum class DMADirection {
		UNDEFINED=0,
		MEMORY_TO_PERIPHERAL,
		PERIPHERAL_TO_MEMORY
	};
	struct DMATransferParams {
		volatile void* address = nullptr;
		uint8_t bytesPerWord = 1;
		bool increment = false;
	};
}
