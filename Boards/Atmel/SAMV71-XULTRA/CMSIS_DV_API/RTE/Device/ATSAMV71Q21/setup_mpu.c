/* ----------------------------------------------------------------------------
 *         SAM Software Package License 
 * ----------------------------------------------------------------------------
 * Copyright (c) 2014, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 * Modifications Copyright (c) 2017 ARM Germany GmbH. All rights reserved.
 */

/*-------------------------------------------------------------------------- */
/*         Headers                                                           */
/*-------------------------------------------------------------------------- */
#include <chip.h>

/*----------------------------------------------------------------------------
 *        Definitions
 *----------------------------------------------------------------------------*/
#define MPU_DEFAULT_ITCM_REGION             ( 1 )
#define MPU_DEFAULT_IFLASH_REGION           ( 2 )
#define MPU_DEFAULT_DTCM_REGION             ( 3 )
#define MPU_DEFAULT_SRAM_REGION_1           ( 4 )
#define MPU_DEFAULT_SRAM_REGION_2           ( 5 )
#define MPU_NOCACHE_SRAM_REGION             ( 6 )
#define MPU_PERIPHERALS_REGION              ( 7 )
#define MPU_EXT_EBI_REGION                  ( 8 )
#define MPU_DEFAULT_SDRAM_REGION            ( 9 )
#define MPU_QSPIMEM_REGION                  ( 10 )
#define MPU_USBHSRAM_REGION                 ( 11 )

/********* IFLASH memory macros *********************/
#define ITCM_START_ADDRESS                  0x00000000UL
#define ITCM_END_ADDRESS                    0x003FFFFFUL
#define IFLASH_START_ADDRESS                0x00400000UL
#define IFLASH_END_ADDRESS                  0x005FFFFFUL

#define IFLASH_PRIVILEGE_START_ADDRESS      (IFLASH_START_ADDRESS)
#define IFLASH_PRIVILEGE_END_ADDRESS        (IFLASH_START_ADDRESS + 0xFFF)

#define IFLASH_UNPRIVILEGE_START_ADDRESS    (IFLASH_PRIVILEGE_END_ADDRESS + 1)
#define IFLASH_UNPRIVILEGE_END_ADDRESS      (IFLASH_END_ADDRESS)

/**************** DTCM  *******************************/
#define DTCM_START_ADDRESS                  0x20000000UL
#define DTCM_END_ADDRESS                    0x203FFFFFUL


/******* SRAM memory macros ***************************/

#define SRAM_START_ADDRESS                  0x20400000UL
#define SRAM_END_ADDRESS                    0x2045FFFFUL

#define NOCACHE_SRAM_REGION_SIZE            0x1000

/* Regions should be a 2^(N+1)  where 4 < N < 31 */
#define SRAM_FIRST_START_ADDRESS            (SRAM_START_ADDRESS)
#define SRAM_FIRST_END_ADDRESS              (SRAM_FIRST_START_ADDRESS + 0x3FFFF)        // (2^18) 256 KB 

#define SRAM_SECOND_START_ADDRESS           (SRAM_FIRST_END_ADDRESS+1)
#define SRAM_SECOND_END_ADDRESS             (SRAM_END_ADDRESS - NOCACHE_SRAM_REGION_SIZE )              // (2^17) 128 - 0x1000 KB
#define SRAM_NOCACHE_START_ADDRESS          (SRAM_SECOND_END_ADDRESS + 1)
#define SRAM_NOCACHE_END_ADDRESS            (SRAM_END_ADDRESS )

/************** Peripherals memory region macros ********/
#define PERIPHERALS_START_ADDRESS            0x40000000UL
#define PERIPHERALS_END_ADDRESS              0x5FFFFFFFUL

/******* Ext EBI memory macros ***************************/
#define EXT_EBI_START_ADDRESS                0x60000000UL
#define EXT_EBI_END_ADDRESS                  0x6FFFFFFFUL

/******* Ext-SRAM memory macros ***************************/
#define SDRAM_START_ADDRESS                  0x70000000UL
#define SDRAM_END_ADDRESS                    0x7FFFFFFFUL

/******* QSPI macros ***************************/
#define QSPI_START_ADDRESS                   0x80000000UL
#define QSPI_END_ADDRESS                     0x9FFFFFFFUL

/************** USBHS_RAM region macros ******************/
#define USBHSRAM_START_ADDRESS               0xA0100000UL
#define USBHSRAM_END_ADDRESS                 0xA01FFFFFUL


/* Default memory map 
   Address range        Memory region      Memory type   Shareability  Cache policy
   0x00000000- 0x1FFFFFFF Code             Normal        Non-shareable  WT
   0x20000000- 0x3FFFFFFF SRAM             Normal        Non-shareable  WBWA
   0x40000000- 0x5FFFFFFF Peripheral       Device        Non-shareable  -
   0x60000000- 0x7FFFFFFF RAM              Normal        Non-shareable  WBWA
   0x80000000- 0x9FFFFFFF RAM              Normal        Non-shareable  WT
   0xA0000000- 0xBFFFFFFF Device           Device        Shareable
   0xC0000000- 0xDFFFFFFF Device           Device        Non Shareable
   0xE0000000- 0xFFFFFFFF System           -                  -
   */

