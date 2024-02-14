/*
	Copyright (c) 2012, Andy Gock

	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
		* Redistributions of source code must retain the above copyright
		  notice, this list of conditions and the following disclaimer.
		* Redistributions in binary form must reproduce the above copyright
		  notice, this list of conditions and the following disclaimer in the
		  documentation and/or other materials provided with the distribution.
		* Neither the name of Andy Gock nor the
		  names of its contributors may be used to endorse or promote products
		  derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL ANDY GOCK BE LIABLE FOR ANY
	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#define GLCD_DEVICE_CH32V003
#if defined(GLCD_DEVICE_CH32V003)

#include "ch32v003fun/ch32v003fun.h"

#include "include/main.h"
/* Includes from GLCD */
#include "../glcd.h"

void glcd_init(void)
{ 
	glcd_reset();

	glcd_ST7565R_init(); 

	glcd_set_start_line(0);
	glcd_clear_now();

	glcd_select_screen((uint8_t *)&glcd_buffer,&glcd_bbox);
	glcd_clear();
}

void glcd_spi_write(uint8_t c)
{ 
	// wait for TXE
	while(!(SPI1->STATR & SPI_STATR_TXE));
	// Send byte
	SPI1->DATAR = c; 
	// wait for not busy before exiting
	while(SPI1->STATR & SPI_STATR_BSY); 
}

void glcd_reset(void)
{
	/* Toggle RST low to reset. Minimum pulse 100ns on datasheet. */ 
	/* Reset the chip */ 
	LCD_RESET_GPIO_PORT->BSHR = (1 << (16 + LCD_RESET_GPIO_NUM));
	Delay_Ms(100); 
	LCD_RESET_GPIO_PORT->BSHR = (1 << LCD_RESET_GPIO_NUM); 
}

#endif
