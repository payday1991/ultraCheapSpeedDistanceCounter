#pragma once

#include "main.h"
#include "machineData.h"

/**
 * @brief Read an ADC input in polling mode.
 * 
 * @return uint16_t ADC reading averaged over 4 samples. 
 */
uint16_t adcGet( uint8_t channel );