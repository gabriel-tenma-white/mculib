#include <mculib/dma_adc.hpp>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/rcc.h>
#include <stdlib.h>

namespace mculib {

static rcc_periph_clken ADC_RCC(ADCDevice dev) {
	if(dev == ADC1)
		return RCC_ADC1;
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


	// F0 requires adc to be off when calibrating,
	// but F1 requires adc to be on.
	// on F0 running calibration seems to generate spurious dma requests,
	// so calibration has to be done before enabling dma.
	adc_calibrate(adcDevice);

	//adc_enable_scan_mode(adcDevice);
	adc_set_continuous_conversion_mode(adcDevice);
	adc_disable_external_trigger_regular(adcDevice);
	adc_set_right_aligned(adcDevice);
	adc_enable_temperature_sensor();
	
	// from section 11.6 of stm32f1 reference manual:
	// The total conversion time is calculated as follows:
	// Tconv = Sampling time + 12.5 cycles
	adc_set_sample_time_on_all_channels(adcDevice, ADC_SMPR_SMP_071DOT5);
	
	adc_set_regular_sequence(adcDevice, nChannels, channels);
	adc_enable_dma(adcDevice);
	adc_enable_dma_circular_mode(adcDevice);
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

	dma.start();
	adc_start_conversion_regular(adcDevice);
}
void DMAADC::stop() {
	dma.stop();
	adc_power_off(adcDevice);
}

}
