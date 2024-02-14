#pragma once

void init (void);
void systick_init(void);
void iwdgFeed(void);


/**
 * @brief Systick stuff
 * 
 */ 
/* some bit definitions for systick regs */
#define SYSTICK_SR_CNTIF (1<<0)
#define SYSTICK_CTLR_STE (1<<0)
#define SYSTICK_CTLR_STIE (1<<1)
#define SYSTICK_CTLR_STCLK (1<<2)
#define SYSTICK_CTLR_STRE (1<<3)
#define SYSTICK_CTLR_SWIE (1<<31)

// I2C Bus clock rate - must be lower the Logic clock rate
#define I2C_CLKRATE 100000 
// I2C Logic clock rate - must be higher than Bus clock rate
#define I2C_PRERATE 200000 