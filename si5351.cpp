/*
 * si5351.c
 *
 *  Created on: Jan 14, 2019
 *      Author: Petr Polasek
 *
 *      HOOKS:
 *      WriteRegister
 *      ReadRegister
 *      	You need to write your own I2C handlers here
 *
 */

//put your I2C HAL library name here
//#include "stm32f0xx_i2c.h"


#include <mculib/si5351.hpp>

namespace mculib {
namespace Si5351 {

	//set safe values in the config structure
	void Si5351Driver::SetFieldsToDefault()
	{
		uint8_t i;

		this->f_CLKIN = 0;
		this->f_XTAL = 26000000;

		this->Interrupt_Mask_CLKIN = ON;
		this->Interrupt_Mask_PLLA = ON;
		this->Interrupt_Mask_PLLB = ON;
		this->Interrupt_Mask_SysInit = ON;
		this->Interrupt_Mask_XTAL = ON;

		this->Fanout_CLKIN_EN = ON;
		this->Fanout_MS_EN = ON;
		this->Fanout_XO_EN = ON;

		this->OSC.CLKIN_Div = CLKINDiv_Div1;
		this->OSC.OSC_XTAL_Load = XTAL_Load_10_pF;
		this->OSC.VCXO_Pull_Range_ppm = 0; //maybe should be set to 30 ppm, not clear from the AN-619

		for (i=0; i<=1; i++)
		{
			this->PLL[i].PLL_Clock_Source = PLL_Clock_Source_XTAL;
			this->PLL[i].PLL_Multiplier_Integer = 32; 		//range 24..36 for 25 MHz clock
			this->PLL[i].PLL_Multiplier_Numerator = 0; 		//range 0..1048575
			this->PLL[i].PLL_Multiplier_Denominator = 1; 	//range 1..1048575
			this->PLL[i].PLL_Capacitive_Load = PLL_Capacitive_Load_0;		//select 0, unless you want to tune the PLL to <200 MHZ
		}

		this->SS.SS_Amplitude_ppm = 15000;
		this->SS.SS_Enable = OFF;
		this->SS.SS_Mode = SS_Mode_CenterSpread;

		for (i=0; i<=7; i++)
		{
			this->MS[i].MS_Clock_Source = MS_Clock_Source_PLLA;
			this->MS[i].MS_Divider_Integer = 4;
			this->MS[i].MS_Divider_Numerator = 0;
			this->MS[i].MS_Divider_Denominator = 1;

			this->CLK[i].CLK_Clock_Source = CLK_Clock_Source_MS_Own;
			this->CLK[i].CLK_Disable_State = CLK_Disable_State_HIGH_Z;
			this->CLK[i].CLK_Enable = OFF;
			this->CLK[i].CLK_I_Drv = CLK_I_Drv_8mA;
			this->CLK[i].CLK_Invert = OFF;
			this->CLK[i].CLK_QuarterPeriod_Offset = 0;
			this->CLK[i].CLK_R_Div = CLK_R_Div1;
			this->CLK[i].CLK_Use_OEB_Pin = OFF;
		}
	}

