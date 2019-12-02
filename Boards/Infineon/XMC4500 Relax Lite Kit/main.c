/*----------------------------------------------------------------------------
 * Name:    main.c
 *----------------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
#include "cmsis_dv.h"                   // ARM.API::CMSIS Driver Validation:Framework

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
