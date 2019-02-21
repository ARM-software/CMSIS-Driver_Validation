/*----------------------------------------------------------------------------
 * Name:    main.c
 *----------------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
#include "sam.h"
#include "cmsis_dv.h"                   // ARM.API::CMSIS Driver Validation:Framework

extern void _SetupMemoryRegion( void );

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{

#if defined(RTE_CMSIS_RTOS) || defined(RTE_CMSIS_RTOS2)
  osKernelInitialize ();
#endif

  /* Disable watchdog */
  WDT->WDT_MR = WDT_MR_WDDIS;
  
  /* Setup MPU */
  _SetupMemoryRegion();

  /* Configure the system clock to 300 MHz */
  SystemCoreClockUpdate();
  
  /* Enable data and instruction caches */
  SCB_EnableDCache();	
  SCB_EnableICache();

#if defined(RTE_CMSIS_RTOS2)
  osThreadNew(cmsis_dv, NULL, NULL);
#else
  cmsis_dv(NULL);
#endif

#if defined(RTE_CMSIS_RTOS) || defined(RTE_CMSIS_RTOS2)
  osKernelStart ();
#endif

  /* Infinite loop */
  while (1)
  {
  }
}