	void Si5351Driver::OSCConfig()
	{
		uint8_t tmp;
		uint32_t VCXO_Param;

		//set XTAL capacitive load and PLL VCO load capacitance
		tmp = ReadRegister(REG_XTAL_CL);
		tmp &= ~(XTAL_CL_MASK | PLL_CL_MASK);
		tmp |= (XTAL_CL_MASK & (this->OSC.OSC_XTAL_Load)) | (PLL_CL_MASK & ((this->PLL[0].PLL_Capacitive_Load) << 1)) | (PLL_CL_MASK & ((this->PLL[1].PLL_Capacitive_Load) << 4));
		WriteRegister(REG_XTAL_CL, tmp);

		//set CLKIN pre-divider
		tmp = ReadRegister(REG_CLKIN_DIV);
		tmp &= ~CLKIN_MASK;
		tmp |= CLKIN_MASK & this->OSC.CLKIN_Div;
		WriteRegister(REG_CLKIN_DIV, tmp);

		//set fanout of XO, MS0, MS4 and CLKIN - should be always on unless you
		//need to reduce power consumption
		tmp = ReadRegister(REG_FANOUT_EN);
		tmp &= ~(FANOUT_CLKIN_EN_MASK | FANOUT_MS_EN_MASK | FANOUT_XO_EN_MASK);
		if (this->Fanout_CLKIN_EN == ON) tmp |= FANOUT_CLKIN_EN_MASK;
		if (this->Fanout_MS_EN == ON) tmp |= FANOUT_MS_EN_MASK;
		if (this->Fanout_XO_EN == ON) tmp |= FANOUT_XO_EN_MASK;
		WriteRegister(REG_FANOUT_EN, tmp);

		//set default value of SS_NCLK - spread spectrum reserved register
		tmp = ReadRegister(REG_SS_NCLK);
		tmp &= ~SS_NCLK_MASK; //set upper nibble to 0000b
		WriteRegister(REG_SS_NCLK, tmp);

		//if "b" in PLLB set to 10^6, set VCXO parameter
		if (this->PLL[1].PLL_Multiplier_Denominator == 1000000)
		{
			VCXO_Param = VCXO_PARAM_MASK & (uint32_t)
					((103 * this->OSC.VCXO_Pull_Range_ppm
							* ((uint64_t)128000000 * this->PLL[1].PLL_Multiplier_Integer +
									this->PLL[1].PLL_Multiplier_Numerator))/100000000);
		} else {
			VCXO_Param = 0;
		}

		tmp = (uint8_t) VCXO_Param;
		WriteRegister(REG_VCXO_PARAM_0_7, tmp);
		tmp = (uint8_t)(VCXO_Param>>8);
		WriteRegister(REG_VCXO_PARAM_8_15, tmp);
		tmp = (uint8_t)((VCXO_Param>>16) & VCXO_PARAM_16_21_MASK);
		WriteRegister(REG_VCXO_PARAM_16_21, tmp);
	}

	EnableState Si5351Driver::CheckStatusBit(StatusBit statusBit)
	{
		uint8_t tmp;

		tmp = ReadRegister(REG_DEV_STATUS);
		tmp &= statusBit;
		return (EnableState) tmp;
	}

	EnableState Si5351Driver::CheckStickyBit(StatusBit statusBit)
	{
		uint8_t tmp;

		tmp = ReadRegister(REG_DEV_STICKY);
		tmp &= statusBit;
		return (EnableState) tmp;
	}

	void Si5351Driver::InterruptConfig()
	{
		uint8_t tmp;
		tmp = ReadRegister(REG_INT_MASK);

		tmp &= ~INT_MASK_LOS_CLKIN_MASK;
		if (this->Interrupt_Mask_CLKIN == ON)
		{
			tmp |= INT_MASK_LOS_CLKIN_MASK;
		}

		tmp &= ~INT_MASK_LOS_XTAL_MASK;
		if (this->Interrupt_Mask_XTAL == ON)
		{
			tmp |= INT_MASK_LOS_XTAL_MASK;
		}

		tmp &= ~INT_MASK_LOL_A_MASK;
		if (this->Interrupt_Mask_PLLA == ON)
		{
			tmp |= INT_MASK_LOL_A_MASK;
		}

		tmp &= ~INT_MASK_LOL_B_MASK;
		if (this->Interrupt_Mask_PLLB == ON)
		{
			tmp |= INT_MASK_LOL_B_MASK;
		}

		tmp &= ~INT_MASK_SYS_INIT_MASK;
		if (this->Interrupt_Mask_SysInit == ON)
		{
			tmp |= INT_MASK_SYS_INIT_MASK;
		}

		WriteRegister(REG_INT_MASK, tmp);
	}

	void Si5351Driver::ClearStickyBit(StatusBit statusBit)
	{
		uint8_t tmp;

		tmp = ReadRegister(REG_DEV_STICKY);
		tmp &= ~statusBit;
		WriteRegister(REG_DEV_STICKY, tmp);
	}

