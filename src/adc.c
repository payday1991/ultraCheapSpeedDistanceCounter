#include "include/adc.h"



uint16_t adcGet( uint8_t channel )
{
	ADC1->RSQR3 = channel;	// 0-9 for 8 ext inputs and two internals
	uint16_t adcAverage=0;
	for (uint8_t i = 0; i < NUM_ADC_SAMPLES; i++)
	{
		// start sw conversion (auto clears)
		ADC1->CTLR2 |= ADC_SWSTART; 
		// wait for conversion complete
		while(!(ADC1->STATR & ADC_EOC));
		adcAverage+= ADC1->RDATAR;
	} 
	// return result
	return adcAverage>>2;
}