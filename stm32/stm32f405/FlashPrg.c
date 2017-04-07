/* CMSIS-DAP Interface Firmware
 * Copyright (c) 2009-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../../FlashOS.H"        // FlashOS Structures

#define U8  unsigned char
#define U16 unsigned short
#define U32 unsigned long
#define U64 unsigned long long

#define I8  signed char
#define I16 signed short
#define I32 signed long


/*********************************************************************
*
*       Register definitions
*/
#define FLASH_ACR_REG        (*(volatile unsigned long *)0x40023C00)
#define FLASH_KEYR_REG       (*(volatile unsigned long *)0x40023C04)
#define FLASH_OPTKEYR_REG    (*(volatile unsigned long *)0x40023C08)
#define FLASH_SR_REG         (*(volatile unsigned long *)0x40023C0C)
#define FLASH_CR_REG         (*(volatile unsigned long *)0x40023C10)
#define FLASH_OPTCR_REG      (*(volatile unsigned long *)0x40023C14)

	

/*********************************************************************
*
*      SR Register bit definitions
*/
#define FLASH_SR_EOP          0x00000001
#define FLASH_SR_OPERR        0x00000002
#define FLASH_SR_WRPRTERR     0x00000010
#define FLASH_SR_PGAERR       0x00000020
#define FLASH_SR_PGPERR       0x00000040
#define FLASH_SR_PGSERR       0x00000080
#define FLASH_SR_BSY          0x00010000

/*********************************************************************
*
*      CR Register bit definitions
*/
#define FLASH_CR_PG          0x00000001
#define FLASH_CR_SER         0x00000002
#define FLASH_CR_MER         0x00000004

#define FLASH_CR_SNB_MASK	   0x00000078		// 0111 1000
#define FLASH_CR_SNB_SHIFT	 3

#define FLASH_CR_8_SIZE			 0x00000000
#define FLASH_CR_16_SIZE		 0x00000100
#define FLASH_CR_32_SIZE		 0x00000200
#define FLASH_CR_64_SIZE		 0x00000300
#define FLASH_CR_SIZE_MASK	 0x00000300

#define FLASH_CR_STRT        0x00010000
#define FLASH_CR_LOCK        0x80000000





/*********************************************************************
*
*      FLASH key
*/
#define FLASH_UNLOCK_KEY1      0x45670123
#define FLASH_UNLOCK_KEY2      0xCDEF89AB


/*
 *  get Sector number 
 *   STM32F045:   0x08000000 - 0x08010000: 4 sector/16KB
									0x08010000 - 0x08020000: 1 sector/64KB
									0x08020000 - 0x08100000: 7 sector/128KB									
 *    Parameter:      addr:  flash address
 *    Return Value:   sector: 0 - 11
 */

unsigned long getSector (unsigned long addr) {
		unsigned long sector = 0xFFFFFFFF;
		if(addr >= 0x08000000 && addr < 0x08010000)
		{
			sector = (addr - 0x08000000)/0x00004000;
		}
		else if(addr >= 0x08010000 && addr < 0x08020000)
		{
			sector = 4;
		}
		else if(addr >= 0x08020000 && addr < 0x08100000)
		{
			sector = 5 + (addr - 0x08020000)/0x00020000;
		}
		return sector;
}
	
	

/*
 *  Unlock the flash
 *    Parameter:      None
 *    Return Value:   0 - OK,  
 */
 int UnlockFlash(void) {
    FLASH_KEYR_REG = FLASH_UNLOCK_KEY1;
    FLASH_KEYR_REG = FLASH_UNLOCK_KEY2;
    return 0;
 }



/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */
int Init (unsigned long adr, unsigned long clk, unsigned long fnc) {
	//
	// No special init necessary
	//
	/*clear SR*/
	FLASH_SR_REG = FLASH_SR_PGAERR | FLASH_SR_PGPERR | FLASH_SR_PGSERR | FLASH_SR_WRPRTERR | FLASH_SR_EOP;
  return (0);
}

/*
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int UnInit (unsigned long fnc) {
	//
	// No special uninit necessary
	//
  return (0);
}



/*
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 */
int EraseChip (void) {

	U32 cr = 0;
	U32 sr = 0;
	
	/*check flash is locked, if yes, unlock it*/
	cr = FLASH_CR_REG;
	if( (cr & FLASH_CR_LOCK) == FLASH_CR_LOCK)
	{
		UnlockFlash();
	}
	/*wait SR BSY cleared*/
	do{
		sr = FLASH_SR_REG;
	}while((sr & FLASH_SR_BSY) == FLASH_SR_BSY);
	
	/*first clear PG size, set MER bit, then set STRT bit*/
	cr = FLASH_CR_REG;
	cr &= ~FLASH_CR_SIZE_MASK;
	cr |= FLASH_CR_MER;
	FLASH_CR_REG = cr;
	
	cr = FLASH_CR_REG;
	cr |= FLASH_CR_STRT;
	FLASH_CR_REG = cr;	
	
	/*wait SR BSY cleared*/
	do{
		sr = FLASH_SR_REG;
	}while((sr & FLASH_SR_BSY) == FLASH_SR_BSY);

	/*clear MER bit*/
	cr = FLASH_CR_REG;
	cr &= ~FLASH_CR_MER;
	FLASH_CR_REG = cr;
	

  return (0);                                    // Finished without Errors
}