	void Si5351Driver::PLLConfig(PLLChannel PLL_Channel)
	{
		uint8_t tmp, tmp_mask;
		uint32_t MSN_P1, MSN_P2, MSN_P3;

		//set PLL clock source
		tmp = ReadRegister(REG_PLL_CLOCK_SOURCE);
		tmp_mask = PLLA_CLOCK_SOURCE_MASK << PLL_Channel;
		tmp &= ~tmp_mask;
		tmp |= tmp_mask & this->PLL[PLL_Channel].PLL_Clock_Source;
		WriteRegister(REG_PLL_CLOCK_SOURCE, tmp);

		//if new multiplier not even  integer, disable the integer mode
		if ((this->PLL[PLL_Channel].PLL_Multiplier_Numerator != 0) | ((this->PLL[PLL_Channel].PLL_Multiplier_Integer & 127) != 0 ))
		{
			tmp = ReadRegister(REG_FB_INT + PLL_Channel);
			tmp &= ~FB_INT_MASK;
			WriteRegister(REG_FB_INT + PLL_Channel, tmp);
		}

		//configure the PLL multiplier
		/*MSN_P1 = 128 * this->PLL[PLL_Channel].PLL_Multiplier_Integer
				+ ((128 * this->PLL[PLL_Channel].PLL_Multiplier_Numerator)
						/ this->PLL[PLL_Channel].PLL_Multiplier_Denominator)
				- 512;
		MSN_P2 = 128 * this->PLL[PLL_Channel].PLL_Multiplier_Numerator
				- this->PLL[PLL_Channel].PLL_Multiplier_Denominator
					* ((128 * this->PLL[PLL_Channel].PLL_Multiplier_Numerator)
						/ this->PLL[PLL_Channel].PLL_Multiplier_Denominator);
		MSN_P3 = this->PLL[PLL_Channel].PLL_Multiplier_Denominator;
		*/
		MSN_P1 = this->PLL[PLL_Channel].PLL_Multiplier_Integer - 512;
		MSN_P2 = this->PLL[PLL_Channel].PLL_Multiplier_Numerator;
		MSN_P3 = this->PLL[PLL_Channel].PLL_Multiplier_Denominator;

		tmp = (uint8_t) MSN_P1;
		WriteRegister(REG_MSN_P1_0_7 + 8 * PLL_Channel, tmp);
		tmp = (uint8_t) (MSN_P1 >> 8);
		WriteRegister(REG_MSN_P1_8_15 + 8 * PLL_Channel, tmp);
		tmp = (uint8_t) (MSN_P1_16_17_MASK & (MSN_P1 >> 16));
		WriteRegister(REG_MSN_P1_16_17 + 8 * PLL_Channel, tmp);

		tmp = (uint8_t) MSN_P2;
		WriteRegister(REG_MSN_P2_0_7 + 8 * PLL_Channel, tmp);
		tmp = (uint8_t) (MSN_P2 >> 8);
		WriteRegister(REG_MSN_P2_8_15 + 8 * PLL_Channel, tmp);
		tmp = ReadRegister(REG_MSN_P2_16_19);
		tmp &= ~MSN_P2_16_19_MASK;
		tmp |= (uint8_t) (MSN_P2_16_19_MASK & (MSN_P2 >> 16));
		WriteRegister(REG_MSN_P2_16_19 + 8 * PLL_Channel, tmp);
		this->val_REG_MSN_P2_16_19 = tmp;

		tmp = (uint8_t) MSN_P3;
		WriteRegister(REG_MSN_P3_0_7 + 8 * PLL_Channel, tmp);
		tmp = (uint8_t) (MSN_P3 >> 8);
		WriteRegister(REG_MSN_P3_8_15 + 8 * PLL_Channel, tmp);
		tmp = ReadRegister(REG_MSN_P3_16_19);
		tmp &= ~MSN_P3_16_19_MASK;
		tmp |= (uint8_t) (MSN_P3_16_19_MASK & ((MSN_P3 >> 16) << 4));
		WriteRegister(REG_MSN_P3_16_19 + 8 * PLL_Channel, tmp);
		this->val_REG_MSN_P3_16_19 = tmp;

		//if new multiplier is an even integer, enable integer mode
		if ((this->PLL[PLL_Channel].PLL_Multiplier_Numerator == 0) & ((this->PLL[PLL_Channel].PLL_Multiplier_Integer & 127) == 0 ))
		{
			tmp = ReadRegister(REG_FB_INT + PLL_Channel);
			tmp |= FB_INT_MASK;
			WriteRegister(REG_FB_INT + PLL_Channel, tmp);
		}
	}

