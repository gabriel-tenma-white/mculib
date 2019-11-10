#pragma once
#include <mculib/fastwiring.hpp>

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
			_delay();
			digitalWrite(sel, 0);
			_delay(); _delay();
		}
		void endTransfer() {
			_delay(); _delay();
			digitalWrite(clk, 0);
			digitalWrite(sel, 1);
		}
		
		// perform a send & recv transfer
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

		// perform a send-only transfer
		void doTransfer_send(uint32_t data, int bits) {
			data <<= (32-bits);
			for(int i=0;i<bits;i++) {
				// clock low; put data on bus
				digitalWrite(mosi, data>>31);
				digitalWrite(clk, 0);
				data <<= 1;
				_delay();
				// clock high
				digitalWrite(clk, 1);
				_delay();
			}
		}
		
		void doTransfer_bulk_send(const uint8_t* buf, int len) {
			const uint8_t* end = buf + len;
			while(buf != end) {
				uint8_t value = *buf;
				for(int i=0;i<8;i++) {
					// clock low; put data on bus
					digitalWrite(mosi, value>>7);
					digitalWrite(clk, 0);
					value <<= 1;
					_delay();
					// clock high
					digitalWrite(clk, 1);
					_delay();
				}
				buf++;
			}
		}
		
		void doTransfer_bulk_recv(uint8_t* buf, int len) {
			digitalWrite(mosi, 0);
			uint8_t* end = buf + len;
			while(buf != end) {
				uint8_t ret = 0;
				for(int i=0;i<8;i++) {
					// clock low
					digitalWrite(clk, 0);
					_delay();
					// clock high, read data from
					ret = uint32_t(digitalRead(miso)) | (ret << 1);
					digitalWrite(clk, 1);
					_delay();
				}
				*buf = ret;
				buf++;
			}
		}

		void doTransfer_bulk_send(const uint16_t* buf, int words) {
			const uint16_t* end = buf + words;
			while(buf != end) {
				uint16_t value = *buf;
				for(int i=0;i<16;i++) {
					// clock low; put data on bus
					digitalWrite(mosi, value>>15);
					digitalWrite(clk, 0);
					value <<= 1;
					_delay();
					// clock high
					digitalWrite(clk, 1);
					_delay();
				}
				buf++;
			}
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
