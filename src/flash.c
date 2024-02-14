/********************************** (C) COPYRIGHT  *******************************
 * File Name          : ch32v00x_flash.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2022/08/08
 * Description        : This file provides all the FLASH firmware functions.
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 ***************************************************************************************/

#include "include/flash.h"
#include "include/main.h"

/* Flash Access Control Register bits */
#define ACR_LATENCY_Mask           ((uint32_t)0x00000038)

/* Flash Control Register bits */
#define CR_PG_Set                  ((uint32_t)0x00000001)
#define CR_PG_Reset                ((uint32_t)0xFFFFFFFE)
#define CR_PER_Set                 ((uint32_t)0x00000002)
#define CR_PER_Reset               ((uint32_t)0xFFFFFFFD)
#define CR_MER_Set                 ((uint32_t)0x00000004)
#define CR_MER_Reset               ((uint32_t)0xFFFFFFFB)
#define CR_OPTPG_Set               ((uint32_t)0x00000010)
#define CR_OPTPG_Reset             ((uint32_t)0xFFFFFFEF)
#define CR_OPTER_Set               ((uint32_t)0x00000020)
#define CR_OPTER_Reset             ((uint32_t)0xFFFFFFDF)
#define CR_STRT_Set                ((uint32_t)0x00000040)
#define CR_LOCK_Set                ((uint32_t)0x00000080)
#define CR_PAGE_PG                 ((uint32_t)0x00010000)
#define CR_PAGE_ER                 ((uint32_t)0x00020000)
#define CR_BUF_LOAD                ((uint32_t)0x00040000)
#define CR_BUF_RST                 ((uint32_t)0x00080000)

/* FLASH Status Register bits */
#define SR_BSY                     ((uint32_t)0x00000001)
#define SR_WRPRTERR                ((uint32_t)0x00000010)
#define SR_EOP                     ((uint32_t)0x00000020)

/* FLASH Mask */
#define RDPRT_Mask                 ((uint32_t)0x00000002)
#define WRP0_Mask                  ((uint32_t)0x000000FF)
#define WRP1_Mask                  ((uint32_t)0x0000FF00)
#define WRP2_Mask                  ((uint32_t)0x00FF0000)
#define WRP3_Mask                  ((uint32_t)0xFF000000)

/* FLASH Keys */
#define RDP_Key                    ((uint16_t)0x00A5)
#define FLASH_KEY1                 ((uint32_t)0x45670123)
#define FLASH_KEY2                 ((uint32_t)0xCDEF89AB)

/* FLASH BANK address */
#define FLASH_BANK1_END_ADDRESS    ((uint32_t)0x807FFFF)

/* Delay definition */
#define EraseTimeout               ((uint32_t)0x000B0000)
#define ProgramTimeout             ((uint32_t)0x00002000)

/* Flash Program Vaild Address */
#define ValidAddrStart             (FLASH_BASE)
#define ValidAddrEnd               (FLASH_BASE + 0x4000) 

/*********************************************************************
 * @fn         FLASH_ProgramHalfWord
 *
 * @brief       Programs a half word at a specified address.
 *
 * @param       Address - specifies the address to be programmed.
 *              Data - specifies the data to be programmed.
 *
 * @return      FLASH Status - The returned value can be: FLASH_BUSY, FLASH_ERROR_PG,
 *             FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
 */
inline FLASH_Status FLASH_ProgramHalfWord(uint32_t Address, uint16_t Data)
{
    FLASH_Status status = FLASH_COMPLETE;

    status = FLASH_WaitForLastOperation(ProgramTimeout);
    if(status == FLASH_COMPLETE)
    {
        FLASH->CTLR |= CR_PG_Set;
        *(__IO uint16_t *)Address = Data;
        status = FLASH_WaitForLastOperation(ProgramTimeout);
        FLASH->CTLR &= CR_PG_Reset;
    }

    return status;
}