	// change only the integer and numerator
	void Si5351Driver::PLLConfig2(PLLChannel channel)
	{
		uint8_t tmp, tmp_mask;
		uint32_t MSN_P1, MSN_P2, MSN_P3;

		//configure the PLL multiplier
		
		MSN_P1 = PLL[channel].PLL_Multiplier_Integer - 512;
		MSN_P2 = PLL[channel].PLL_Multiplier_Numerator;
		MSN_P3 = PLL[channel].PLL_Multiplier_Denominator;

		tmp = (uint8_t) MSN_P1;
		WriteRegister(REG_MSN_P1_0_7 + 8 * channel, tmp);
		tmp = (uint8_t) (MSN_P1 >> 8);
		WriteRegister(REG_MSN_P1_8_15 + 8 * channel, tmp);
		tmp = (uint8_t) (MSN_P1_16_17_MASK & (MSN_P1 >> 16));
		WriteRegister(REG_MSN_P1_16_17 + 8 * channel, tmp);

		tmp = (uint8_t) MSN_P2;
		WriteRegister(REG_MSN_P2_0_7 + 8 * channel, tmp);
		tmp = (uint8_t) (MSN_P2 >> 8);
		WriteRegister(REG_MSN_P2_8_15 + 8 * channel, tmp);
		tmp = val_REG_MSN_P2_16_19 & (~MSN_P2_16_19_MASK);
		tmp |= (uint8_t) (MSN_P2_16_19_MASK & (MSN_P2 >> 16));
		WriteRegister(REG_MSN_P2_16_19 + 8 * channel, tmp);

		tmp = (uint8_t) MSN_P3;
		WriteRegister(REG_MSN_P3_0_7 + 8 * channel, tmp);
		tmp = (uint8_t) (MSN_P3 >> 8);
		WriteRegister(REG_MSN_P3_8_15 + 8 * channel, tmp);
		tmp = val_REG_MSN_P3_16_19 & (~MSN_P3_16_19_MASK);
		tmp |= (uint8_t) (MSN_P3_16_19_MASK & ((MSN_P3 >> 16) << 4));
		WriteRegister(REG_MSN_P3_16_19 + 8 * channel, tmp);
	}

	void Si5351Driver::PLLReset(PLLChannel PLL_Channel)
	{
		uint8_t tmp;

		//reset PLL
		tmp = ReadRegister(REG_PLL_RESET);
		if (PLL_Channel == PLL_A)
		{
			tmp |= PLLA_RESET_MASK;
		} else {
			tmp |= PLLB_RESET_MASK;
		}
		WriteRegister(REG_PLL_RESET, tmp);
	}
	void Si5351Driver::PLLReset2()
	{
		uint8_t tmp = this->val_REG_PLL_RESET;
		tmp |= PLLA_RESET_MASK | PLLB_RESET_MASK;
		WriteRegister(REG_PLL_RESET, tmp);
	}

