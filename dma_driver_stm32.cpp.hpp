#include <mculib/dma_driver_stm32.hpp>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/f1/dma.h>
#include <stdlib.h>

namespace mculib {
	rcc_periph_clken DMA_RCC(DMADevice d) {
		if(d == DMA1)
			return RCC_DMA1;
		if(d == DMA2)
			return RCC_DMA2;
		abort();
	}
	void DMADriver::enable() {
		rcc_periph_clock_enable(DMA_RCC(device));
	}
	void DMADriver::disable() {
		rcc_periph_clock_disable(DMA_RCC(device));
	}
	void DMADriver::channelEnabled(DMAChannel& ch) {
		if(_channelsActive == 0) enable();
		_channelsActive++;
	}
	void DMADriver::channelDisabled(DMAChannel& ch) {
		_channelsActive--;
		if(_channelsActive == 0) disable();
	}


	void DMAChannel::enable() {
		if(enabled) return;
		enabled = true;
		driver.channelEnabled(*this);
	}
	void DMAChannel::disable() {
		enabled = false;
		driver.channelDisabled(*this);
	}
	void DMAChannel::start() {
		dma_enable_channel(driver.device, channel);
	}
	void DMAChannel::stop() {
		dma_disable_channel(driver.device, channel);
	}
	static uint32_t DMASizeP(uint32_t bytesPerWord) {
		if(bytesPerWord == 1) return 0 << 8;
		if(bytesPerWord == 2) return 1 << 8;
		if(bytesPerWord == 4) return 2 << 8;
		abort();
	}
	static uint32_t DMASizeM(uint32_t bytesPerWord) {
		if(bytesPerWord == 1) return 0 << 10;
		if(bytesPerWord == 2) return 1 << 10;
		if(bytesPerWord == 4) return 2 << 10;
		abort();
	}
	void DMAChannel::setTransferParams(const DMATransferParams& srcParams,
							const DMATransferParams& dstParams,
							DMADirection dir, int nWords, bool repeat) {
		uint32_t device = driver.device;
		dma_set_number_of_data(device, channel, nWords);

		if(dir == DMADirection::MEMORY_TO_PERIPHERAL) {
			// memory to peripheral
			dma_set_read_from_memory(device, channel);
			dma_set_memory_address(device, channel, (uint32_t)srcParams.address);
			dma_set_peripheral_address(device, channel, (uint32_t)dstParams.address);
			if(srcParams.increment)
				dma_enable_memory_increment_mode(device, channel);
			else dma_disable_memory_increment_mode(device, channel);
			dma_set_memory_size(device, channel, DMASizeM(srcParams.bytesPerWord));
			dma_set_peripheral_size(device, channel, DMASizeP(dstParams.bytesPerWord));
		} else {
			// peripheral to memory
			dma_set_read_from_peripheral(device, channel);
			dma_set_peripheral_address(device, channel, (uint32_t)srcParams.address);
			dma_set_memory_address(device, channel, (uint32_t)dstParams.address);
			if(dstParams.increment)
				dma_enable_memory_increment_mode(device, channel);
			else dma_disable_memory_increment_mode(device, channel);
			dma_set_peripheral_size(device, channel, DMASizeP(srcParams.bytesPerWord));
			dma_set_memory_size(device, channel, DMASizeM(dstParams.bytesPerWord));
		}
		if(repeat)
			dma_enable_circular_mode(device, channel);
		else
			DMA_CCR(device, channel) &= ~DMA_CCR_CIRC;
		_nWords = nWords;
	}
	uint32_t DMAChannel::position() {
		return _nWords - DMA_CNDTR(driver.device, channel);
	}
	bool DMAChannel::finished() {
		return DMA_CNDTR(driver.device, channel) == 0;
	}
}