/*********************************************************************
 * @fn      FLASH_GetBank1Status
 *
 * @brief   Returns the FLASH Bank1 Status.
 *
 * @return  FLASH Status - The returned value can be: FLASH_BUSY, FLASH_ERROR_PG,
 *        FLASH_ERROR_WRP or FLASH_COMPLETE.
 */
FLASH_Status FLASH_GetBank1Status(void)
{
    FLASH_Status flashstatus = FLASH_COMPLETE;

    if((FLASH->STATR & FLASH_FLAG_BANK1_BSY) == FLASH_FLAG_BSY)
    {
        flashstatus = FLASH_BUSY;
    }
    else
    {
        if((FLASH->STATR & FLASH_FLAG_BANK1_WRPRTERR) != 0)
        {
            flashstatus = FLASH_ERROR_WRP;
        }
        else
        {
            flashstatus = FLASH_COMPLETE;
        }
    }
    return flashstatus;
}

/*********************************************************************
 * @fn      FLASH_WaitForLastOperation
 *
 * @brief   Waits for a Flash operation to complete or a TIMEOUT to occur.
 *
 * @param   Timeout - FLASH programming Timeout
 *
 * @return  FLASH Status - The returned value can be: FLASH_BUSY, FLASH_ERROR_PG,
 *        FLASH_ERROR_WRP or FLASH_COMPLETE.
 */
FLASH_Status FLASH_WaitForLastOperation(uint32_t Timeout)
{
    FLASH_Status status = FLASH_COMPLETE;

    status = FLASH_GetBank1Status();
    while((status == FLASH_BUSY) && (Timeout != 0x00))
    {
        status = FLASH_GetBank1Status();
        Timeout--;
    }
    if(Timeout == 0x00)
    {
        status = FLASH_TIMEOUT;
    }
    return status;
}

volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;



uint16_t* findMemoryBlock(uint16_t* ptrToFlashLocation, uint16_t storageSize, uint16_t sizeOfData, bool readOrWrite);
uint16_t* findMemoryBlock(uint16_t* ptrToFlashLocation, uint16_t storageSize, uint16_t sizeOfData, bool readOrWrite)
{ 
	if (readOrWrite) 
	{
		//Set the pointer to the last block because we want to find the LAST written block.
		uint16_t* ptr = ptrToFlashLocation + storageSize/sizeof(*ptr)- sizeOfData/sizeof(*ptr) ; //
		//We want to read the data. So we have to find the last written block or return NULL if all blocks are empty.
		while (ptr > ptrToFlashLocation - sizeOfData/sizeof(*ptr))
		{
			uint16_t* i = ptr;
			for (uint16_t j = 0; j < (sizeOfData/sizeof(*ptr))+0; j++)
			{
				if (*i != 0xFFFFu)
				{
					//We've found the last written block. Return the pointer to it.
					return ptr;
				}
				i++;
			}
			//Next block
			ptr -= sizeOfData/sizeof(*ptr);
		}
		//All blocks are empty. Return NULL.
		return NULL;									
	}
	else //We want to write the data. So we have to find the first empty block or return NULL if all blocks are full.
	{
		uint16_t* ptr = ptrToFlashLocation;
		while (ptr < (ptrToFlashLocation + storageSize/sizeof(*ptr)))
		{
			uint16_t* i = ptr;
			for (uint16_t j = 0; j < sizeOfData/sizeof(*ptr); j++)
			{
				if (*i != 0xFFFFu)
				{
					//Next block 
					goto nextBlock;
				}
				i++;
			}
			//We've found the first empty block. Return the pointer to it. 
			return ptr;

			nextBlock:
			ptr += sizeOfData/sizeof(*ptr);
		}
		//All blocks are full. Return NULL.
		return NULL;									
	}
}

