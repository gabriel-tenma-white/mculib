#pragma once

#include <stdint.h>

namespace mculib {
namespace ADF4350 {

	template<class sendWord_t>
	class ADF4350Driver {
	public:
		// send a 32 bit word over spi (msb first).
		// void sendWord(uint32_t word);
		sendWord_t sendWord;

		int N = 1;
		int numerator = 0;
		int denominator = 32;
		int O = 1;
		int R = 1;
		bool pdwn = false;
		uint8_t rfPower = 0b01;
		static constexpr int auxPower = 0b00;

		// band select divider. 1 to 255.
		static constexpr int  bsDivider = 255;

		// charge pump current, 0 to 15.
		static constexpr int cpCurrent = 15;

		// CLKDIV divider (for fastlock and phase resync). 0 to 4095.
		static constexpr int clkDivDivider = 6;

		static constexpr enum {
			CLKDIVMODE_OFF = 0b00,
			CLKDIVMODE_FASTLOCK = 0b01,
			CLKDIVMODE_RESYNC = 0b10
		} clkDivMode = CLKDIVMODE_OFF;

		static constexpr bool refDouble = false;
		static constexpr bool refDiv2 = false;
		static constexpr bool rfEnable = true;
		static constexpr bool auxEnable = false;
		static constexpr bool feedbackFromDivided = false;
		static constexpr bool lowSpurMode = true;
		static constexpr int noiseMode = lowSpurMode ? 0b11 : 0b00;
		static constexpr int refDbDivMode = (refDouble << 1) | (refDiv2 << 0);

		ADF4350Driver(const sendWord_t& _sendWord): sendWord(_sendWord) {}


		//odiv: output division factor, 1 to 16
		void sendConfig() {
			bool was_pwdn = pdwn;
			pdwn = false; //Power down is no longer active.
			uint8_t odiv;
			switch(O) {
				case 1:  odiv=0b000; break;
				case 2:  odiv=0b001; break;
				case 4:  odiv=0b010; break;
				case 8:  odiv=0b011; break;
				case 16: odiv=0b100; break;
				case 32: odiv=0b101; break;
				case 64: odiv=0b110; break;
				default: odiv = 0b000; break;
			}

			bool rfEn = rfEnable ? 1 : 0;
			bool auxEn = auxEnable ? 1 : 0;
			bool fb = feedbackFromDivided ? 0 : 1;

			// reg 5
			//        LD pin      register 5
			sendWord((0b01<<22) | 0b101);

			// reg 4
			//        fb        rf divider       bs divider       aux en      aux pwr         rf en         rf pwr     register 4
			sendWord((fb<<23) | (odiv<<20) 	| (bsDivider<<12) | (auxEn<<8) | (auxPower<<6) | (rfEn<<5) | (rfPower<<3) | 0b100);

			// reg 3
			//           clkdiv mode             clkdiv           register 3
			sendWord((int(clkDivMode)<<15) | (clkDivDivider<<3) | 0b011);

			// reg 2
			//        low spur mode     muxout        reference db/div2      R          CP current    int-N    LDP     PD pol      powerdown  register 2
			sendWord((noiseMode<<29) | (0b001<<26) | (refDbDivMode << 24) | (R<<14) | (cpCurrent<<9) | (0<<8) | (0<<7) | (1<<6) | (pdwn<<5) | 0b010);

			// reg 1
			//      prescaler   phase  frac modulus
			sendWord((1<<27) | (0<<15) | (denominator<<3) | 0b001);
		}

		// sets integer N and numerator
		void sendN() {
			sendWord((N<<15) | (numerator<<3));		//register 0
		}

		void sendPowerDown() {
			if(pdwn == true)
				return;
			pdwn = true;
			sendWord((noiseMode<<29) | (0b001<<26) | (refDbDivMode << 24) | (R<<14) | (cpCurrent<<9) | (0<<8) | (0<<7) | (1<<6) | (pdwn<<5) | 0b010);

		}
		void sendPowerUp() {
			if(pdwn == false)
				return;
			pdwn = false;
			sendWord((noiseMode<<29) | (0b001<<26) | (refDbDivMode << 24) | (R<<14) | (cpCurrent<<9) | (0<<8) | (0<<7) | (1<<6) | (pdwn<<5) | 0b010);

		}
	};

}
}