	void Si5351Driver::SSConfig()
	{
		uint8_t tmp;
		uint32_t SSUDP, SSUP_P1, SSUP_P2, SSUP_P3, SSDN_P1, SSDN_P2, SSDN_P3;
		uint64_t SSDN, SSUP;

		//turn off SS if it should be disabled
		if ((this->SS.SS_Enable == OFF)|
				(((this->PLL[0].PLL_Multiplier_Integer & 0x01) == 0)
						& (this->PLL[0].PLL_Multiplier_Numerator == 0)) )
		{
			tmp = ReadRegister(REG_SSC_EN);
			tmp &= ~SSC_EN_MASK;
			WriteRegister(REG_SSC_EN, tmp);
		}

		//set default SS_NCLK value = 0
		tmp = ReadRegister(REG_SS_NCLK);
		tmp &= ~SS_NCLK_MASK;
		WriteRegister(REG_SS_NCLK, tmp);

		//set SS mode
		tmp = ReadRegister(REG_SSC_MODE);
		tmp &= ~SSC_MODE_MASK;
		tmp |= SSC_MODE_MASK & this->SS.SS_Mode;
		WriteRegister(REG_SSC_MODE, tmp);

		//set SSUDP parameter
		if (this->PLL[0].PLL_Clock_Source == PLL_Clock_Source_CLKIN)
		{
			SSUDP = (this->f_CLKIN)/(4*31500);
		} else {
			SSUDP = (this->f_XTAL)/(4*31500);
		}

		//set SSUDP parameter
		tmp = (uint8_t) SSUDP;
		WriteRegister(REG_SSUDP_0_7, tmp);
		tmp = ReadRegister(REG_SSUDP_8_11);
		tmp &= ~SSUDP_8_11_MASK;
		tmp |= (uint8_t) (SSUDP_8_11_MASK & ((SSUDP >> 8) << 4));
		WriteRegister(REG_SSUDP_8_11, tmp);

		//calculate SSUP and SSDN parameters
		if (this->SS.SS_Mode == SS_Mode_CenterSpread)
		{
			SSUP = ((uint64_t)(64000000*this->PLL[0].PLL_Multiplier_Integer
					  + (64000000*this->PLL[0].PLL_Multiplier_Numerator)/(this->PLL[0].PLL_Multiplier_Denominator)
					 ) * this->SS.SS_Amplitude_ppm
					) / ((1000000 - this->SS.SS_Amplitude_ppm) * SSUDP);

			SSDN = ((uint64_t)(64000000*this->PLL[0].PLL_Multiplier_Integer
					  + (64000000*this->PLL[0].PLL_Multiplier_Numerator)/(this->PLL[0].PLL_Multiplier_Denominator)
					 ) * this->SS.SS_Amplitude_ppm
					) / ((1000000 + this->SS.SS_Amplitude_ppm) * SSUDP);

			SSUP_P1 = (uint32_t) (SSUP/1000000);
			SSUP_P2 = (uint32_t)(32767*(SSUP/1000000-SSUP_P1));
			SSUP_P3 = 0x7FFF;

		} else {

			SSDN = ((uint64_t)(64000000*this->PLL[0].PLL_Multiplier_Integer
						 + (64000000*this->PLL[0].PLL_Multiplier_Numerator)/(this->PLL[0].PLL_Multiplier_Denominator)
					 ) * this->SS.SS_Amplitude_ppm
					) / ((1000000 + this->SS.SS_Amplitude_ppm) * SSUDP);

			SSUP_P1 = 0;
			SSUP_P2 = 0;
			SSUP_P3 = 1;

		}

		//set SSDN parameter
		SSDN_P1 = (uint32_t) (SSDN/1000000);
		SSDN_P2 = (uint32_t)(32767*(SSDN/1000000-SSDN_P1));
		SSDN_P3 = 0x7FFF;

		//write SSUP parameter P1
		tmp = (uint8_t) SSUP_P1;
		WriteRegister(REG_SSUP_P1_0_7, tmp);
		tmp = ReadRegister(REG_SSUP_P1_8_11);
		tmp &= ~SSUP_P1_8_11_MASK;
		tmp |= (uint8_t)(SSUP_P1_8_11_MASK & (SSUP_P1 >> 8));
		WriteRegister(REG_SSUP_P1_8_11, tmp);

		//write SSUP parameter P2
		tmp = (uint8_t) SSUP_P2;
		WriteRegister(REG_SSUP_P2_0_7, tmp);
		tmp = ReadRegister(REG_SSUP_P2_8_14);
		tmp &= ~SSUP_P2_8_14_MASK;
		tmp |= (uint8_t)(SSUP_P2_8_14_MASK & (SSUP_P2 >> 8));
		WriteRegister(REG_SSUP_P2_8_14, tmp);

		//write SSUP parameter P3
		tmp = (uint8_t) SSUP_P3;
		WriteRegister(REG_SSUP_P3_0_7, tmp);
		tmp = ReadRegister(REG_SSUP_P3_8_14);
		tmp &= ~SSUP_P3_8_14_MASK;
		tmp |= (uint8_t)(SSUP_P3_8_14_MASK & (SSUP_P3 >> 8));
		WriteRegister(REG_SSUP_P3_8_14, tmp);

		//write SSDN parameter P1
		tmp = (uint8_t) SSDN_P1;
		WriteRegister(REG_SSDN_P1_0_7, tmp);
		tmp = ReadRegister(REG_SSDN_P1_8_11);
		tmp &= ~SSDN_P1_8_11_MASK;
		tmp |= (uint8_t)(SSDN_P1_8_11_MASK & (SSDN_P1 >> 8));
		WriteRegister(REG_SSDN_P1_8_11, tmp);

		//write SSDN parameter P2
		tmp = (uint8_t) SSDN_P2;
		WriteRegister(REG_SSDN_P2_0_7, tmp);
		tmp = ReadRegister(REG_SSDN_P2_8_14);
		tmp &= ~SSDN_P2_8_14_MASK;
		tmp |= (uint8_t)(SSDN_P2_8_14_MASK & (SSDN_P2 >> 8));
		WriteRegister(REG_SSDN_P2_8_14, tmp);

		//write SSDN parameter P3
		tmp = (uint8_t) SSDN_P3;
		WriteRegister(REG_SSDN_P3_0_7, tmp);
		tmp = ReadRegister(REG_SSDN_P3_8_14);
		tmp &= ~SSDN_P3_8_14_MASK;
		tmp |= (uint8_t)(SSDN_P3_8_14_MASK & (SSDN_P3 >> 8));
		WriteRegister(REG_SSDN_P3_8_14, tmp);

		//turn on SS if it should be enabled
		if ((this->SS.SS_Enable == ON)
				& (((this->PLL[0].PLL_Multiplier_Integer & 0x01) != 0)
						| (this->PLL[0].PLL_Multiplier_Numerator != 0)))
		{
			tmp = ReadRegister(REG_SSC_EN);
			tmp |= SSC_EN_MASK;
			WriteRegister(REG_SSC_EN, tmp);
		}
	}

