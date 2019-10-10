#pragma once
#include <mculib/fastwiring.H>

namespace mculib {

	template<class delayFunc_t>
	class SoftSPI {
	public:
		// pins
		Pad sel,clk,mosi,miso;

		delayFunc_t _delay;

		SoftSPI(const delayFunc_t& delay): _delay(delay) {
			
		}

		void init() {
			digitalWrite(sel, 1);
			pinMode(sel, OUTPUT);
			pinMode(clk, OUTPUT);
			pinMode(mosi, OUTPUT);
			pinMode(miso, INPUT);
			digitalWrite(clk, 0);
			for(int i=0;i<10;i++) _delay();
		}
		void beginTransfer() {
			digitalWrite(sel, 0);
			_delay();
		}
		void endTransfer() {
			digitalWrite(clk, 0);
			digitalWrite(sel, 1);
			_delay();
		}
		
		uint32_t doTransfer(uint32_t data, int bits) {
			data <<= (32-bits);
			uint32_t ret = 0;
			for(int i=0;i<bits;i++) {
				// clock low; put data on bus
				digitalWrite(mosi, data>>31);
				digitalWrite(clk, 0);
				data <<= 1;
				_delay();
				// clock high, read data from
				ret = uint32_t(digitalRead(miso)) | (ret << 1);
				digitalWrite(clk, 1);
				_delay();
			}
			return ret;
		}
		
		uint32_t doTransfer_slow(uint32_t data, int bits) {
			pinMode(mosi, OUTPUT);
			data <<= (32-bits);
			uint32_t ret = 0;
			for(int i=0;i<bits;i++) {
				// put data on bus
				digitalWrite(mosi, data>>31);
				
				_delay();
				// clock high, read data from 
				ret = uint32_t(digitalRead(miso)) | (ret << 1);
				digitalWrite(clk, 1);
				_delay();
				_delay();
				digitalWrite(clk, 0);
				_delay();
				data <<= 1;
			}
			return ret;
		}
		uint32_t doTransfer_recv(int bits) {
			uint32_t ret = 0;
			digitalWrite(mosi, 0);
			_delay();
			pinMode(mosi, INPUT);
			_delay();
			for(int i=0;i<bits;i++) {
				// clock low
				digitalWrite(clk, 0);
				_delay();
				ret = uint32_t(digitalRead(mosi)?1:0) | (ret << 1);
				_delay();
				// clock high
				digitalWrite(clk, 1);
				_delay();
				_delay();
			}
			//pinMode(mosi, OUTPUT);
			return ret;
		}
		
	};
}