void getSavedMileageDataFromFlash(uint16_t* ptrToFlashLocation, uint32_t storageSize, uint32_t sizeOfData) {
    uint16_t* ptr = findMemoryBlock(ptrToFlashLocation, storageSize, sizeOfData, true);
	if (ptr == NULL)
	{
		//No data found. Reset the mileageData structure to zero. 
		memset(&mileageData, 0, sizeof(mileageData));
		mileageData.serviceOverdue = MACHINE_SERVICE_INTERVALS;
	}
	else
	{
		//Data found. Copy it to mileageData structure.
		memcpy(&mileageData, ptr, sizeof(mileageData));
		machineData.machine.currentDistance = mileageData.currentDistance;
		machineData.machine.time = mileageData.currentTime;
		if (mileageData.serviceOverdue <= MACHINE_SERVICE_WARNING_MESSAGE_SHOW_WHEN) machineData.flags.needsServicing = true;
	}
}

/**
 * @brief If the battery is low, save data to FLASH to ensure that mileage can't be reset just by fully-discharging the battery
 * 
 * @param machineData 
 */
 void saveMachineMileageDataToFlash(uint16_t* ptrToFlashLocation, uint16_t storageSize, uint16_t sizeOfData) { 
	
	// Unkock flash - be aware you need extra stuff for the bootloader.
	FLASH->KEYR = 0x45670123; //Magic numbers from the datasheet.
	FLASH->KEYR = 0xCDEF89AB; //Magic numbers from the datasheet.

	// For unlocking programming, in general.
	FLASH->MODEKEYR = 0x45670123; //Magic numbers from the datasheet.
	FLASH->MODEKEYR = 0xCDEF89AB; //Magic numbers from the datasheet.

	/*Check if the memory is busy*/
	while( FLASH->STATR & FLASH_STATR_BSY );
	
	/*Ensure that flash been unlocked*/
	if( FLASH->CTLR & 0x8080 ) 
	{
		//Something is really really wrong. Reset everything 
		NVIC_SystemReset();
	}
	
	/*Use the last 512 bytes of flash memory to store the data*/
	uint16_t* ptr = findMemoryBlock(ptrToFlashLocation, storageSize, sizeOfData, false);

	if (ptr == NULL) 
	{
		//No free space found. Erase the whole storage and try again.
		//We erase 8 pages of 64 bytes each
		//Erase Page
		ptr = ptrToFlashLocation;
		while (ptr < (ptrToFlashLocation + (storageSize))) //sizeof(*ptr)
		{
			FLASH->CTLR = CR_PAGE_ER;
			FLASH->ADDR = (uint32_t)ptr;
			FLASH->CTLR = CR_STRT_Set | CR_PAGE_ER;
			while (FLASH->STATR & FLASH_STATR_BSY);  // Takes about 3ms.
			FLASH->CTLR = FLASH_STATR_EOP; 
			ptr += 32; //Go to the next page
		}
		ptr = ptrToFlashLocation; //Reset the pointer to the beginning of the storage
		/*Ensure that we've actually erased the memory*/
		uint16_t* i = ptrToFlashLocation;
		while (i < (ptrToFlashLocation + (storageSize/sizeof(*i))))
		{
			if (*i != 0xFFFFu)
			{
				//Something is really really wrong. Reset everything
				NVIC_SystemReset();
			}
			i++; 
		}
	}
	//Copy the values to the mileageData structure
	mileageData.currentDistance = machineData.machine.currentDistance;
	mileageData.currentTime = machineData.machine.time;
	//Write the data to the flash
	uint16_t* mileageDataPtr = (uint16_t*)&mileageData;
	for (uint8_t i = 0; i < sizeof(mileageData)/sizeof(*ptr); i++, ptr++, mileageDataPtr++)
	{
		if (FLASH_ProgramHalfWord((uint32_t)ptr, *mileageDataPtr) != FLASH_COMPLETE)
		{
			NVIC_SystemReset();
		} 
	}
	return;
}