/**
 * \brief Set up a memory region.
 */
void _SetupMemoryRegion( void )
{
  ARM_MPU_Region_t region;
    
  memory_barrier();

/***************************************************
  ITCM memory region --- Normal 
  START_Addr:-  0x00000000UL
  END_Addr:-    0x00400000UL
****************************************************/

  region.RBAR = ARM_MPU_RBAR( MPU_DEFAULT_ITCM_REGION,    // Region 1
                              ITCM_START_ADDRESS);        // BaseAddress

  region.RASR = ARM_MPU_RASR( 0U,                         // DisableExec     
                              ARM_MPU_AP_PRIV,            // AccessPermission
                              0U,                         // TypeExtField    
                              0U,                         // IsShareable     
                              0U,                         // IsCacheable     
                              0U,                         // IsBufferable    
                              0U,                         // SubRegionDisable
                              ARM_MPU_REGION_SIZE_4MB);   // Size                                                   
      
  ARM_MPU_SetRegion(region.RBAR, region.RASR);

/****************************************************
  Internal flash memory region --- Normal read-only 
  (update to Strongly ordered in write accesses)
  START_Addr:-  0x00400000UL
  END_Addr:-    0x00600000UL
******************************************************/
    
  region.RBAR = ARM_MPU_RBAR( MPU_DEFAULT_IFLASH_REGION,  // Region 2
                              IFLASH_START_ADDRESS);      // BaseAddress

  region.RASR = ARM_MPU_RASR( 0U,                         // DisableExec     
                              ARM_MPU_AP_RO,              // AccessPermission
                              4U,                         // TypeExtField    
                              0U,                         // IsShareable     
                              1U,                         // IsCacheable     
                              1U,                         // IsBufferable    
                              0U,                         // SubRegionDisable
                              ARM_MPU_REGION_SIZE_2MB);   // Size                                                   

  ARM_MPU_SetRegion(region.RBAR, region.RASR);

/****************************************************
  DTCM memory region --- Normal
  START_Addr:-  0x20000000L
  END_Addr:-    0x20400000UL
******************************************************/
    
  region.RBAR = ARM_MPU_RBAR( MPU_DEFAULT_DTCM_REGION,    // Region 3
                              DTCM_START_ADDRESS);        // BaseAddress

  region.RASR = ARM_MPU_RASR( 0U,                         // DisableExec     
                              ARM_MPU_AP_PRIV,            // AccessPermission
                              0U,                         // TypeExtField    
                              0U,                         // IsShareable     
                              0U,                         // IsCacheable     
                              0U,                         // IsBufferable    
                              0U,                         // SubRegionDisable
                              ARM_MPU_REGION_SIZE_4MB);   // Size                                                   

  ARM_MPU_SetRegion(region.RBAR, region.RASR);

/****************************************************
  SRAM Cacheable memory region --- Normal
  START_Addr:-  0x20400000UL
  END_Addr:-    0x2043FFFFUL
******************************************************/

  /* SRAM memory region */
  region.RBAR = ARM_MPU_RBAR( MPU_DEFAULT_SRAM_REGION_1,  // Region 4
                              SRAM_FIRST_START_ADDRESS);  // BaseAddress

  region.RASR = ARM_MPU_RASR( 0U,                         // DisableExec     
                              ARM_MPU_AP_FULL,            // AccessPermission
                              4U,                         // TypeExtField    
                              0U,                         // IsShareable     
                              1U,                         // IsCacheable     
                              1U,                         // IsBufferable    
                              0U,                         // SubRegionDisable
                              ARM_MPU_REGION_SIZE_256KB); // Size                                                   

  ARM_MPU_SetRegion(region.RBAR, region.RASR);

/****************************************************
  Internal SRAM second partition memory region --- Normal 
  START_Addr:-  0x20440000UL
  END_Addr:-    0x2045FFFFUL
******************************************************/

  /* SRAM memory region */  
  region.RBAR = ARM_MPU_RBAR( MPU_DEFAULT_SRAM_REGION_2,  // Region 5
                              SRAM_SECOND_START_ADDRESS); // BaseAddress

  region.RASR = ARM_MPU_RASR( 0U,                         // DisableExec     
                              ARM_MPU_AP_FULL,            // AccessPermission
                              4U,                         // TypeExtField    
                              0U,                         // IsShareable     
                              1U,                         // IsCacheable     
                              1U,                         // IsBufferable    
                              0U,                         // SubRegionDisable
                              ARM_MPU_REGION_SIZE_128KB); // Size                                                   

  ARM_MPU_SetRegion(region.RBAR, region.RASR);

  /* NOCACHE_REGION */      
  region.RBAR = ARM_MPU_RBAR( MPU_NOCACHE_SRAM_REGION,    // Region 6
                              SRAM_NOCACHE_START_ADDRESS);// BaseAddress

  region.RASR = ARM_MPU_RASR( 0U,                         // DisableExec     
                              ARM_MPU_AP_FULL,            // AccessPermission
                              1U,                         // TypeExtField    
                              1U,                         // IsShareable     
                              0U,                         // IsCacheable     
                              0U,                         // IsBufferable    
                              0U,                         // SubRegionDisable
                              ARM_MPU_REGION_SIZE_4KB);   // Size                                                   

  ARM_MPU_SetRegion(region.RBAR, region.RASR);

/****************************************************
  Peripheral memory region --- DEVICE Shareable
  START_Addr:-  0x40000000UL
  END_Addr:-    0x5FFFFFFFUL
******************************************************/

  region.RBAR = ARM_MPU_RBAR( MPU_PERIPHERALS_REGION,     // Region 7
                              PERIPHERALS_START_ADDRESS); // BaseAddress

  region.RASR = ARM_MPU_RASR( 1U,                         // DisableExec     
                              ARM_MPU_AP_FULL,            // AccessPermission
                              0U,                         // TypeExtField    
                              0U,                         // IsShareable     
                              0U,                         // IsCacheable     
                              1U,                         // IsBufferable    
                              0U,                         // SubRegionDisable
                              ARM_MPU_REGION_SIZE_512MB); // Size                                                   

  ARM_MPU_SetRegion(region.RBAR, region.RASR);

/****************************************************
  External EBI memory  memory region --- Strongly Ordered
  START_Addr:-  0x60000000UL
  END_Addr:-    0x6FFFFFFFUL
******************************************************/

  /* External memory Must be defined with 'Device' or 'Strongly Ordered'
  attribute for write accesses (AXI) */   
  region.RBAR = ARM_MPU_RBAR( MPU_EXT_EBI_REGION,         // Region 8
                              EXT_EBI_START_ADDRESS);     // BaseAddress

  region.RASR = ARM_MPU_RASR( 0U,                         // DisableExec     
                              ARM_MPU_AP_FULL,            // AccessPermission
                              0U,                         // TypeExtField    
                              0U,                         // IsShareable     
                              0U,                         // IsCacheable     
                              0U,                         // IsBufferable    
                              0U,                         // SubRegionDisable
                              ARM_MPU_REGION_SIZE_256MB); // Size                                                   

  ARM_MPU_SetRegion(region.RBAR, region.RASR);

/****************************************************
  SDRAM Cacheable memory region --- Normal
  START_Addr:-  0x70000000UL
  END_Addr:-    0x7FFFFFFFUL
******************************************************/

  region.RBAR = ARM_MPU_RBAR( MPU_DEFAULT_SDRAM_REGION,   // Region 9
                              SDRAM_START_ADDRESS);       // BaseAddress

  region.RASR = ARM_MPU_RASR( 0U,                         // DisableExec     
                              ARM_MPU_AP_FULL,            // AccessPermission
                              4U,                         // TypeExtField    
                              1U,                         // IsShareable     
                              0U,                         // IsCacheable     
                              1U,                         // IsBufferable    
                              0U,                         // SubRegionDisable
                              ARM_MPU_REGION_SIZE_256MB); // Size                                                   

  ARM_MPU_SetRegion(region.RBAR, region.RASR);

/****************************************************
  QSPI memory region --- Strongly ordered
  START_Addr:-  0x80000000UL
  END_Addr:-    0x9FFFFFFFUL
******************************************************/

  region.RBAR = ARM_MPU_RBAR( MPU_QSPIMEM_REGION,         // Region 10
                              QSPI_START_ADDRESS);        // BaseAddress

  region.RASR = ARM_MPU_RASR( 0U,                         // DisableExec     
                              ARM_MPU_AP_FULL,            // AccessPermission
                              0U,                         // TypeExtField    
                              0U,                         // IsShareable     
                              0U,                         // IsCacheable     
                              0U,                         // IsBufferable    
                              0U,                         // SubRegionDisable
                              ARM_MPU_REGION_SIZE_512MB); // Size                                                   

  ARM_MPU_SetRegion(region.RBAR, region.RASR);

/****************************************************
  USB RAM Memory region --- Device
  START_Addr:-  0xA0100000UL
  END_Addr:-    0xA01FFFFFUL
******************************************************/

  region.RBAR = ARM_MPU_RBAR( MPU_USBHSRAM_REGION,        // Region 11
                              USBHSRAM_START_ADDRESS);    // BaseAddress

  region.RASR = ARM_MPU_RASR( 1U,                         // DisableExec     
                              ARM_MPU_AP_FULL,            // AccessPermission
                              0U,                         // TypeExtField    
                              0U,                         // IsShareable     
                              0U,                         // IsCacheable     
                              1U,                         // IsBufferable    
                              0U,                         // SubRegionDisable
                              ARM_MPU_REGION_SIZE_1MB);   // Size                                                   

  ARM_MPU_SetRegion(region.RBAR, region.RASR);

  /* Enable the MPU and privileged software access to the default memory map*/
  ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);

  memory_sync();
}
