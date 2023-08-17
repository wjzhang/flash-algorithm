#define uint8_t     unsigned char
#define uint16_t    unsigned short
#define uint32_t    unsigned long
#define uint64_t    unsigned long long

#define int8_t      signed char
#define int16_t     signed short
#define int32_t     signed long


/*********************************************************************
*
*       Register definitions
*/
#define FMC_WS_REG        (*(volatile unsigned long *)0x40022000)
#define FMC_KEY_REG       (*(volatile unsigned long *)0x40022004)
#define FMC_OPTKEY_REG    (*(volatile unsigned long *)0x40022008)
#define FMC_STAT_REG      (*(volatile unsigned long *)0x4002200C)
#define FMC_CTL_REG       (*(volatile unsigned long *)0x40022010)
#define FMC_ADDR_REG      (*(volatile unsigned long *)0x40022014)
#define FMC_OBSTAT_REG    (*(volatile unsigned long *)0x4002201C)
#define FMC_WR_REG        (*(volatile unsigned long *)0x40022020)
#define FMC_PID_REG       (*(volatile unsigned long *)0x40022100)


/*********************************************************************
*
*      state Register bit definitions
*/
#define FLASH_STAT_BSY          0x0001
#define FLASH_STAT_PGERR        0x0004
#define FLASH_STAT_WRPRTERR     0x0010
#define FLASH_STAT_ENDF         0x0020

/*********************************************************************
*
*      control Register bit definitions
*/
#define FLASH_CTL_PG          0x0001
#define FLASH_CTL_PER         0x0002
#define FLASH_CTL_MER         0x0004
#define FLASH_CTL_OBPG        0x0010
#define FLASH_CTL_OBER        0x0020
#define FLASH_CTL_START       0x0040
#define FLASH_CTL_LOCK        0x0080
#define FLASH_CTL_OBWEN       0x0200
#define FLASH_CTL_ERRIE       0x0400
#define FLASH_CTL_ENDIE       0x1000




/*********************************************************************
*
*      FLASH key
*/
#define FLASH_RDPRT_KEY        0x00A5
#define FLASH_UNLOCK_KEY1      0x45670123
#define FLASH_UNLOCK_KEY2      0xCDEF89AB




/*
 *  Unlock the flash
 *    Parameter:      None
 *    Return Value:   0 - OK,  
 */
int unlockFlash(void) {
    FMC_KEY_REG = FLASH_UNLOCK_KEY1;
    FMC_KEY_REG = FLASH_UNLOCK_KEY2;
    return 0;
 }



/*
 *  Initialize Flash Programming Functions
 *    Parameter:      addr:  Device Base Address
 *    Return Value:   0 - OK,  1 - Failed
 */
int init (unsigned long addr) {
    //
    // No special init necessary
    //
    /* clear error, disable interrupt */
    FMC_STAT_REG = FLASH_STAT_PGERR | FLASH_STAT_WRPRTERR | FLASH_STAT_ENDF;
    
    __asm volatile("mv a0, x0\n");
    __asm volatile("ebreak\n");
    return (0);
}

/*
 *  De-Initialize Flash Programming Functions
 *    Return Value:   0 - OK,  1 - Failed
 */

int unInit (void) {
    //
    // No special uninit necessary
    //
    __asm volatile("mv a0, x0\n");
    __asm volatile("ebreak\n");
    return (0);
}

/*
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 */
int eraseChip (void) {

    uint32_t cr = 0;
    uint32_t sr = 0;
    
    /* check flash is locked, if yes, unlock it */
    cr = FMC_CTL_REG;
    if ((cr & FLASH_CTL_LOCK) == FLASH_CTL_LOCK)
    {
        unlockFlash();
    }
    /* wait SR BSY cleared */
    do {
        sr = FMC_STAT_REG;
    } while ((sr & FLASH_STAT_BSY) == FLASH_STAT_BSY);
    
    /* first set MER bit, then set START bit */
    cr = FMC_CTL_REG;
    cr |= FLASH_CTL_MER;
    FMC_CTL_REG = cr;
    
    cr = FMC_CTL_REG;
    cr |= FLASH_CTL_START;
    FMC_CTL_REG = cr;
    
    /* wait SR BSY cleared */
    do {
        sr = FMC_STAT_REG;
    } while ((sr & FLASH_STAT_BSY) == FLASH_STAT_BSY);

    /* clear MER bit */
    cr = FMC_CTL_REG;
    cr &= ~FLASH_CTL_MER;
    FMC_CTL_REG = cr;
    
    __asm volatile("mv a0, x0\n");
    __asm volatile("ebreak\n");
    return (0);                                    // Finished without Errors
}

