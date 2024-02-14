#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "ch32v003fun/ch32v003fun.h"
#include "machineData.h"

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

//Use the last 512 bytes of the flash for storing the mileage data
#define NON_VOLATILE_FLASH_DATA_STORAGE_SIZE 256u
#define FLASH_ADDR_TO_STORE_BACKUP_DATA (uint16_t*)0x8003F00//0x8003FB0 

/* Exported functions ------------------------------------------------------- */
FLASH_Status FLASH_WaitForLastOperation(uint32_t Timeout); 
FLASH_Status FLASH_ProgramHalfWord(uint32_t Address, uint16_t Data);
void saveMachineMileageDataToFlash(uint16_t* ptrToFlashLocation, uint16_t storageSize, uint16_t sizeOfData);
uint16_t* findMemoryBlock(uint16_t* ptrToFlashLocation, uint16_t storageSize, uint16_t sizeOfData, bool readOrWrite);
void getSavedMileageDataFromFlash(uint16_t* ptrToFlashLocation, uint32_t storageSize, uint32_t sizeOfData);