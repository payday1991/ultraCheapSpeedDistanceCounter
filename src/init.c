#include "ch32v003fun/ch32v003fun.h"
#include "extralibs/ch32v003_GPIO_branchless.h"
#include "include/init.h"
#include "include/main.h"
#include "lcd/glcd.h"


/**
 * @brief Enable clocks for the peripherials we need
 * 
 */
static inline void clockInit(void)
{
	// Enable DMA CLK
	RCC->AHBPCENR |= RCC_AHBPeriph_DMA1;

	#if defined(USE_TEMPERATURE_HUMIDITY_SENSOR) || defined(USE_EXTERNAL_FLASH)
	RCC->APB1PCENR |= RCC_APB1Periph_I2C1; //I2C1 clock
	#endif
	//SPI1, TIM1 CLK and alternate IO function module clock, GPIO's and ADC
	RCC->APB2PCENR |= RCC_APB2Periph_SPI1 | RCC_APB2Periph_TIM1 | RCC_AFIOEN | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_ADC1;;
}


/**
 * @brief Set up SysTick timer and it's interrupts
 * 
 */
inline void systick_init(void)
{
	/* disable default SysTick behavior */
	SysTick->CTLR = 0;
	
	/* enable the SysTick IRQ */
	NVIC_EnableIRQ(SysTicK_IRQn);
	
	/* Set the tick interval to 1ms for normal op */
	SysTick->CMP = (FUNCONF_SYSTEM_CORE_CLOCK/1000)-1;
	
	/* Start at zero */
	SysTick->CNT = 0;
	sysTickCnt = 0;
	
	/* Enable SysTick counter, IRQ, HCLK/1 */
	SysTick->CTLR = SYSTICK_CTLR_STE | SYSTICK_CTLR_STIE |
					SYSTICK_CTLR_STCLK;
}


/**
 * @brief Does what it says on the tin. Init all the GPIO's
 * 
 */
