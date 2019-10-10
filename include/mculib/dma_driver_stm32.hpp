#pragma once
#include <mculib/dma_defs.hpp>

namespace mculib {
	class DMAChannel;
	class DMADriver {
	public:
		DMADevice device;
		int _channelsActive = 0;
		DMADriver(DMADevice d): device(d) {}
		void enable();
		void disable();
		void channelEnabled(DMAChannel& ch);
		void channelDisabled(DMAChannel& ch);
	};
	class DMAChannel {
	public:
		DMADriver& driver;
		int channel;
		bool enabled = false;

		uint32_t _nWords = 0;
		DMAChannel(DMADriver& d, int ch): driver(d), channel(ch) {}
		void enable();
		void disable();
		void start();
		void stop();

		void setTransferParams(const DMATransferParams& srcParams,
							const DMATransferParams& dstParams,
							DMADirection dir, int nWords, bool repeat);

		// returns current memory offset into array (in words)
		uint32_t position();
	};
}