/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */
int eraseSector (unsigned long adr) {
    uint32_t cr = 0;
    uint32_t sr = 0;
    /* check flash is locked, if yes, unlock it */
    cr = FMC_CTL_REG;
    if ((cr & FLASH_CTL_LOCK) == FLASH_CTL_LOCK)
    {
        unlockFlash();
    }
    /* wait SR BSY cleared */
    do {
        sr = FMC_STAT_REG;
    } while ((sr & FLASH_STAT_BSY) == FLASH_STAT_BSY);    
    
    /* first set PER bit, then set address, last set STRT bit */
    cr = FMC_CTL_REG;
    cr |= FLASH_CTL_PER;
    FMC_CTL_REG = cr;
    
    /* set erase sector address */
    FMC_ADDR_REG = adr; 
    
    cr = FMC_CTL_REG;
    cr |= FLASH_CTL_START;
    FMC_CTL_REG = cr;    
    
    /* wait SR BSY cleared */
    do {
        sr = FMC_STAT_REG;
    } while ((sr & FLASH_STAT_BSY) == FLASH_STAT_BSY);

    /* clear PER bit */
    cr = FMC_CTL_REG;
    cr &= ~FLASH_CTL_PER;
    FMC_CTL_REG = cr;    

    __asm volatile("mv a0, x0\n");
    __asm volatile("ebreak\n");
    return (0);
}

/*
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */
int programPage (unsigned long adr, unsigned long sz, unsigned char *buf) {
    volatile uint16_t * pDest;
    volatile uint16_t * pSrc;
    uint32_t cr = 0;
    uint32_t sr = 0;
    uint32_t i = 0;
    uint16_t data;
    
    pDest = (volatile uint16_t *)adr;
    pSrc  = (volatile uint16_t *)buf;    // Always 32-bit aligned. Made sure by CMSIS-DAP firmware

    /* check flash is locked, if yes, unlock it */
    cr = FMC_CTL_REG;
    if ((cr & FLASH_CTL_LOCK) == FLASH_CTL_LOCK)
    {
        unlockFlash();
    }
    /* wait SR BSY cleared */
    do {
        sr = FMC_STAT_REG;
    } while ((sr & FLASH_STAT_BSY) == FLASH_STAT_BSY);    

    while (i < sz/2 )
    {
        /* first set PG bit in CR, then write data to flash address */
        cr = FMC_CTL_REG;
        cr |= FLASH_CTL_PG;
        FMC_CTL_REG = cr;
        
        *pDest = *pSrc;
        /* wait SR BSY cleared */
        do {
            sr = FMC_STAT_REG;
        } while((sr & FLASH_STAT_BSY) == FLASH_STAT_BSY);
        
        /* check program word is ok */
        if (*pSrc != *pDest)
        {
            /* clear PG bit */
            cr = FMC_CTL_REG;
            cr &= ~FLASH_CTL_PG;
            FMC_CTL_REG = cr;    
            
            __asm volatile("li a0, 0x01\n");
            __asm volatile("ebreak\n");
            return 1;
        }
        pDest++;
        pSrc++;
        i++;    
    }
    
    /* clear PG bit */
    cr = FMC_CTL_REG;
    cr &= ~FLASH_CTL_PG;
    FMC_CTL_REG = cr;
    
    __asm volatile("mv a0, x0\n");
    __asm volatile("ebreak\n");
    return (0);
}