/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */
int EraseSector (unsigned long adr) {
	U32 cr = 0;
	U32 sr = 0;
	unsigned long sector = 0;
	/*check flash is locked, if yes, unlock it*/
	cr = FLASH_CR_REG;
	if( (cr & FLASH_CR_LOCK) == FLASH_CR_LOCK)
	{
		UnlockFlash();
	}
	/*wait SR BSY cleared*/
	do{
		sr = FLASH_SR_REG;
	}while((sr & FLASH_SR_BSY) == FLASH_SR_BSY);	
	
	/*first set SER bit, erase sector, last set STRT bit*/
	sector = getSector(adr);	
	cr = FLASH_CR_REG;
	cr &= ~(FLASH_CR_SIZE_MASK | FLASH_CR_SNB_MASK);
	cr |= (FLASH_CR_SER | (sector << FLASH_CR_SNB_SHIFT) | FLASH_CR_32_SIZE);
	FLASH_CR_REG = cr;
	 
	cr = FLASH_CR_REG;
	cr |= FLASH_CR_STRT;
	FLASH_CR_REG = cr;	
	
	/*wait SR BSY cleared*/
	do{
		sr = FLASH_SR_REG;
	}while((sr & FLASH_SR_BSY) == FLASH_SR_BSY);

	/*clear SER bit*/
	cr = FLASH_CR_REG;
	cr &= ~FLASH_CR_SER;
	FLASH_CR_REG = cr;	

  return (0);
}

/*
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */
int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf) {
  volatile U32* p32Dest;
  volatile U32* p32Src;
  volatile U16* p16Dest;
  volatile U16* p16Src;
	U32 cr = 0;
	U32 sr = 0;
	unsigned long i = 0;
	
  p32Dest = (volatile U32*)adr;
  p32Src = (volatile U32*)buf;    // Always 32-bit aligned. Made sure by CMSIS-DAP firmware
	//
	// adr is always aligned to "Programming Page Size" specified in table in FlashDev.c
  // sz is always a multiple of "Programming Page Size"
	//
	/*check flash is locked, if yes, unlock it*/
	cr = FLASH_CR_REG;
	if( (cr & FLASH_CR_LOCK) == FLASH_CR_LOCK)
	{
		UnlockFlash();
	}
	/*wait SR BSY cleared*/
	do{
		sr = FLASH_SR_REG;
	}while((sr & FLASH_SR_BSY) == FLASH_SR_BSY);	

	//clear Program size
	cr = FLASH_CR_REG;
	cr &= ~FLASH_CR_SIZE_MASK;
	FLASH_CR_REG = cr;
	
	while(i < sz/4 )
	{
		/*first set PG bit/Program size: 32bit in CR, then write data to flash address*/
		cr = FLASH_CR_REG;
		cr |= (FLASH_CR_PG | FLASH_CR_32_SIZE);
		FLASH_CR_REG = cr;
		
		*p32Dest = *p32Src;
		/*wait SR BSY cleared*/
		do{
			sr = FLASH_SR_REG;
		}while((sr & FLASH_SR_BSY) == FLASH_SR_BSY);
		
		/*check program word is ok*/
		if(*p32Src != *p32Dest)
		{
			/*clear PG bit*/
			cr = FLASH_CR_REG;
			cr &= ~(FLASH_CR_PG | FLASH_CR_SIZE_MASK);
			FLASH_CR_REG = cr;	
			
			return 1;
		}
		p32Dest++;
		p32Src++;
		i++;	
	}
	//half word
	if(sz%4 != 0)
	{
		p16Dest = (volatile U16*)p32Dest;
		p16Src  = (volatile U16*)p32Src;
		/*set PG bit/Program size: 16 bit in CR, then write data to flash address*/
		cr = FLASH_CR_REG;
		cr &= ~FLASH_CR_SIZE_MASK;
		cr |= (FLASH_CR_PG | FLASH_CR_16_SIZE);
		FLASH_CR_REG = cr;
		
		*p16Dest = *p16Src;
		/*wait SR BSY cleared*/
		do{
			sr = FLASH_SR_REG;
		}while((sr & FLASH_SR_BSY) == FLASH_SR_BSY);
		
		/*check program word is ok*/
		if(*p16Src != *p16Dest)
		{
			/*clear PG bit*/
			cr = FLASH_CR_REG;
			cr &= ~(FLASH_CR_PG | FLASH_CR_SIZE_MASK);
			FLASH_CR_REG = cr;	
			
			return 1;
		}		
	}	
	
	/*clear PG bit*/
	cr = FLASH_CR_REG;
	cr &= ~(FLASH_CR_PG | FLASH_CR_SIZE_MASK);
	FLASH_CR_REG = cr;		
	
  return (0);                                  // Finished without Errors
}
