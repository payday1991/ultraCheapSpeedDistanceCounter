#include "include/aht20.h"

/*
 * check for 32-bit event codes
 */
static uint8_t i2c_chk_evt(uint32_t event_mask)
{
	/* read order matters here! STAR1 before STAR2!! */
	uint32_t status = I2C1->STAR1 | (I2C1->STAR2<<16);
	return (status & event_mask) == event_mask;
} 



/**
 * @brief Performs I2C read or write operation.
 *
 * This function sends or receives data over I2C bus based on the provided parameters.
 *
 * @param addr The 7-bit I2C address of the device.
 * @param data Pointer to the data buffer.
 * @param sz The size of the data buffer.
 * @param rw Specifies whether to perform a read or write operation. Use the READ constant for read operation and WRITE constant for write operation.
 *
 * @return Returns 0 on success, or an error code if an error occurred.
 */
static bool i2cReadOrWrite(uint8_t addr, uint8_t *data, uint8_t sz, bool rw)
{
	int32_t timeout;
	
	// wait for not busy
	timeout = TIMEOUT_MAX;
	while((I2C1->STAR2 & I2C_STAR2_BUSY) && (timeout--));
	if(timeout==-1)
		return ERROR;

	// Set START condition
	I2C1->CTLR1 |= I2C_CTLR1_START;
	
	// wait for master mode select
	timeout = TIMEOUT_MAX;
	while((!i2c_chk_evt(I2C_EVENT_MASTER_MODE_SELECT)) && (timeout--));
	if(timeout==-1)
		return ERROR;

	if (rw == READ) //Read
	{ 
		// send 7-bit address + read flag
		I2C1->DATAR = (addr<<1)+1;
        
        if (sz > 1) I2C1->CTLR1 |= I2C_CTLR1_ACK;

		// wait for transmit condition
		timeout = TIMEOUT_MAX;
		while((!i2c_chk_evt(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) && (timeout--));
		if(timeout==-1)
			return ERROR;
        
		// get data one byte at a time
		while(sz--)
		{
			if (!sz) I2C1->CTLR1 &= ~I2C_CTLR1_ACK; //signal it's the last byte
            while(!(I2C1->STAR1 & I2C_FLAG_RXNE) && (timeout--))
			if(timeout==-1) return ERROR;
				
            *data++ = I2C1->DATAR;
		}
        I2C1->CTLR1 |= I2C_CTLR1_STOP;	// set STOP condition
	}
	else //Write
	{ 
		// send 7-bit address + write flag
		I2C1->DATAR = addr<<1;

		// wait for transmit condition
		timeout = TIMEOUT_MAX;
		while((!i2c_chk_evt(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) && (timeout--));
		if(timeout==-1)
			return ERROR;

		// send data one byte at a time
		while(sz--)
		{
			// wait for TX Empty
			timeout = TIMEOUT_MAX;
			while(!(I2C1->STAR1 & I2C_STAR1_TXE) && (timeout--));
			if(timeout==-1)
				return ERROR;
			
			// send command
			I2C1->DATAR = *data++;
		}
        // wait for tx complete
        timeout = TIMEOUT_MAX;
        while((!i2c_chk_evt(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) && (timeout--));
        if(timeout==-1)
            return ERROR;

        // set STOP condition
        I2C1->CTLR1 |= I2C_CTLR1_STOP;
	}
	return 0;
}


bool aht20read(int8_t *tem, uint8_t *hum)
{
	uint8_t sendbffer[3]={0xAC,0x33,0x00};
	uint8_t readbuffer[6]; 

	if(i2cReadOrWrite(AHT20_I2C_ADDR, sendbffer, 3, WRITE)) return 1; //error
	Delay_Ms(100);
	if(i2cReadOrWrite(AHT20_I2C_ADDR, readbuffer, 6, READ)) return 1; //error 
	
	volatile uint32_t data=0;
	volatile uint64_t tmpData=0;
	//Humidity
	data=((uint32_t)readbuffer[3]>>4) +
			((uint32_t)readbuffer[2]<<4) +
			((uint32_t)readbuffer[1]<<12);
	tmpData = (uint64_t)data<<18; 
	tmpData = ((tmpData)*25)>>18*2; //*hum = data*100.0f/(1<<20);
	*hum = (uint8_t)tmpData;
	//Temperature
	data=((uint32_t)(readbuffer[3] & 0x0F)<<16) +
					((uint32_t)readbuffer[4]<<8) +
					((uint32_t)readbuffer[5]); 
	tmpData = (uint64_t)data<<17; //*tem=data*200.0f/(1<<20)-50; 
	tmpData = (((tmpData)*25)>>17*2)-50;
	*tem = (int8_t)tmpData;

	return OK;
}


bool aht20init(void)
{ 
	uint8_t sendbffer[3]={0xBA,0x08,0x00};
    if(i2cReadOrWrite(AHT20_I2C_ADDR, sendbffer, 1, WRITE)) return ERROR; //error 
	Delay_Ms(40); 
    sendbffer[0]=0xBE;
    if(i2cReadOrWrite(AHT20_I2C_ADDR, sendbffer, 3, WRITE)) return ERROR; //error 
    Delay_Ms(15);
    return OK;
}