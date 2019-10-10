#include <mculib/dma_adc.hpp>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/rcc.h>
#include <stdlib.h>

namespace mculib {

static rcc_periph_clken ADC_RCC(ADCDevice dev) {
	if(dev == ADC1)
		return RCC_ADC1;
	if(dev == ADC2)
		return RCC_ADC2;
	abort();
}

void DMAADC::init(uint8_t* channels, int nChannels) {

	// set up dma
	DMATransferParams srcParams, dstParams;
	srcParams.address = &ADC_DR(adcDevice);
	srcParams.bytesPerWord = 2;
	srcParams.increment = false;

	dstParams.address = buffer;
	dstParams.bytesPerWord = 2;
	dstParams.increment = true;

	dma.enable();
	dma.setTransferParams(srcParams, dstParams, DMADirection::PERIPHERAL_TO_MEMORY,
							bufferSizeBytes/2, true);


	rcc_periph_clock_enable(ADC_RCC(adcDevice));

	adc_power_off(adcDevice);
	adc_enable_scan_mode(adcDevice);
	adc_set_continuous_conversion_mode(adcDevice);
	adc_disable_external_trigger_regular(adcDevice);
	adc_set_right_aligned(adcDevice);
	
	// from section 11.6 of stm32f1 reference manual:
	// The total conversion time is calculated as follows:
	// Tconv = Sampling time + 12.5 cycles
	adc_set_sample_time_on_all_channels(adcDevice, ADC_SMPR_SMP_7DOT5CYC);
	
	adc_set_regular_sequence(adcDevice, nChannels, channels);
	adc_enable_dma(adcDevice);
}
void DMAADC::deinit() {
	stop();
	rcc_periph_clock_disable(ADC_RCC(adcDevice));
	dma.disable();
}
void DMAADC::start() {
	adc_power_on(adcDevice);
	/* Wait for ADC starting up. */
	int i;
	for (i = 0; i < 800000; i++) /* Wait a bit. */
		__asm__("nop");
	
	adc_reset_calibration(adcDevice);
	adc_calibrate(adcDevice);
	adc_enable_temperature_sensor();
	adc_start_conversion_direct(adcDevice);
	dma.start();
}
void DMAADC::stop() {
	dma.stop();
	adc_power_off(adcDevice);
}

}
