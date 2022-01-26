/***************************************************************************//**
 * @file     DataFlashProg.h
 * @brief    M480 series data flash programming driver header
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#ifndef __DATA_FLASH_PROG_H__
#define __DATA_FLASH_PROG_H__

#define DATA_FLASH_BASE       		(0x0007D000)  		/* To avoid the code to write APROM */
//#define DATA_FLASH_STORAGE_SIZE   	(8*1024)  		/* Configure the DATA FLASH storage size. To pass USB-IF MSC Test, it needs > 64KB */
#define FLASH_PAGE_SIZE           	FMC_FLASH_PAGE_SIZE
#define BUFFER_PAGE_SIZE          	(6)

void FLASH_Lock(void);
void FLASH_Unlock(void);

void DataFlashInit(void);
void DataFlashWrite(uint32_t addr, uint32_t size, uint32_t buffer);
void DataFlashRead(uint32_t addr, uint32_t size, uint32_t buffer);

void Emulate_EEPROM_Write(uint8_t idx , uint8_t value);
uint8_t Emulate_EEPROM_Read(uint8_t index);


#endif  /* __DATA_FLASH_PROG_H__ */

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