	void Si5351Driver::MSConfig(MSChannel MS_Channel) {
		MSConfig2(MS_Channel);
		MSSourceConfig(MS_Channel);
	}
	void Si5351Driver::MSConfig2(MSChannel MS_Channel) {
		auto div = this->MS[MS_Channel].MS_Divider_Integer;
		auto num = this->MS[MS_Channel].MS_Divider_Numerator;
		auto denom = this->MS[MS_Channel].MS_Divider_Denominator;
		auto rdiv = (int) this->CLK[MS_Channel].CLK_R_Div;
		uint8_t dat;

		uint32_t P1;
		uint32_t P2;
		uint32_t P3;
		uint32_t div4 = 0;

		/* Output Multisynth Divider Equations
		* where: a = div, b = num and c = denom
		* P1 register is an 18-bit value using following formula:
		* 	P1[17:0] = 128 * a + floor(128*(b/c)) - 512
		* P2 register is a 20-bit value using the following formula:
		* 	P2[19:0] = 128 * b - c * floor(128*(b/c))
		* P3 register is a 20-bit value using the following formula:
		* 	P3[19:0] = c
		*/
		/* Set the main PLL config registers */
		if (div == 4) {
			div4 = MS_DIVBY4_MASK;
			P1 = P2 = 0;
			P3 = 1;
		} else if (num == 0) {
			/* Integer mode */
			P1 = 128 * div - 512;
			P2 = 0;
			P3 = 1;
		} else {
			/* Fractional mode */
			P1 = 128 * div + ((128 * num) / denom) - 512;
			P2 = 128 * num - denom * ((128 * num) / denom);
			P3 = denom;
		}

		/* Set the MSx config registers */
		uint8_t regBase = 42 + 8 * MS_Channel;
		uint8_t reg[9];
		reg[0] = regBase;
		reg[1] = (P3 & 0x0000FF00) >> 8;
		reg[2] = (P3 & 0x000000FF);
		reg[3] = ((P1 & 0x00030000) >> 16) | div4 | rdiv;
		reg[4] = (P1 & 0x0000FF00) >> 8;
		reg[5] = (P1 & 0x000000FF);
		reg[6] = ((P3 & 0x000F0000) >> 12) | ((P2 & 0x000F0000) >> 16);
		reg[7] = (P2 & 0x0000FF00) >> 8;
		reg[8] = (P2 & 0x000000FF);
		WriteRegisters(reg, 9);
	}
	void Si5351Driver::MSSourceConfig(MSChannel MS_Channel) {
		//configure MultiSynth clock source
		uint8_t tmp = ReadRegister(REG_MS_SRC + MS_Channel);
		tmp &= ~MS_SRC_MASK;
		if (MS[MS_Channel].MS_Clock_Source == MS_Clock_Source_PLLB) {
			tmp |= MS_SRC_MASK;
		}
		WriteRegister(REG_MS_SRC + MS_Channel, tmp);
	}