static inline void gpioInit (void) 
{
	/*Battery charge enable*/
	GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_D, BATTERY_CHARGE_GPIO_NUM), GPIO_pinMode_O_pushPull, GPIO_Speed_2MHz);

	/*Button UP*/
	GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_D, BUTTON_UP_GPIO_NUM), GPIO_pinMode_I_pullDown, GPIO_Speed_In);

	/*Button DOWN*/
	GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_C, BUTTON_DOWN_GPIO_NUM), GPIO_pinMode_I_pullDown, GPIO_Speed_In);

	/*Encoder input A(PD4 with interrupt)*/
	GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_D, HALL_INPUT_A_GPIO_NUM), GPIO_pinMode_I_pullDown, GPIO_Speed_In); 
	AFIO->EXTICR |= (uint32_t)(0b11 << (HALL_INPUT_A_GPIO_NUM*2));
	EXTI->INTENR |= 1<<HALL_INPUT_A_GPIO_NUM ; // Enable and EXT4 
	EXTI->FTENR |= EXTI_Line4;
	NVIC_EnableIRQ( EXTI7_0_IRQn );

	asm volatile(
	#if __GNUC__ > 10
			".option arch, +zicsr\n"
	#endif
			"addi t1, x0, 3\n"
			"csrrw x0, 0x804, t1\n"
			: : :  "t1" );
	
	/*Encoder input B(PD3 without interrupt)*/
	GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_D, HALL_INPUT_B_GPIO_NUM), GPIO_pinMode_I_pullDown, GPIO_Speed_In); 

	/*USB Voltage Sense*/
	GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_A, USB_V_SENSE_GPIO_NUM), GPIO_pinMode_I_pullDown, GPIO_Speed_In);

	/*Boost Enable*/ 
	BOOST_ENABLE_GPIO_PORT->CFGLR &= ~(0xf<<(4*BOOST_ENABLE_GPIO_NUM));
	BOOST_ENABLE_GPIO_PORT->CFGLR |= (GPIO_Speed_2MHz | GPIO_CNF_OUT_PP)<<(4*BOOST_ENABLE_GPIO_NUM);

	/*Battery Voltage Sense*/
	GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_A, BATTERY_V_SENSE_GPIO_NUM), GPIO_pinMode_I_analog, GPIO_Speed_In);

	/*Temperature sensor in Voltage Sense*/
	GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_D, TEMPERATURE_SENSOR_GPIO_NUM), GPIO_pinMode_I_analog, GPIO_Speed_In); 

	/*LCD Backlight*/
	LCD_BACKLIGHT_GPIO_PORT->CFGLR &= ~(0xf<<(4*LCD_BACKLIGHT_GPIO_NUM));
	LCD_BACKLIGHT_GPIO_PORT->CFGLR |= (GPIO_Speed_2MHz | GPIO_CNF_OUT_OD_AF)<<(4*LCD_BACKLIGHT_GPIO_NUM); 

	/*LCD SPI*/
	LCD_DI_GPIO_PORT->CFGLR &= ~(0xf<<(4*LCD_DI_GPIO_NUM));
	LCD_DI_GPIO_PORT->CFGLR |= (GPIO_Speed_2MHz | GPIO_CNF_OUT_PP_AF)<<(4*LCD_DI_GPIO_NUM);

	LCD_CLK_GPIO_PORT->CFGLR &= ~(0xf<<(4*LCD_CLK_GPIO_NUM));
	LCD_CLK_GPIO_PORT->CFGLR |= (GPIO_Speed_2MHz | GPIO_CNF_OUT_PP_AF)<<(4*LCD_CLK_GPIO_NUM);

	LCD_RESET_GPIO_PORT->CFGLR &= ~(0xf<<(4*LCD_RESET_GPIO_NUM));
	LCD_RESET_GPIO_PORT->CFGLR |= (GPIO_Speed_2MHz | GPIO_CNF_OUT_PP)<<(4*LCD_RESET_GPIO_NUM);

	LCD_DC_GPIO_PORT->CFGLR &= ~(0xf<<(4*LCD_DC_GPIO_NUM));
	LCD_DC_GPIO_PORT->CFGLR |= (GPIO_Speed_2MHz | GPIO_CNF_OUT_PP)<<(4*LCD_DC_GPIO_NUM);

	/*I2C GPIO*/
	
	// PC1 is SDA, 10MHz Output, alt func, open-drain
	GPIOC->CFGLR &= ~(0xf<<(4*1));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_OD_AF)<<(4*1);
	
	// PC2 is SCL, 10MHz Output, alt func, open-drain
	GPIOC->CFGLR &= ~(0xf<<(4*2));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_OD_AF)<<(4*2);
} 



/**
 * @brief Init TIM1 to do the PWM output
 * 
 */
static inline void backlightPWMinit(void)
{ 
	// Reset TIM1 to init all regs
	RCC->APB2PRSTR |= RCC_APB2Periph_TIM1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_TIM1;
	
	// Prescaler 
	TIM1->PSC = 0x0004;
	
	// Auto Reload - sets period
	TIM1->ATRLR = 255;
	
	// Reload immediately
	TIM1->SWEVGR |= TIM_UG;
	
	// Enable CH1 output, positive pol
	TIM1->CCER |= TIM_CC1E | TIM_CC1P;
	
	// CH1 Mode is output, PWM1 (CC1S = 00, OC1M = 110)
	TIM1->CHCTLR1 |= TIM_OC1M_2 | TIM_OC1M_1; 
	
	// Set the Capture Compare Register value to 50% initially
	TIM1->CH1CVR = 128; 
	
	// Enable TIM1 outputs
	TIM1->BDTR |= TIM_MOE;
	
	// Enable TIM1
	TIM1->CTLR1 |= TIM_CEN;

	//Turn off the backlight
	TIM1->CH1CVR = 0;
}


/**
 * @brief Init SPI for LCD
 * 
 */
