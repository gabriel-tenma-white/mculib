#pragma once
#include <mculib/dma_driver.hpp>
#include <mculib/adc_defs.hpp>

namespace mculib {

	class DMAADC {
	public:
		DMAChannel& dma;
		ADCDevice adcDevice;
		volatile void* buffer = nullptr;
		int bufferSizeBytes = 0;

		DMAADC(DMAChannel& dma, ADCDevice adcDevice):
			dma(dma), adcDevice(adcDevice) {}

		void init(uint8_t* channels, int nChannels);
		void deinit();
		void start();
		void stop();

		uint32_t position() {
			return dma.position();
		}
	};
}