	void Si5351Driver::CLKPowerCmd(CLKChannel CLK_Channel)
	{
		uint8_t tmp;
		if (this->CLK[CLK_Channel].CLK_Enable == ON)
		{
			//power up output driver
			tmp = ReadRegister(REG_CLK_PDN + CLK_Channel);
			tmp &= ~CLK_PDN_MASK;
			WriteRegister(REG_CLK_PDN + CLK_Channel, tmp);
			//power up the clock
			tmp = ReadRegister(REG_CLK_EN);
			tmp &= ~(1 << CLK_Channel);
			WriteRegister(REG_CLK_EN, tmp);
		} else {
			//power down the clock
			tmp = ReadRegister(REG_CLK_EN);
			tmp |= 1 << CLK_Channel;
			WriteRegister(REG_CLK_EN, tmp);
			//power down output driver
			tmp = ReadRegister(REG_CLK_PDN + CLK_Channel);
			tmp |= CLK_PDN_MASK;
			WriteRegister(REG_CLK_PDN + CLK_Channel, tmp);
		}
	}

	void Si5351Driver::CLKConfig(CLKChannel CLK_Channel)
	{
		uint8_t tmp, tmp_mask;

		//set CLK source clock
		tmp = ReadRegister(REG_CLK_SRC + CLK_Channel);
		tmp &= ~CLK_SRC_MASK;
		tmp |= CLK_SRC_MASK & this->CLK[CLK_Channel].CLK_Clock_Source;
		WriteRegister(REG_CLK_SRC + CLK_Channel, tmp);

		//set CLK inversion
		tmp = ReadRegister(REG_CLK_INV + CLK_Channel);
		tmp &= ~CLK_INV_MASK;
		if (this->CLK[CLK_Channel].CLK_Invert == ON)
		{
			tmp |= CLK_INV_MASK;
		}
		WriteRegister(REG_CLK_INV + CLK_Channel, tmp);

		//set CLK disable state
		tmp = ReadRegister(REG_CLK_DIS_STATE + (CLK_Channel >> 2)); //increment the address by 1 if CLKx>=CLK4
		tmp_mask = CLK_DIS_STATE_MASK << ((CLK_Channel & 0x03)<<1); //shift the mask according to the selected channel
		tmp &= ~tmp_mask;
		tmp |= tmp_mask & ((this->CLK[CLK_Channel].CLK_Disable_State) << ((CLK_Channel & 0x03)<<1));
		WriteRegister(REG_CLK_DIS_STATE + (CLK_Channel >> 2), tmp);

		//set CLK current drive
		tmp = ReadRegister(REG_CLK_IDRV + CLK_Channel);
		tmp &= ~CLK_IDRV_MASK;
		tmp |= CLK_IDRV_MASK & this->CLK[CLK_Channel].CLK_I_Drv;
		WriteRegister(REG_CLK_IDRV + CLK_Channel, tmp);

		//set OEB pin
		tmp = ReadRegister(REG_CLK_OEB);
		tmp_mask = 1 << CLK_Channel;
		tmp &= ~tmp_mask;
		if (this->CLK[CLK_Channel].CLK_Use_OEB_Pin == OFF)
		{
			tmp |= tmp_mask;
		}

		if (CLK_Channel <= CLK5) //CLK6 and 7 are integer only, which causes several limitations
		{
			//set CLK phase offset
			tmp = CLK_PHOFF_MASK & (this->CLK[CLK_Channel].CLK_QuarterPeriod_Offset);
			WriteRegister(REG_CLK_PHOFF + CLK_Channel, tmp);
			//set Rx divider
			tmp = ReadRegister(REG_CLK_R_DIV + CLK_Channel * CLK_R_DIV_STEP);
			tmp &= ~CLK_R_DIV_MASK;
			tmp |= CLK_R_DIV_MASK & (this->CLK[CLK_Channel].CLK_R_Div);
			WriteRegister(REG_CLK_R_DIV + CLK_Channel * CLK_R_DIV_STEP, tmp);
		} else {
			//CLK6 and CLK7 have no fractional mode, so they lack the phase offset function

			//set Rx divider
			tmp_mask = CLK_R67_DIV_MASK << ((CLK_Channel-CLK6) << 2); //shift mask left by 4 if CLK7
			tmp = ReadRegister(REG_CLK_R67_DIV);
			tmp &= ~tmp_mask;
			tmp |= tmp_mask & ((this->CLK[CLK_Channel].CLK_R_Div >> 4) << ((CLK_Channel-CLK6) << 2));
			WriteRegister(REG_CLK_R67_DIV, tmp);
		}
	}