static inline void spiInit(void)
{ 
	// Configure SPI 
	SPI1->CTLR1 = 
		SPI_NSS_Soft | SPI_CPHA_1Edge | SPI_CPOL_Low | SPI_DataSize_8b |
		SPI_Mode_Master | SPI_Direction_1Line_Tx |
		SPI_BaudRatePrescaler_16;

	// enable SPI port
	SPI1->CTLR1 |= CTLR1_SPE_Set; 
}

static inline void adcInit (void)
{
	RCC->CFGR0 &= ~RCC_ADCPRE;  // Clear out the bis in case they were set
    RCC->CFGR0 |= RCC_ADCPRE_DIV6;	// The supply voltage is just a 3v. Set ADC clock to 6MHz.

	// Reset the ADC to init all regs
	RCC->APB2PRSTR |= RCC_APB2Periph_ADC1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_ADC1;

	// Set up single conversion on chl 0
	ADC1->RSQR1 = 0;	
	ADC1->RSQR2 = 0;
	ADC1->RSQR3 = (5<<(5*0)) | (0<<(5*1));	// 0-9 for 8 ext inputs and two internals

	// set sampling time for ch 0
	// ADC1->SAMPTR2 &= ~(ADC_SMP0<<(3*0));
	// ADC1->SAMPTR2 |= 7<<(3*0);	// 0:7 => 3/9/15/30/43/57/73/241 cycles
	ADC1->SAMPTR2 = (3<<(3*5)) | (3<<(3*0));

	// turn on ADC and set rule group to sw trig
	ADC1->CTLR2 |= ADC_ADON | ADC_EXTSEL;

	// Reset calibration
	ADC1->CTLR2 |= ADC_RSTCAL;
	while(ADC1->CTLR2 & ADC_RSTCAL);
	
	// Calibrate
	ADC1->CTLR2 |= ADC_CAL;
	while(ADC1->CTLR2 & ADC_CAL);
	// should be ready for SW conversion now
}

 void i2cInit(void)
{
	uint16_t tempreg;
	
	// Reset I2C1 to init all regs
	RCC->APB1PRSTR |= RCC_APB1Periph_I2C1;
	RCC->APB1PRSTR &= ~RCC_APB1Periph_I2C1;
	
	// set freq
	tempreg = I2C1->CTLR2;
	tempreg &= ~I2C_CTLR2_FREQ;
	tempreg |= (FUNCONF_SYSTEM_CORE_CLOCK/I2C_PRERATE)&I2C_CTLR2_FREQ;
	I2C1->CTLR2 = tempreg;
	
	// Set clock config
	tempreg = 0;
	// standard mode good to 100kHz
	tempreg = (FUNCONF_SYSTEM_CORE_CLOCK/(2*I2C_CLKRATE))&I2C_CTLR2_FREQ; 
	I2C1->CKCFGR = tempreg; 
	
	// Enable I2C
	I2C1->CTLR1 |= I2C_CTLR1_PE;

	// set ACK mode
	I2C1->CTLR1 |= I2C_CTLR1_ACK;
}



static inline void iwdgInit(uint16_t reload_val, uint8_t prescaler) {
	IWDG->CTLR = 0x5555;
	IWDG->PSCR = prescaler; 
	IWDG->CTLR = 0x5555;
	IWDG->RLDR = reload_val & 0xfff; 
	IWDG->CTLR = 0xCCCC;
}

inline void iwdgFeed() {
	IWDG->CTLR = 0xAAAA;
}

/**
 * @brief Init all the peripherials
 * 
 */
inline void  init (void) {
    SystemInit(); 
    clockInit(); 
	gpioInit();

	//Have to keep the Boost Enable pin high to enable the boost converter. Otherwise everything will be off.
	BOOST_ENABLE_GPIO_PORT->BSHR = (1 << BOOST_ENABLE_GPIO_NUM);

	backlightPWMinit();
	spiInit();
	glcd_init();
	adcInit();
	#if defined(USE_TEMPERATURE_HUMIDITY_SENSOR) || defined(USE_EXTERNAL_FLASH)
	i2cInit();
	#endif
	iwdgInit(0xfff, IWDG_Prescaler_256); // set up watchdog to about 2 s
}