	int Si5351Driver::Init()
	{
		uint32_t timeout = SI5351_TIMEOUT;
		uint8_t i;

		//wait for the 5351 to initialize
		while (CheckStatusBit(StatusBit_SysInit))
		{
			timeout--;
			if (timeout==0) return 1; //return 1 if initialization timed out
		}

		//configure oscillator, fanout, interrupts, VCXO
		OSCConfig();
		InterruptConfig();

		//configure PLLs
		for (i=PLL_A; i<=PLL_B; i++)
		{
			PLLConfig((PLLChannel) i);
			PLLReset((PLLChannel) i);
		}

		//configure Spread Spectrum
		SSConfig();

		//Configure Multisynths
		for (i=MS0; i<=MS7; i++)
		{
			MSConfig((MSChannel) i);
		}

		//configure outputs
		for (i=CLK0; i<=CLK7; i++)
		{
			CLKConfig((CLKChannel) i);
		}

		//wait for PLLs to lock
		while (CheckStatusBit(
			StatusBit(StatusBit_SysInit | StatusBit_PLLA | StatusBit_PLLB)))
		{
			timeout--;
			if (timeout==0) return 1; //return 1 if problem with any PLL
		}
		//clear sticky bits
		ClearStickyBit(
			StatusBit(StatusBit_SysInit | StatusBit_PLLA | StatusBit_PLLB));

		if (this->f_CLKIN != 0) //if CLKIN used, check it as well
		{
			while (CheckStatusBit(StatusBit_CLKIN))
			{
				timeout--;
				if (timeout==0) return 1; //return 1 if initialization timed out
			}
			//clear CLKIN sticky bit
			ClearStickyBit(
				StatusBit(StatusBit_SysInit | StatusBit_PLLA | StatusBit_PLLB));
		}

		//power on or off the outputs
		for (i=CLK0; i<=CLK7; i++)
		{
			CLKPowerCmd((CLKChannel) i);
		}
		
		this->val_REG_PLL_RESET = ReadRegister(REG_PLL_RESET);

		return 0;
	}


}
}
